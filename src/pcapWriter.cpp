/*
* author: Wenrui Liu
* last edit time: 2023-4-21
*/
#include<pcapWriter.h>
#include<util.h>
#include <sstream>
#include <stdlib.h>
extern uint8_t m_writeBuffer[];
extern size_t m_bufferPointer;
extern MiniTAP::pcapFileHeader_t g_pcapFileHdr;

namespace MiniTAP{
    
    PcapWriter::PcapWriter(const std::string & _config_path): m_writeDirName{}, bgts_sec(0), bgts_usec(0), edts_sec(0), edts_usec(0), targetIP{} {
            std::ifstream is(_config_path);
            is >> m_writeDirName;
            is.close();
            m_bufferPointer = 0;
        }
    
    int PcapWriter::pass(const Packet * pkt) {
        if(writeContentSelect == WRITE_FIVETUPLE) {
            return writeFiveTuple(pkt);
        }
        else if(writeContentSelect == WRITE_DNSPCAP) {
            return writeDNSPcap(pkt);
        }
        else if(writeContentSelect == WRITE_SPECIFICIP){
            return writeSpecificIP(pkt);
        }
        return 0;
    }

    void PcapWriter::initTargetIP(const std::string & _config_path){
        std::ifstream file(_config_path);
        if (!file.is_open()) {
            printf("Error: Unable to open file.\n");
            return ;
        }
        std::string line;
        std::string str;
        while (std::getline(file, str, ',')) {
            PcapWriter::targetIP.insert(std::atoi(str.c_str()));
        }
        file.close();
    }

    int PcapWriter::finish() {
        if(m_bufferPointer > 0) {
            std::string path = m_writeDirName + "/tap_" + std::to_string(bgts_sec) + "_" + 
                std::to_string(bgts_usec) + "_" + std::to_string(edts_sec) + "_" + std::to_string(edts_usec) + ".dat";
            FILE * fp = fopen(path.c_str(), "wb");
            fwrite(m_writeBuffer, sizeof(uint8_t), m_bufferPointer, fp);
            fclose(fp);
            printf("write file: %s\n", path.c_str());
        }
        return 0;
    }

    int PcapWriter::writeFiveTuple(const Packet * pkt) {
        if(pkt != nullptr && pkt->layer3Protocol == ProtoType::IPv4 && (pkt->layer4Protocol == ProtoType::TCP||pkt->layer4Protocol == ProtoType::UDP)) {
            if(m_bufferPointer >= expectFileSize) {
                std::string path = m_writeDirName + "/tap_" + std::to_string(bgts_sec) + "_" + 
                    std::to_string(bgts_usec) + "_" + std::to_string(edts_sec) + "_" + std::to_string(edts_usec) + ".dat";
                FILE * fp = fopen(path.c_str(), "wb");
                fwrite(m_writeBuffer, sizeof(uint8_t), m_bufferPointer, fp);
                fclose(fp);
                printf("write file: %s\n", path.c_str());
                m_bufferPointer = 0;
            }

            if(m_bufferPointer == 0) {
                memset(m_writeBuffer, 0, writeBufferSize);
                bgts_sec = pkt->rawData.pktHeader.ts_sec;
                bgts_usec = pkt->rawData.pktHeader.ts_usec;
            }

            //copy five tuple: 4 bytes srcIP, 4 bytes dstIP, 2 bytes srcPort, 2 bytes dstPort, 1 byte Protocol, 8 bytes ts
            memcpy(m_writeBuffer+m_bufferPointer, &(pkt->ptr_ipv4->srcIP), sizeof(uint32_t));
            m_bufferPointer += sizeof(uint32_t);
            memcpy(m_writeBuffer+m_bufferPointer, &(pkt->ptr_ipv4->dstIP), sizeof(uint32_t));
            m_bufferPointer += sizeof(uint32_t);
            if(pkt->layer4Protocol == ProtoType::TCP) {
                memcpy(m_writeBuffer+m_bufferPointer, &(pkt->ptr_tcp->srcPort), sizeof(uint16_t));
                m_bufferPointer += sizeof(uint16_t);
                memcpy(m_writeBuffer+m_bufferPointer, &(pkt->ptr_tcp->dstPort), sizeof(uint16_t));
                m_bufferPointer += sizeof(uint16_t);
            }
            else {
                memcpy(m_writeBuffer+m_bufferPointer, &(pkt->ptr_udp->srcPort), sizeof(uint16_t));
                m_bufferPointer += sizeof(uint16_t);
                memcpy(m_writeBuffer+m_bufferPointer, &(pkt->ptr_udp->dstPort), sizeof(uint16_t));
                m_bufferPointer += sizeof(uint16_t);
            }
            memcpy(m_writeBuffer+m_bufferPointer, &(pkt->ptr_ipv4->protocol), sizeof(uint8_t));
            m_bufferPointer += sizeof(uint8_t);
            memcpy(m_writeBuffer+m_bufferPointer, &(pkt->rawData.pktHeader.ts_sec), sizeof(uint32_t));
            m_bufferPointer += sizeof(uint32_t);
            memcpy(m_writeBuffer+m_bufferPointer, &(pkt->rawData.pktHeader.ts_usec), sizeof(uint32_t));
            m_bufferPointer += sizeof(uint32_t);

            edts_sec = pkt->rawData.pktHeader.ts_sec;
            edts_usec = pkt->rawData.pktHeader.ts_usec;
        }
        return 0;
    }
    int a = 0;
    int PcapWriter::writePcap(const Packet * pkt) {
            a++;
            if(a % 1000 == 0) printf("pcapwriter: %d, %d : %d\n", a, m_bufferPointer, expectFileSize);
            if(m_bufferPointer >= expectFileSize) {
                std::string path = m_writeDirName + "/tap_" + std::to_string(bgts_sec) + "_" + 
                    std::to_string(bgts_usec) + "_" + std::to_string(edts_sec) + "_" + std::to_string(edts_usec) + ".dat";
                FILE * fp = fopen(path.c_str(), "wb");
                fwrite(m_writeBuffer, sizeof(uint8_t), m_bufferPointer, fp);
                fclose(fp);
                printf("write file: %s\n", path.c_str());
                m_bufferPointer = 0;
            }

            if(m_bufferPointer == 0) {
                memset(m_writeBuffer, 0, writeBufferSize);
                memcpy(m_writeBuffer, &g_pcapFileHdr, sizeof(pcapFileHeader_t));
                m_bufferPointer += sizeof(pcapFileHeader_t);
                bgts_sec = pkt->rawData.pktHeader.ts_sec;
                bgts_usec = pkt->rawData.pktHeader.ts_usec;
            }

            //restore data
            if(pkt->ptr_vlan != nullptr) {
                std::swap(pkt->ptr_ethernetII->type, pkt->ptr_vlan->type);
                if(g_pcapByteOrder == TAP_BIG_ENDIAN) {
                    VLAN_hton(pkt->ptr_vlan);
                }
            }
            if(g_pcapByteOrder == TAP_BIG_ENDIAN) {
                EthernetII_hton(pkt->ptr_ethernetII);
                IPv4_hton(pkt->ptr_ipv4);
                IPv6_hton(pkt->ptr_ipv6);
                UDP_hton(pkt->ptr_udp);
                TCP_hton(pkt->ptr_tcp);
            }
            memcpy(m_writeBuffer + m_bufferPointer, &(pkt->rawData.pktHeader), sizeof(pcapPacketHeader_t));
            m_bufferPointer += sizeof(pcapPacketHeader_t);
            memcpy(m_writeBuffer + m_bufferPointer, pkt->rawData.pktData, pkt->rawData.pktHeader.caplen);
            m_bufferPointer += pkt->rawData.pktHeader.caplen;

            edts_sec = pkt->rawData.pktHeader.ts_sec;
            edts_usec = pkt->rawData.pktHeader.ts_usec;
    }

    int PcapWriter::writeDNSPcap(const Packet * pkt) {
        if(pkt != nullptr && pkt->layer3Protocol == ProtoType::IPv4 && pkt->layer4Protocol == ProtoType::UDP &&
            (pkt->ptr_udp->srcPort == 53 || pkt->ptr_udp->dstPort == 53) && pkt->ptr_ipv4->srcIP) {
                // printf("found DNS pcap!!!\n");
                writePcap(pkt);
        }
        return 0;
    }

    // int PcapWriter::writeSpecificIP(const Packet * pkt) {
    //     uint32_t IP_1 = 0X7482e400, mask_1 = 0xffffff00;        // 116.130.228.X
    //     uint32_t IP_2 = 0X7c5fe000, mask_2 = 0xffffff00;        // 124.95.224.X
    //     uint32_t IP_3 = 0X7d270300, mask_3 = 0xffffff00;        // 125.39.3.X
    //     uint32_t IP_4 = 0Xda0c4700, mask_4 = 0xffffff00;        // 218.12.71.X
    //     if(pkt->layer3Protocol == ProtoType::IPv4 && ((pkt->ptr_ipv4->srcIP & mask_1 == IP_1 || pkt->ptr_ipv4->dstIP & mask_1 == IP_1)
    //      || (pkt->ptr_ipv4->srcIP & mask_2 == IP_2 || pkt->ptr_ipv4->dstIP & mask_2 == IP_2)
    //      || (pkt->ptr_ipv4->srcIP & mask_3 == IP_3 || pkt->ptr_ipv4->dstIP & mask_3 == IP_3)
    //      || (pkt->ptr_ipv4->srcIP & mask_4 == IP_4 || pkt->ptr_ipv4->dstIP & mask_4 == IP_4))) {
    //             writePcap(pkt);
    //     }
    // }

    int PcapWriter::writeSpecificIP(const Packet * pkt) {
        if(pkt != nullptr && pkt->layer3Protocol == ProtoType::IPv4 &&
           (targetIP.find(pkt->ptr_ipv4->srcIP) != targetIP.end() || targetIP.find(pkt->ptr_ipv4->dstIP) != targetIP.end())) {
                // printf("found DNS pcap!!!\n");

                writePcap(pkt);

        }
        return 0;
    }


    PcapWriter::~PcapWriter() {}
}


