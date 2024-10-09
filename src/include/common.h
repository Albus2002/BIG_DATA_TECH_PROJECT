/*
* author: Wenrui Liu
* last edit time: 2023-4-22
*/
#ifndef _MINITAP_COMMON_H_
#define _MINITAP_COMMON_H_

#include<cstdint>
#include<cstdio>
#include<cstring>
#include<string>
#include<vector>
#include<queue>
#include<fstream>
#include<unordered_map>
#include<map>

#include<stdlib.h>
#include<ctime>
#if defined(_WIN32)
#include<io.h>
#include<windows.h>
#elif defined(__linux__)
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<unistd.h>
#endif

// #define _DEBUG_MODE_
#define _RECORD_MODE_
// #define _WRITE_MODE_

const int TAP_BIG_ENDIAN = 0;
const int TAP_LITTLE_ENDIAN = 1;

const int WRITE_FIVETUPLE = 1;
const int WRITE_DNSPCAP = 2;
const int WRITE_SPECIFICIP = 3;

const uint32_t heavyHitterThreshold = 100000;  //100K
const uint64_t recordInterval = 1000000;      //1s, units: us
const uint64_t IPv4DstRecordThreshold = 100000;      // 100 KB
const uint64_t recordFlushCacheInterval = 30 * 60 * 1000000;   //30min
const uint64_t recordFlushFlowThreshold = recordFlushCacheInterval / 2;   //15min

const bool g_fastReadByFileMapping = true;

const int writeContentSelect = WRITE_SPECIFICIP;
// const int writeContentSelect = WRITE_DNSPCAP;
const size_t writeBufferSize = 505*1024*1024;
const size_t expectFileSize = 500*1024*1024;


//default: pcap big_endian, host little_endian
extern int g_pcapByteOrder;
extern bool g_pcapByteOrderDecided;
extern bool g_pcapByteOrderError;

// main namespace of this project
namespace MiniTAP{
    //24 bytes pcap file header
    #pragma pack(1)
    struct pcapFileHeader
    {
        uint32_t magic;     //for plantform byte-order: big 0xa1b2c3d4, small 0xd4c3b2a1
        uint16_t major;
        uint16_t minor;
        uint32_t thisZone;
        uint32_t sigFigs;
        uint32_t snapLen;
        uint32_t linkType;
    };
    #pragma pack()

    typedef struct pcapFileHeader pcapFileHeader_t;

    #pragma pack(1)
    struct pcapPacketHeader
    {
        uint32_t ts_sec;
        uint32_t ts_usec;
        uint32_t caplen;        //capture len
        uint32_t len;           //real len in line
    };
    #pragma pack()

    typedef struct pcapPacketHeader pcapPacketHeader_t;


    struct fiveTuple {
        uint32_t srcIP;
        uint32_t dstIP;
        uint16_t srcPort;
        uint16_t dstPort;
        uint16_t protocol;

        fiveTuple();
        fiveTuple(uint32_t _srcIP, uint32_t _dstIP, uint16_t _srcPort, uint16_t _dstPort, uint16_t _protocol);
        bool operator==(const fiveTuple & b) const;
    };

    typedef struct fiveTuple fiveTuple_t;

    struct packetDetail{
        uint32_t arrTime_sec;
        uint32_t arrTime_usec;
        uint64_t byteCount;
        uint64_t seq_num;
        uint64_t window_size;

        // 默认构造函数，初始化所有属性为0
        packetDetail() 
            : arrTime_sec(0), arrTime_usec(0), byteCount(0), seq_num(0), window_size(0) {}

        // 参数化构造函数，允许指定属性的值
        packetDetail(uint32_t sec, uint32_t usec, uint64_t byteCnt, uint64_t sq, uint64_t ws)
            : arrTime_sec(sec), arrTime_usec(usec), byteCount(byteCnt), seq_num(sq), window_size(ws) {}

        // 相等比较函数
        bool operator==(const packetDetail& other) const {
            return arrTime_sec == other.arrTime_sec &&
                arrTime_usec == other.arrTime_usec &&
                byteCount == other.byteCount;
        }

        // 如果需要不等比较函数，可以添加如下：
        bool operator!=(const packetDetail& other) const {
            return !(*this == other);
        }
    };

    struct twoTuple{
        uint32_t srcIP;
        uint32_t dstIP;

        bool operator==(const twoTuple & b) const;
    };

    typedef struct twoTuple twoTuple_t;

    struct burstInfo{
        std::pair <uint32_t, uint32_t> timeSpan_sec;
        std::pair <uint32_t, uint32_t> timeSpan_usec;
        uint64_t packetNum;
        uint64_t byteCount;

        burstInfo();
    };

    typedef burstInfo burstInfo_t;

    struct fiveTupleInfo{
        uint32_t packetTotalNum;
        uint32_t packetTotalSize;
        uint32_t bgts_sec;
        uint32_t bgts_usec;
        uint32_t edts_sec;
        uint32_t edts_usec;
        std::vector<burstInfo> burstStatics;
        uint64_t lst_pkt_size;

        bool isHeavyHitter;

        fiveTupleInfo();
        uint64_t getDurationSec();
        uint64_t getDurationMilliSec();
        uint64_t getDurationMicroSec();
    };

    typedef struct fiveTupleInfo fiveTupleInfo_t;

    struct twoTupleInfo{
        uint16_t pkt_cnt;
    };

    typedef struct twoTupleInfo twoTupleInfo_t;

    fiveTuple_t TCPStreamFiveTupleConvert(fiveTuple_t ft);
    struct TCPPktInfo {
        uint32_t ts_sec;
        uint32_t ts_usec;
        uint32_t seqNum;
        uint32_t ackNum;
        uint16_t flags;
        uint16_t windowSize;
        uint16_t pkt_cnt;
        
        TCPPktInfo();
        TCPPktInfo(uint32_t _ts_sec, uint32_t _ts_usec, uint32_t _seqNum,
            uint32_t _ackNum, uint16_t _flags, uint16_t _windowSize, uint16_t _pkt_cnt);
    };

    typedef struct TCPPktInfo TCPPktInfo_t;
    typedef std::pair<uint64_t, uint64_t> puu;
    struct usr_traffic_detail{
        std::uint32_t IP;
        std::vector<puu> activeFlow;
        std::vector<puu> actAdd;
        std::vector<puu> actRmv;

        usr_traffic_detail();
        usr_traffic_detail(uint32_t ip);
    };

    struct actDetail{
        fiveTuple_t ft;
        uint64_t timeStamp; // usec
        bool flag;
        actDetail(fiveTuple_t _ft, uint64_t _ts, uint64_t ff):ft(_ft), timeStamp(_ts), flag(ff){};
    };

    struct  trafficInfo{
        std::vector<uint32_t> throughput;
        std::vector<uint32_t> activeFlow;
        std::vector<puu> actAdd;
        std::vector<puu> actRmv;
        std::unordered_map<uint32_t, uint32_t> packetSizeCount;     //size-count
        std::unordered_map<uint32_t, uint32_t> heavyFlowPacketNumCount;     //flowPacketNum-count
        std::unordered_map<uint32_t, uint32_t> littleFlowPacketNumCount;
        std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> layer5Count;
        std::vector<uint64_t> heavyFlowDuration;     //MicroSec
        std::vector<uint64_t> littleFlowDuration;     //MicroSec
        std::unordered_map<uint32_t, uint32_t> pktInterArrivalTime;    //MicroSec
        std::unordered_map<uint32_t, uint32_t> flowInterArrivalTime;    //MicroSec
        std::unordered_map<uint32_t, uint64_t> IPv4DstByteCount;    //dstIP-size
        std::unordered_map<uint32_t, usr_traffic_detail> usrTrafficDetail;
        std::vector<actDetail> activeFlowDetail;

        uint64_t totalPacketNum;
        uint64_t totalThroughput;

        uint64_t ipv4PacketNum;
        uint64_t ipv6PacketNum;
        uint64_t ipv4Throughput;
        uint64_t ipv6Throughput;

        uint64_t ipv4_tcpPacketNum;
        uint64_t ipv4_udpPacketNum;
        uint64_t ipv4_tcpThroughput;
        uint64_t ipv4_udpThroughput;
        uint64_t ipv6_tcpPacketNum;
        uint64_t ipv6_udpPacketNum;
        uint64_t ipv6_tcpThroughput;
        uint64_t ipv6_udpThroughput;

        uint64_t heavyFlowPacketNum;
        uint64_t littleFlowPacketNum;
        uint64_t heavyFlowThroughput;
        uint64_t littleFlowThroughput;

        trafficInfo();
    };

    typedef struct trafficInfo trafficInfo_t;

    struct FileManager{
        FILE * throughputFile;
        FILE * activeFlowFile;
        FILE * protocolCountFile;
        FILE * layer5ProtoCountFile;
        FILE * packetSizeFile;
        FILE * heavyLittleFlowSizeFile;
        FILE * fiveTupleFile;
        FILE * heavyFlowPktSizeFile;
        FILE * littleFlowPktSizeFile;
        FILE * heavyFlowDurationFile;
        FILE * littleFlowDurationFile;
        FILE * pktInterTimeFile;
        FILE * flowInterTimeFile;
        FILE * IPv4DstByteFile;
        FILE * burstDurationFile;
        FILE * burstPacketNumFile;
        FILE * burstByteCountFile;
        FILE * TCPCongestionInfoFile;
        FILE * topkFlowFile;
        FILE * topkFlowDetailFile;
        FILE * activeFlowAddFile;
        FILE * activeFlowRmvFile;
        FILE * activeFlowUserFile;
        FILE * activeFlowAddUserFile;
        FILE * activeFlowRmvUserFile;
        FILE * activeFlowDetailFile;
        FileManager();
    };
    
}

#endif