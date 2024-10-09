/*
* author: Wenrui Liu
* last edit time: 2023-3-29
*/
#ifndef _MINITAP_PROTOCOLHEADER_H_
#define _MINITAP_PROTOCOLHEADER_H_

#include<stdint.h>

namespace MiniTAP{

    /*
    * Layer 2, protocol header
    */
    #pragma pack(1)
    struct EthernetII_hdr {
        uint8_t dstMAC[6];
        uint8_t srcMAX[6];
        uint16_t type;      //type <= 0x5DC IEEE 802.3, >= 0x0600 Ethernet II
    };
    #pragma pack()

    typedef struct EthernetII_hdr EthernetII_h;

    #pragma pack(1)
    struct Ethernet802_3_hdr {
        uint8_t dstMAC[6];
        uint8_t srcMAC[6];
        uint16_t length;      
        uint8_t LLC;
        uint8_t SNAP;
    };
    #pragma pack()

    typedef struct Ethernet802_3_hdr Ethernet802_3_h;

    #pragma pack(1)
    struct VLAN_hdr {
        uint8_t pri_cfi_vid;
        uint8_t vid;
        uint16_t type;
    };
    #pragma pack()

    typedef struct VLAN_hdr VLAN_h;

    #pragma pack(1)
    struct IPv4_hdr {
        uint8_t versionLHL;
        uint8_t tos;
        uint16_t totalLength;
        uint16_t identification;
        uint8_t flagOffset_1;
        uint8_t offset_2;
        uint8_t ttl;
        uint8_t protocol;
        uint16_t checksum;
        uint32_t srcIP;
        uint32_t dstIP;
    };
    #pragma pack()

    typedef struct IPv4_hdr IPv4_h;

    #pragma pack(1)
    struct IPv6_hdr {
        uint32_t basicLabel;
        uint16_t payloadLegth;
        uint8_t nextHeader;
        uint8_t hopLimit;
        uint8_t srcIP[16];
        uint8_t dstIP[16];
    };
    #pragma pack()

    typedef struct IPv6_hdr IPv6_h;

    #pragma pack(1)
    struct IPv6Ext_hdr{
        uint8_t nextHeader;
        uint8_t len;
    };
    #pragma pack()
    typedef struct IPv6Ext_hdr IPv6Ext_h;

    #pragma pack(1)
    struct UDP_hdr {
        uint16_t srcPort;
        uint16_t dstPort;
        uint16_t totalLength;
        uint16_t checksum;
    };
    #pragma pack()

    typedef struct UDP_hdr UDP_h;

    #pragma pack(1)
    struct TCP_hdr {
        uint16_t srcPort;
        uint16_t dstPort;
        uint32_t seqNum;
        uint32_t ackNum;
        uint16_t flags;
        uint16_t windowSize;
        uint16_t checksum;
        uint16_t pointer;
    };
    #pragma pack()

    typedef struct TCP_hdr TCP_h;
}

#endif