/*
* author: Wenrui Liu
* last edit time: 2023-4-4
*/
#include<common.h>

int g_pcapByteOrder = TAP_BIG_ENDIAN;
bool g_pcapByteOrderDecided = false;
bool g_pcapByteOrderError = false;

uint8_t m_writeBuffer[writeBufferSize];
size_t m_bufferPointer = 0;

namespace MiniTAP{
    fiveTuple::fiveTuple(uint32_t _srcIP, uint32_t _dstIP, uint16_t _srcPort, 
    uint16_t _dstPort, uint16_t _protocol): srcIP(_srcIP), dstIP(_dstIP), srcPort(_srcPort),
    dstPort(_dstPort), protocol(_protocol){}

    fiveTuple::fiveTuple(): srcIP(0), dstIP(0), srcPort(0),
    dstPort(0), protocol(0){}

    bool fiveTuple::operator==(const fiveTuple & b) const{
        if(srcIP == b.srcIP && dstIP == b.dstIP && srcPort == b.srcPort && 
        dstPort == b.dstPort && protocol == b.protocol) {
            return true;
        }
        return false;
    }

    fiveTuple_t TCPStreamFiveTupleConvert(fiveTuple_t ft) {
        if (ft.srcIP > ft.dstIP || (ft.srcIP == ft.dstIP && ft.srcPort > ft.dstPort)) {
            std::swap(ft.srcIP, ft.dstIP);
            std::swap(ft.srcPort, ft.dstPort);
        }
        return ft;
    }

    TCPPktInfo::TCPPktInfo() { }
    TCPPktInfo::TCPPktInfo(uint32_t _ts_sec, uint32_t _rs_usec, uint32_t _seqNum,
        uint32_t _ackNum, uint16_t _flags, uint16_t _windowSize, uint16_t _pkt_cnt)
        : ts_sec(_ts_sec), ts_usec(ts_usec), seqNum(_seqNum), 
        ackNum(_ackNum), flags(_flags), windowSize(_windowSize), pkt_cnt(_pkt_cnt) { }
    
    bool twoTuple::operator==(const twoTuple & b) const{
        if(srcIP == b.srcIP && dstIP == b.dstIP) {
            return true;
        }
        return false;
    }

    usr_traffic_detail::usr_traffic_detail(): IP(0), activeFlow{}, actAdd{}, actRmv{}{};
    usr_traffic_detail::usr_traffic_detail(uint32_t ip): IP(ip), activeFlow{}, actAdd{}, actRmv{}{};

    trafficInfo::trafficInfo(): throughput{}, activeFlow{}, packetSizeCount{},
        heavyFlowDuration{}, littleFlowDuration{}, pktInterArrivalTime{}, flowInterArrivalTime{},
        totalPacketNum(0), totalThroughput(0), IPv4DstByteCount{},
        ipv4PacketNum(0), ipv4Throughput(0), ipv6PacketNum(0), ipv6Throughput(0),
        ipv4_tcpPacketNum(0), ipv4_tcpThroughput(0), ipv4_udpPacketNum(0), ipv4_udpThroughput(0),
        ipv6_tcpPacketNum(0), ipv6_tcpThroughput(0), ipv6_udpPacketNum(0), ipv6_udpThroughput(0),
        heavyFlowPacketNum(0), heavyFlowThroughput(0), littleFlowPacketNum(0), littleFlowThroughput(0),
        layer5Count{}, usrTrafficDetail{}, actAdd{}, actRmv{}, activeFlowDetail{}{
    }

    fiveTupleInfo::fiveTupleInfo(): packetTotalNum(0), packetTotalSize(0),
        bgts_sec(0), bgts_usec(0), edts_sec(0), edts_usec(0), isHeavyHitter(false), burstStatics{}{} // burst edit

    burstInfo::burstInfo(): timeSpan_sec{}, timeSpan_usec{}, packetNum(0), byteCount(0){} // burst edit
    
    uint64_t fiveTupleInfo::getDurationSec() {
        uint64_t bgts_sec_u64 = (uint64_t)bgts_sec;
        uint64_t edts_sec_u64 = (uint64_t)edts_sec;
        return edts_sec_u64-bgts_sec_u64;
    }

    uint64_t fiveTupleInfo::getDurationMilliSec() {
        uint64_t bgts_msec_u64 = (uint64_t)bgts_sec*1000 + (uint64_t)bgts_usec/1000;
        uint64_t edts_msec_u64 = (uint64_t)edts_sec*1000 + (uint64_t)edts_usec/1000;
        return edts_msec_u64-bgts_msec_u64;
    }

    uint64_t fiveTupleInfo::getDurationMicroSec() {
        uint64_t bgts_msec_u64 = (uint64_t)bgts_sec*1000000 + (uint64_t)bgts_usec;
        uint64_t edts_msec_u64 = (uint64_t)edts_sec*1000000 + (uint64_t)edts_usec;
        return edts_msec_u64-bgts_msec_u64;
    }

    FileManager::FileManager(): throughputFile(nullptr), activeFlowFile(nullptr),
    protocolCountFile(nullptr), packetSizeFile(nullptr), heavyLittleFlowSizeFile(nullptr),
    fiveTupleFile(nullptr), heavyFlowPktSizeFile(nullptr), littleFlowPktSizeFile(nullptr),
    heavyFlowDurationFile(nullptr), littleFlowDurationFile(nullptr), layer5ProtoCountFile(nullptr), burstDurationFile(nullptr),
    burstPacketNumFile(nullptr), burstByteCountFile(nullptr), activeFlowAddFile(nullptr), activeFlowRmvFile(nullptr), activeFlowUserFile(nullptr), activeFlowDetailFile(nullptr)

    {}
}