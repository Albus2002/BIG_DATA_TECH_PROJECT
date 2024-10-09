/*
* author: Wenrui Liu
* last edit time: 2023-4-12
*/
#include<malloc.h>
#include<packet.h>
#include<util.h>

namespace MiniTAP{

    /*
    * class RawData functions
    */
    RawData::RawData(const int _dirIndex): dirIndex(_dirIndex) {
        pktHeader.ts_sec = 0;
        pktHeader.ts_usec = 0;
        pktHeader.caplen = 0;
        pktHeader.len = 0;
        pktData = nullptr;
    }


    RawData::RawData(const pcapPacketHeader_t & _pktHeader, const int _dirIndex): dirIndex(_dirIndex) {
        pktHeader.ts_sec = _pktHeader.ts_sec;
        pktHeader.ts_usec = _pktHeader.ts_usec;
        pktHeader.caplen = _pktHeader.caplen;
        pktHeader.len = _pktHeader.len;
        pktData = static_cast<uint8_t*>(malloc(pktHeader.caplen));
    }

    size_t RawData::readPktData(FILE * fp) {
        size_t retVal = 0;
        if(pktHeader.caplen == 0) {
            return 0;
        }
        
        if(pktData != nullptr) {
            free(pktData);
            pktData = nullptr;
        }

        pktData = static_cast<uint8_t*>(malloc(pktHeader.caplen));
        retVal = fread(pktData, sizeof(uint8_t), pktHeader.caplen, fp);
        return retVal;
    }

    size_t RawData::fastReadPktData(uint8_t * mapData, uint64_t & mapPointer){
        size_t retVal = 0;
        if(pktHeader.caplen == 0) {
            return 0;
        }
        
        if(pktData != nullptr) {
            free(pktData);
            pktData = nullptr;
        }

        pktData = static_cast<uint8_t*>(malloc(pktHeader.caplen));
        memcpy(pktData, mapData+mapPointer, pktHeader.caplen);
        mapPointer += pktHeader.caplen;
        retVal = pktHeader.caplen;
        return retVal;
    }


    RawData::~RawData() {
        if(pktData != nullptr) {
            free(pktData);
        }
        pktData = nullptr;
    }


    bool RawDataTSLess::operator()(const RawData * r1, const RawData * r2) {
        if(r1->pktHeader.ts_sec == r2->pktHeader.ts_sec) {
            return r1->pktHeader.ts_usec < r2->pktHeader.ts_usec;
        }
        else {
            return r1->pktHeader.ts_sec < r2->pktHeader.ts_sec;
        }
    }


    bool RawDataTSGreater::operator()(const RawData * r1, const RawData * r2) {
        if(r1->pktHeader.ts_sec == r2->pktHeader.ts_sec) {
            return r1->pktHeader.ts_usec > r2->pktHeader.ts_usec;
        }
        else {
            return r1->pktHeader.ts_sec > r2->pktHeader.ts_sec;
        }
    }

    /*
    * class Packet functions
    */
    Packet::Packet():rawData(0), layer2Protocol(0), layer3Protocol(0), layer4Protocol(0), layer5Protocol(0),
        ptr_ethernetII(nullptr), ptr_ethernet8023(nullptr), ptr_vlan(nullptr), ptr_ipv4(nullptr),
        ptr_ipv6(nullptr), ptr_tcp(nullptr), ptr_udp(nullptr), ptr_data(nullptr), parse_error(0) {
    }

    //RawData * != nullptr
    Packet::Packet(RawData * & _rawData): rawData(_rawData->dirIndex), layer2Protocol(0), layer3Protocol(0), layer4Protocol(0),
        ptr_ethernetII(nullptr), ptr_ethernet8023(nullptr), ptr_vlan(nullptr), ptr_ipv4(nullptr),
        ptr_ipv6(nullptr), ptr_ipv6Ext(nullptr), ptr_tcp(nullptr), ptr_udp(nullptr), ptr_data(nullptr), parse_error(0) {
            memcpy(&(rawData.pktHeader), &(_rawData->pktHeader), sizeof(pcapPacketHeader_t));
            rawData.pktData = _rawData->pktData;
            _rawData->pktData = nullptr;
    }

    void Packet::Parse() {
        int parse_pointer = 0;

        if(rawData.pktData == nullptr) {
            parse_error = 1;
            return;
        }

        //parse layer 2
        if(rawData.pktHeader.caplen >= parse_pointer + sizeof(EthernetII_h)) {
            ptr_ethernetII = reinterpret_cast<EthernetII_h *>(rawData.pktData+parse_pointer);
            if(g_pcapByteOrder == TAP_BIG_ENDIAN) {
                EthernetII_hton(ptr_ethernetII);
            }

            // if(ptr_ethernetII->type <= 0x5DC) {     //IEEE 802.3
            //     if(rawData.pktHeader.caplen < parse_pointer + sizeof(EthernetII_h)) {
            //         parse_error = 1;
            //         return;
            //     }
            //     ptr_ethernet8023 = reinterpret_cast<Ethernet802_3_h *>(rawData.pktData+parse_pointer);
            //     ptr_ethernetII = nullptr;
            //     parse_pointer += sizeof(Ethernet802_3_h);
            //     layer2Protocol = ProtoType::EthernetDot3;
            // }
            // else {      //Ethernet II, most of packets are this protocol
                parse_pointer += sizeof(EthernetII_h);
                layer2Protocol = ProtoType::Ethernet;
            // }
        }
        else {
            parse_error = 1;
            return;
        }

        //parse VLAN
        if(layer2Protocol == ProtoType::Ethernet && 
        (ptr_ethernetII->type == ProtoType::MY_ETHPROTO_8021Q ||ptr_ethernetII->type == ProtoType::MY_ETHPROTO_VLAN1 
        || ptr_ethernetII->type == ProtoType::MY_ETHPROTO_VLAN2)) {
            if(rawData.pktHeader.caplen >= parse_pointer + sizeof(VLAN_h)) {
                ptr_vlan = reinterpret_cast<VLAN_h *>(rawData.pktData+parse_pointer);
                if(g_pcapByteOrder == TAP_BIG_ENDIAN) {
                    VLAN_hton(ptr_vlan);
                }
                
                std::swap(ptr_ethernetII->type, ptr_vlan->type);      //rewrite ethernet, for convenience
                parse_pointer += sizeof(VLAN_h);
            }
            else {
                parse_error = 1;
                return;
            }
        }

        //parse layer 3
        ParseLayer3(parse_pointer);
        if(parse_error == 1) {
            return;
        }

        //parse layer 4
        parseLayer4(parse_pointer);

        //parse data
        ptr_data = rawData.pktData+parse_pointer;

        //parse layer 5
        parseLayer5(parse_pointer);

        setPktLen();

        return;
    }

    void Packet::ParseLayer3(int & parse_pointer) {
        if(layer2Protocol == ProtoType::Ethernet && ptr_ethernetII->type == ProtoType::MY_ETHPROTO_IPV4)
        {
            //ipv4
            uint16_t tmp_fragOffset = 0;
            if(rawData.pktHeader.caplen >= parse_pointer + sizeof(IPv4_h)) {
                ptr_ipv4 = reinterpret_cast<IPv4_h *>(rawData.pktData+parse_pointer);
                if(g_pcapByteOrder == TAP_BIG_ENDIAN) {
                    IPv4_hton(ptr_ipv4);
                }
                tmp_fragOffset = (((uint16_t)(ptr_ipv4->flagOffset_1))<<8) + ((uint16_t)ptr_ipv4->offset_2);
                parse_pointer += (ptr_ipv4->versionLHL&0xf)*4;
                layer3Protocol = ProtoType::IPv4;
                if(tmp_fragOffset != 0) {
                    if(ptr_ipv4->protocol == ProtoType::MY_IPv4PROTOTCP) {
                        ptr_ipv4->protocol = ProtoType::MY_IPv4PROTOTCPData;
                    }
                    else if(ptr_ipv4->protocol == ProtoType::MY_IPv4PROTOTCP) {
                        ptr_ipv4->protocol = ProtoType::MY_IPv4PROTOUDPData;
                    }
                }
            }
            else {
                parse_error = 1;
                return;
            }
        }
        else if(layer2Protocol == ProtoType::Ethernet && ptr_ethernetII->type == ProtoType::MY_ETHPROTO_IPV6) {
            //ipv6
            if(rawData.pktHeader.caplen >= parse_pointer + sizeof(IPv6_h)) {
                ptr_ipv6 = reinterpret_cast<IPv6_h *>(rawData.pktData+parse_pointer);
                if(g_pcapByteOrder == TAP_BIG_ENDIAN) {
                    IPv6_hton(ptr_ipv6);
                }
                parse_pointer += sizeof(IPv6_h);
                layer3Protocol = ProtoType::IPv6;
            }
            else {
                parse_error = 1;
                return;
            }

            //ipv6 extension parse
            uint8_t tmp_nextHeader = ptr_ipv6->nextHeader;
            bool other_protoccol = false;
            uint8_t * tmp_fraPointer_1 = nullptr, *tmp_fraPointer_2 = nullptr;
            while(tmp_nextHeader != ProtoType::MY_IPv4PROTOUDP && tmp_nextHeader != ProtoType::MY_IPv4PROTOTCP) {
                if(rawData.pktHeader.caplen < parse_pointer + sizeof(IPv6Ext_h)) {
                    parse_error = 1;
                    return;
                }

                uint16_t tmp_fragmentOffset = 0;
                switch (tmp_nextHeader){
                    case ProtoType::MY_IPv6HopByHop:
                        ptr_ipv6Ext = reinterpret_cast<IPv6Ext_h *>(rawData.pktData+parse_pointer);
                        parse_pointer += (ptr_ipv6Ext->len+1)*8;
                        tmp_nextHeader = ptr_ipv6Ext->nextHeader;
                        break;
                    case ProtoType::MY_IPv6Routing:
                        ptr_ipv6Ext = reinterpret_cast<IPv6Ext_h *>(rawData.pktData+parse_pointer);
                        parse_pointer += (ptr_ipv6Ext->len+1)*8;
                        tmp_nextHeader = ptr_ipv6Ext->nextHeader;
                        break;
                    case ProtoType::MY_IPv6Fragmentation:
                        ptr_ipv6Ext = reinterpret_cast<IPv6Ext_h *>(rawData.pktData+parse_pointer);
                        tmp_fraPointer_1 = reinterpret_cast<uint8_t *>(ptr_ipv6Ext)+sizeof(IPv6Ext_h);
                        tmp_fraPointer_2 = reinterpret_cast<uint8_t *>(ptr_ipv6Ext)+sizeof(IPv6Ext_h)+sizeof(uint8_t);
                        tmp_fragmentOffset += ((((uint16_t)(*tmp_fraPointer_1))<<5) + (((uint16_t)(*tmp_fraPointer_2))>>3));
                        parse_pointer += 8;
                        if(tmp_fragmentOffset != 0 && ptr_ipv6Ext->nextHeader == ProtoType::MY_IPv4PROTOUDP) {
                            tmp_nextHeader = ProtoType::MY_IPv6NonFirstFragUDP;
                            other_protoccol = true;
                        }
                        else if(tmp_fragmentOffset != 0 && ptr_ipv6Ext->nextHeader == ProtoType::MY_IPv4PROTOTCP) {
                            tmp_nextHeader = ProtoType::MY_IPv6NonFirstFragTCP;
                            other_protoccol = true;
                        }
                        else {
                            tmp_nextHeader = ptr_ipv6Ext->nextHeader;
                        }
                        break;
                    case ProtoType::MY_IPv6NoNextHeader:
                        other_protoccol = true;
                        break;
                    case ProtoType::MY_IPv6Destination:
                        ptr_ipv6Ext = reinterpret_cast<IPv6Ext_h *>(rawData.pktData+parse_pointer);
                        parse_pointer += (ptr_ipv6Ext->len+1)*8;
                        tmp_nextHeader = ptr_ipv6Ext->nextHeader;
                        break;
                    default:
                        other_protoccol = true;
                        break;
                }

                if(other_protoccol) {
                    break;
                }

            }
            ptr_ipv6->nextHeader = tmp_nextHeader;
        }

    }

    void Packet::parseLayer4(int & parse_pointer) {
        if(layer3Protocol != ProtoType::IPv4 && layer3Protocol != ProtoType::IPv6) {
            return;
        }
        
        if((layer3Protocol == ProtoType::IPv4&&ptr_ipv4->protocol == ProtoType::MY_IPv4PROTOTCP) || (layer3Protocol == ProtoType::IPv6&&ptr_ipv6->nextHeader == ProtoType::MY_IPv4PROTOTCP)) {
            if(rawData.pktHeader.caplen >= parse_pointer + sizeof(TCP_h)) {
                ptr_tcp = reinterpret_cast<TCP_h *>(rawData.pktData+parse_pointer);
                if(g_pcapByteOrder == TAP_BIG_ENDIAN) {
                    TCP_hton(ptr_tcp);
                }
                parse_pointer += sizeof(TCP_h);
                layer4Protocol = ProtoType::TCP;
            }
            else {
                parse_error = 1;
                return;
            }
        }
        else if((layer3Protocol == ProtoType::IPv4&&ptr_ipv4->protocol == ProtoType::MY_IPv4PROTOUDP) || (layer3Protocol == ProtoType::IPv6&&ptr_ipv6->nextHeader == ProtoType::MY_IPv4PROTOUDP)) {
            if(rawData.pktHeader.caplen >= parse_pointer + sizeof(UDP_h)) {
                ptr_udp = reinterpret_cast<UDP_h *>(rawData.pktData+parse_pointer);
                if(g_pcapByteOrder == TAP_BIG_ENDIAN) {
                    UDP_hton(ptr_udp);
                }
                parse_pointer += sizeof(UDP_h);
                layer4Protocol = ProtoType::UDP;
            }
            else {
                parse_error = 1;
                return;
            }
        }
        else if((layer3Protocol == ProtoType::IPv6&&ptr_ipv6->nextHeader == ProtoType::MY_IPv6NonFirstFragUDP)) {
            layer4Protocol = ProtoType::UDPData;
        }
        else if((layer3Protocol == ProtoType::IPv6&&ptr_ipv6->nextHeader == ProtoType::MY_IPv6NonFirstFragTCP)) {
            layer4Protocol = ProtoType::TCPData;
        }
        else if((layer3Protocol == ProtoType::IPv4&&ptr_ipv4->protocol == ProtoType::MY_IPv4PROTOUDPData)) {
            layer4Protocol = ProtoType::UDPData;
        }
        else if((layer3Protocol == ProtoType::IPv4&&ptr_ipv4->protocol == ProtoType::MY_IPv4PROTOTCPData)) {
            layer4Protocol = ProtoType::TCPData;
        }
    }

    void Packet::parseLayer5(int & parse_pointer) {
        if(layer4Protocol == ProtoType::UDP && (ptr_udp->dstPort == 53 || ptr_udp->srcPort == 53)) {
            layer5Protocol = ProtoType::DNS;
        }
        else if(layer4Protocol == ProtoType::TCP && (ptr_tcp->srcPort == 20 || ptr_tcp->dstPort == 20 ||
        ptr_tcp->srcPort == 21 || ptr_tcp->dstPort == 21)) {
            layer5Protocol = ProtoType::FTP;
        }
        else if(layer4Protocol == ProtoType::TCP && (ptr_tcp->srcPort == 25 || ptr_tcp->dstPort == 25)) {
            layer5Protocol = ProtoType::SMTP;
        }
        else if(layer4Protocol == ProtoType::TCP && (ptr_tcp->srcPort == 110 || ptr_tcp->dstPort == 110)) {
            layer5Protocol = ProtoType::POP3;
        }
        else if(layer4Protocol == ProtoType::TCP && (ptr_tcp->srcPort == 143 || ptr_tcp->dstPort == 143)) {
            layer5Protocol = ProtoType::IMAP;
        }
        else if(layer4Protocol == ProtoType::UDP && ((ptr_udp->dstPort == 67 && ptr_udp->srcPort == 68) ||
        (ptr_udp->dstPort == 68 && ptr_udp->srcPort == 67))) {
            layer5Protocol = ProtoType::DHCP;
        }
        else if(layer4Protocol == ProtoType::TCP && (ptr_tcp->srcPort == 80 || ptr_tcp->dstPort == 80)) {
            layer5Protocol = ProtoType::HTTP;
        }
        else if(layer4Protocol == ProtoType::TCP && (ptr_tcp->srcPort == 443 || ptr_tcp->dstPort == 443)) {
            layer5Protocol = ProtoType::HTTPS;
        }
        else {
            layer5Protocol = ProtoType::UnknownProtocol;
        }
    }


    void Packet::setPktLen() {
        uint16_t tmp_len = 0;
        if(parse_error) {
            pkt_len = 0;
            return;
        }

        pkt_len = rawData.pktHeader.len;
        if(ptr_ethernet8023 != nullptr) {
            tmp_len += sizeof(Ethernet802_3_h);
        }
        if(ptr_ethernetII != nullptr) {
            tmp_len += sizeof(EthernetII_h);
        }
        if(ptr_vlan != nullptr) {
            tmp_len += sizeof(VLAN_h);
        }
        if(ptr_ipv4 != nullptr) {
            tmp_len += sizeof(IPv4_h);
            tmp_len += ptr_ipv4->totalLength;
        }
        if(ptr_ipv6 != nullptr) {
            tmp_len += sizeof(IPv6_h);
            tmp_len += ptr_ipv6->payloadLegth;
        }

        if(tmp_len > pkt_len) {
            pkt_len = tmp_len;
        }
    }

    Packet::~Packet() {}
}