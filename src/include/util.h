/*
* author: Wenrui Liu
* last edit time: 2023-4-9
*/
#ifndef _MINITAP_UTIL_H_
#define _MINITAP_UTIL_H_

#include<protocolHeader.h>
#include<common.h>


namespace MiniTAP{
    inline uint32_t hton_32b(uint32_t hostlong){
        uint32_t retVal = 0;
        retVal = (hostlong << 24) + ((hostlong << 8) & 0x00ff0000) +
         ((hostlong >> 8) & 0x0000ff00) + ((hostlong >> 24) & 0xff);
        return retVal;
    }

    inline uint16_t hton_16b(uint16_t hostshort) {
        uint16_t retVal = 0;
        retVal = (hostshort << 8) + (hostshort >> 8);
        return retVal;
    }

    inline void EthernetII_hton(EthernetII_h * hdr) {
        if(hdr == nullptr) {
            return;
        }
        hdr->type = hton_16b(hdr->type);
    }

    // inline void VLAN_hton(VLAN_h * hdr);

    inline void IPv4_hton(IPv4_h * hdr) {
        if(hdr == nullptr) {
            return;
        }
        hdr->totalLength = hton_16b(hdr->totalLength);
        hdr->identification = hton_16b(hdr->identification);
        hdr->checksum = hton_16b(hdr->checksum);
        hdr->srcIP = hton_32b(hdr->srcIP);
        hdr->dstIP = hton_32b(hdr->dstIP);
    }

    inline void IPv6_hton(IPv6_h * hdr) {
        if(hdr == nullptr) {
            return;
        }
        hdr->payloadLegth = hton_16b(hdr->payloadLegth);
    }

    inline void TCP_hton(TCP_h * hdr) {
        if(hdr == nullptr) {
            return;
        }
        hdr->srcPort = hton_16b(hdr->srcPort);
        hdr->dstPort = hton_16b(hdr->dstPort);
        hdr->seqNum = hton_32b(hdr->seqNum);
        // hdr->ackNum = hton_32b(hdr->ackNum);
        // printf("%u\n", hton_16b(hdr->windowSize));
        hdr->windowSize = hton_16b(hdr->windowSize);
        // hdr->flags = hton_16b(hdr->flags);
        // hdr->checksum = hton_16b(hdr->checksum);
        // hdr->pointer = hton_16b(hdr->pointer);
    }

    inline void UDP_hton(UDP_h * hdr) {
        if(hdr == nullptr) {
            return;
        }
        hdr->srcPort = hton_16b(hdr->srcPort);
        hdr->dstPort = hton_16b(hdr->dstPort);
        hdr->totalLength = hton_16b(hdr->totalLength);
        // hdr->checksum = hton_16b(hdr->checksum);
    }

    inline void VLAN_hton(VLAN_h * hdr) {
        if(hdr == nullptr) {
            return;
        }
        hdr->type = hton_16b(hdr->type);
    }

    struct FiveTupleHash{
        std::size_t operator()(const fiveTuple_t & ft) const;
    };

    struct TwoTupleHash{
        std::size_t operator()(const twoTuple_t & ft) const;
    };

    inline bool TS_bigger_than(uint32_t ts_1_sec, uint32_t ts_1_usec, uint32_t ts_2_sec, uint32_t ts_2_usec) {
        if(ts_1_sec > ts_2_sec) {
            return true;
        }
        else {
            return ts_1_usec > ts_2_usec;
        }
    }

    inline bool TS_biggerEqual_than(uint32_t ts_1_sec, uint32_t ts_1_usec, uint32_t ts_2_sec, uint32_t ts_2_usec) {
        if(ts_1_sec > ts_2_sec) {
            return true;
        }
        else {
            return ts_1_usec >= ts_2_usec;
        }
    }

    inline uint64_t TS_diff_microSec(uint32_t ts_1_sec, uint32_t ts_1_usec, uint32_t ts_2_sec, uint32_t ts_2_usec) {
        uint64_t ts_1_msec = (uint64_t)ts_1_sec*1000000 + (uint64_t)ts_1_usec;
        uint64_t ts_2_msec = (uint64_t)ts_2_sec*1000000 + (uint64_t)ts_2_usec;
        return ts_1_msec - ts_2_msec;
    }

#ifdef _WIN32
    LPWSTR s2LPWSTR(const std::string& s);
#endif
}

#endif