/*
* author: Wenrui Liu
* last edit time: 2023-4-22
*/
#ifndef _MINITAP_PACKET_H_
#define _MINITAP_PACKET_H_

#include<common.h>
#include<protocolHeader.h>
#include<protocolType.h>
#include<queue>
#include<mutex>

namespace MiniTAP{

    // raw data of a pcap packet
    class RawData{
        public:
            pcapPacketHeader_t pktHeader;
            uint8_t * pktData;
            int dirIndex;
            
            RawData(const int _dirIndex);
            RawData(const pcapPacketHeader_t & _pktHeader, const int _dirIndex);
            size_t readPktData(FILE * fp);
            size_t fastReadPktData(uint8_t * mapData, uint64_t & mapPointer);
            ~RawData();
    };

    struct RawDataTSLess{
        bool operator()(const RawData * r1, const RawData * r2);
    };

    struct RawDataTSGreater{
        bool operator()(const RawData * r1, const RawData * r2);
    };

    
    //packet used for switch process and analysis
    class Packet{
        public:
            RawData rawData;

            ProtoType::ProtocolType layer2Protocol;
            ProtoType::ProtocolType layer3Protocol;
            ProtoType::ProtocolType layer4Protocol;
            ProtoType::ProtocolType layer5Protocol;

            EthernetII_h * ptr_ethernetII;
            Ethernet802_3_h * ptr_ethernet8023;
            VLAN_h * ptr_vlan;
            IPv4_h * ptr_ipv4;
            IPv6_h * ptr_ipv6;
            IPv6Ext_h * ptr_ipv6Ext;
            TCP_h * ptr_tcp;
            UDP_h * ptr_udp;
            uint8_t * ptr_data;

            int parse_error;
            uint16_t pkt_len;
            uint16_t pkt_cnt;//count of processing,for now is 2,fiveTuple&&traffic record.

            Packet();
            //rawData will never be used again
            Packet(RawData * & rawData);
            void Parse();
            ~Packet();
        
        private:
            void ParseLayer3(int & parse_pointer);
            void parseLayer4(int & parse_pointer);
            void parseLayer5(int & parse_pointer);

            void setPktLen();
    };
    
}

#endif