/*
* author: Wenrui Liu
* last edit time: 2023-5-7
*/

#include<recorder.h>
#include<fstream>
#include<iostream>
#include<mutex>
#include<bitset>
#include<string>
#include<algorithm>
#include <sstream>
using namespace std;

namespace MiniTAP{
    ElasticSketch::ElasticSketch(uint32_t size, uint32_t threshold) : heavyPartSize(size), heavyPart(size), threshold(threshold){
        for(int i = 0; i < size; i++)
            heavyPart[i] = heavyBucket(0, 0, 0);
        bobHash.initialize(50); // 随便写的一个数
    }

    void ElasticSketch::insert(fiveTuple pkt, uint32_t pktSize){
        auto int322String = [](uint32_t num){return std::bitset<32>(num).to_string();};
        auto int162String = [](uint16_t num){return std::bitset<16>(num).to_string();};
        std::string fiveTupleString = int322String(pkt.srcIP) + int322String(pkt.dstIP) + int162String(pkt.srcPort) + int162String(pkt.dstPort) + int162String(pkt.protocol);
        uint32_t fiveTupleHash = bobHash.run(fiveTupleString.c_str(), fiveTupleString.size()) % heavyPartSize;
        if(!heavyPart[fiveTupleHash].flow.has_value()){
            heavyPart[fiveTupleHash].flow = pkt;
            heavyPart[fiveTupleHash].flag = false;
            heavyPart[fiveTupleHash].posVote = pktSize;
            heavyPart[fiveTupleHash].negVote = 0;
        }
        else{
            if(heavyPart[fiveTupleHash].flow == pkt){
                heavyPart[fiveTupleHash].posVote += pktSize;
            }
            else{
                heavyPart[fiveTupleHash].negVote += pktSize;
                if(heavyPart[fiveTupleHash].negVote >= heavyPart[fiveTupleHash].posVote * threshold){
                    evict(heavyPart[fiveTupleHash].flow.value(), heavyPart[fiveTupleHash].posVote);
                    heavyPart[fiveTupleHash].flow = pkt;
                    heavyPart[fiveTupleHash].posVote = pktSize;
                    heavyPart[fiveTupleHash].negVote = 0;
                    heavyPart[fiveTupleHash].flag = true;
                }
            }
        }
    }

    void ElasticSketch::evict(fiveTuple flow, uint32_t flowSize){

    }

    std::vector<fiveTuple> ElasticSketch::findTopK(uint32_t k){
        std::vector<heavyBucket> tmp(heavyPart);
        std::sort(tmp.begin(), tmp.end(), [](const heavyBucket& a, const heavyBucket& b){return a.posVote > b.posVote;});
        std::vector<fiveTuple> res;
        for(int i = 0; i < k && i < tmp.size(); i++){
            if(tmp[i].flow.has_value())
                res.push_back(tmp[i].flow.value());
        }
        return res;
    }

    Recorder::Recorder(): fiveTupleRecord{}, trafficRecord{}, fileManager(), configInfo{},
    interval_pktSize(0), interval_activeFlow1{}, interval_activeFlow2{}, actAdd{}, actRmv{}, last_interval_ts_sec(0), last_interval_ts_usec(0),
    last_flush_interval_ts_sec(0), last_flush_interval_ts_usec(0), clickhouseStore(0),
    last_pkt_ts_sec(0), last_pkt_ts_usec(0), last_flow_ts_sec(0), last_flow_ts_usec(0),
    needTraverseAfter(false), current_ts_sec(0),current_ts_usec(0), sketch(1e3, 8), topkFlowDetail{}, topkFlow{}, topkFlowHash{}
    {
        fiveTupleRecord.reserve(99991);
        interval_activeFlow1.reserve(997);
        interval_activeFlow2.reserve(997);
    }

    void Recorder::burstCount(Packet *pkt, fiveTuple_t ft){
        if(fiveTupleRecord.find(ft) == fiveTupleRecord.end())
            return ;
        auto ftitmp = fiveTupleRecord[ft];
        // cout << "11111111" << endl;
        if(ftitmp.edts_sec == 0 && ftitmp.edts_usec == 0){ return ;}
        // cout << "22222222" << endl;
        // cout << "2313123123" << endl;
        // uint64_t ts = TS_diff_microSec(pkt->rawData.pktHeader.ts_sec, pkt->rawData.pktHeader.ts_usec, ftitmp.edts_sec, ftitmp.edts_usec);
        // cout << ts << endl;
        if(ftitmp.burstStatics.size() == 0){
            // cout << ts << endl;
            uint64_t ts = TS_diff_microSec(pkt->rawData.pktHeader.ts_sec, pkt->rawData.pktHeader.ts_usec, ftitmp.edts_sec, ftitmp.edts_usec);
            // cout << ts << endl;
            if(ts <= burstGap * 1000){
                burstInfo newBurst;
                newBurst.byteCount = pkt->pkt_len + ftitmp.lst_pkt_size;
                newBurst.packetNum = 2;
                newBurst.timeSpan_sec = {ftitmp.edts_sec, pkt->rawData.pktHeader.ts_sec};
                newBurst.timeSpan_usec = {ftitmp.edts_usec, pkt->rawData.pktHeader.ts_usec};
                fiveTupleRecord[ft].burstStatics.push_back(newBurst);
            }
        }
        else{
            burstInfo backBurst = ftitmp.burstStatics.back();
            // cout << ts << endl;
            uint64_t ts = TS_diff_microSec(pkt->rawData.pktHeader.ts_sec, pkt->rawData.pktHeader.ts_usec, backBurst.timeSpan_sec.second, backBurst.timeSpan_usec.second);
            if(ts <= burstGap * 1000){
                // cout << ts << endl;
                fiveTupleRecord[ft].burstStatics.back().byteCount += pkt->pkt_len;
                fiveTupleRecord[ft].burstStatics.back().packetNum++;
                fiveTupleRecord[ft].burstStatics.back().timeSpan_sec.second = pkt->rawData.pktHeader.ts_sec;
                fiveTupleRecord[ft].burstStatics.back().timeSpan_usec.second = pkt->rawData.pktHeader.ts_usec;
            }
            else{
                uint64_t ts = TS_diff_microSec(pkt->rawData.pktHeader.ts_sec, pkt->rawData.pktHeader.ts_usec, ftitmp.edts_sec, ftitmp.edts_usec);
                // cout << ts << endl;
                if(ts <= burstGap * 1000){
                    burstInfo newBurst;
                    newBurst.byteCount = pkt->pkt_len + ftitmp.lst_pkt_size;
                    newBurst.packetNum = 2;
                    newBurst.timeSpan_sec = {ftitmp.edts_sec, pkt->rawData.pktHeader.ts_sec};
                    newBurst.timeSpan_usec = {ftitmp.edts_usec, pkt->rawData.pktHeader.ts_usec};
                    fiveTupleRecord[ft].burstStatics.push_back(newBurst);
                }
            }
        }
    }

    int Recorder::init(const std::string & configFileName) {
        std::ifstream config_fp("../config/recordConfig");
        std::string tmp_name;
        int tmp_val;

        configInfo["throughput"] = false;
        configInfo["activeFlowNum"] = false;
        configInfo["protocolCount"] = false;
        configInfo["packetSize"] = false;
        configInfo["heavyLittleFlowSize"] = false;
        configInfo["fiveTupleFlowInfo"] = false;
        configInfo["heavyLittleFlowInfo"] = false;
        configInfo["pktInterArrivalTime"] = false;
        configInfo["flowInterArrivalTime"] = false;
        configInfo["IPv4DstByteCount"] = false;
        // burst edit
        configInfo["burstDuration"] = false;
        configInfo["burstPacketNum"] = false;
        configInfo["burstByteCount"] = false;
        // sketch topk
        configInfo["sketchTopK"] = false;

        trafficRecord.layer5Count["DNS"] = std::make_pair(0, 0);
        trafficRecord.layer5Count["FTP"] = std::make_pair(0, 0);
        trafficRecord.layer5Count["SMTP"] = std::make_pair(0, 0);
        trafficRecord.layer5Count["POP3"] = std::make_pair(0, 0);
        trafficRecord.layer5Count["IMAP"] = std::make_pair(0, 0);
        trafficRecord.layer5Count["DHCP"] = std::make_pair(0, 0);
        trafficRecord.layer5Count["HTTP"] = std::make_pair(0, 0);
        trafficRecord.layer5Count["HTTPS"] = std::make_pair(0, 0);
        trafficRecord.layer5Count["Unknown"] = std::make_pair(0, 0);
        // cout << "clickhouseStore: " << clickhouseStore << endl;
        while(config_fp>>tmp_name>>tmp_val) {
            // cout << tmp_name << tmp_val << endl;
            if(tmp_name == "activeFlowNum") {
                // cout << "activeFlow.2///////////////////////////////" << endl;
                configInfo["activeFlowNum"] = (bool)tmp_val;
                if(configInfo["activeFlowNum"]) {
                    fileManager.activeFlowFile = fopen("../output/activeFlow.out","w");
                    fileManager.activeFlowAddFile = fopen("../output/activeFlowAdd.out", "w");
                    fileManager.activeFlowRmvFile = fopen("../output/activeFlowRmv.out", "w");
                    fileManager.activeFlowUserFile = fopen("../output/activeFlowUser.out", "w");
                    fileManager.activeFlowAddUserFile = fopen("../output/activeFlowAddUser.out", "w");
                    fileManager.activeFlowRmvUserFile = fopen("../output/activeFlowRmvUser.out", "w");
                    fileManager.activeFlowDetailFile = fopen("../output/activeFlowDetail.out", "w");
                }
            }
            else if(tmp_name == "throughput") {
                configInfo["throughput"] = (bool)tmp_val;
                if(configInfo["throughput"]) {
                    cout << tmp_name << tmp_val << endl;
                    fileManager.throughputFile = fopen("../output/throughput.out","w"); 
                }
            }
            else if(tmp_name == "protocolCount") {
                configInfo["protocolCount"] = (bool)tmp_val;
                
                // client.Execute("create talbe if not exists protoCount(protocol UInt64, pktNum UInt64, byteCount UInt64) engine=Memory;");
                if(configInfo["protocolCount"]) {
                    fileManager.protocolCountFile = fopen("../output/protocolCount.out","w");
                    fileManager.layer5ProtoCountFile = fopen("../output/layer5ProtoCount.out","w");
                }
            }
            else if(tmp_name == "packetSize") {
                configInfo["packetSize"] = (bool)tmp_val;
                if(configInfo["packetSize"]) {
                    fileManager.packetSizeFile = fopen("../output/packetSize.out","w");
                }
            }
            else if(tmp_name == "heavyLittleFlowSize") {
                configInfo["heavyLittleFlowSize"] = (bool)tmp_val;
                if(configInfo["heavyLittleFlowSize"]) {
                    fileManager.heavyLittleFlowSizeFile = fopen("../output/heavyLittleFlowSize.out","w");
                    needTraverseAfter = true;
                }
            }
            else if(tmp_name == "fiveTupleFlowInfo") {
                configInfo["fiveTupleFlowInfo"] = (bool)tmp_val;
                if(configInfo["fiveTupleFlowInfo"]) {
                    fileManager.fiveTupleFile = fopen("../output/fiveTupleInfo.out","w");
                }
            }
            else if(tmp_name == "heavyLittleFlowInfo") {
                configInfo["heavyLittleFlowInfo"] = (bool)tmp_val;
                if(configInfo["heavyLittleFlowInfo"]) {
                    fileManager.heavyFlowDurationFile = fopen("../output/heavyFlowDuration.out","w");
                    fileManager.littleFlowDurationFile = fopen("../output/littleFlowDuration.out","w");
                    fileManager.heavyFlowPktSizeFile = fopen("../output/heavyFlowPktSize.out","w");
                    fileManager.littleFlowPktSizeFile = fopen("../output/littleFlowPktSize.out","w");
                    needTraverseAfter = true;
                }
            }
            else if(tmp_name == "pktInterArrivalTime") {
                configInfo["pktInterArrivalTime"] = (bool)tmp_val;
                if(configInfo["pktInterArrivalTime"]) {
                    fileManager.pktInterTimeFile = fopen("../output/pktInterArrivalTime.out","w");
                }
            }
            else if(tmp_name == "flowInterArrivalTime") {
                configInfo["flowInterArrivalTime"] = (bool)tmp_val;
                if(configInfo["flowInterArrivalTime"]) {
                    fileManager.flowInterTimeFile = fopen("../output/flowInterArrivalTime.out","w");
                }
            }
            else if(tmp_name == "IPv4DstByteCount") {
                configInfo["IPv4DstByteCount"] = (bool)tmp_val;
                if(configInfo["IPv4DstByteCount"]) {
                    fileManager.IPv4DstByteFile = fopen("../output/IPv4DstByteCount.out","w");
                }
            }
            else if(tmp_name == "TCPCongestionInfo") {
                configInfo["TCPCongestionInfo"] = (bool)tmp_val;
                if(configInfo["TCPCongestionInfo"]) {
                    fileManager.TCPCongestionInfoFile = fopen("../output/TCPCongestionInfo.out","w");
                }
                FILE * TCPStreams = fopen("../config/TCPStreamRecorded", "r");
                
                while (1) {
                    uint32_t srcIP, dstIP;
                    uint16_t srcPort, dstPort;
                    uint32_t a, b, c, d;
                    int ret = fscanf(TCPStreams, "%u.%u.%u.%u:%d", &a, &b, &c, &d, &srcPort);
                    if (ret < 0) break;
                    srcIP = (a<<24) | (b<<16) | (c<<8) | d;
                    fscanf(TCPStreams, "%u.%u.%u.%u:%d", &a, &b, &c, &d, &dstPort);
                    dstIP = (a<<24) | (b<<16) | (c<<8) | d;

                    fiveTuple_t ft = TCPStreamFiveTupleConvert(fiveTuple_t(srcIP, dstIP,
                        srcPort, dstPort, ProtoType::IP));
                    TCPStreamRecorded.insert(ft);
                }

            }
            else if(tmp_name == "burstDuration"){
                configInfo["burstDuration"] = (bool)tmp_val;
                if(configInfo["burstDuration"] && !clickhouseStore){
                    fileManager.burstDurationFile = fopen("../output/burstDuration.out", "w");
                }   
            }
            else if(tmp_name == "TCPCongestionInfo") {
                configInfo["TCPCongestionInfo"] = (bool)tmp_val;
                if(configInfo["TCPCongestionInfo"]) {
                    fileManager.TCPCongestionInfoFile = fopen("../output/TCPCongestionInfo.out","w");
                }
            }
            else if(tmp_name == "burstPacketNum"){
                configInfo["burstPacketNum"] = (bool)tmp_val;
                if(configInfo["burstPacketNum"] && !clickhouseStore){
                    fileManager.burstPacketNumFile = fopen("../output/burstPacketNum.out", "w");
                }
            }
            else if(tmp_name == "burstByteCount"){
                configInfo["burstByteCount"] = (bool)tmp_val;
                if(configInfo["burstByteCount"] && !clickhouseStore){
                    fileManager.burstByteCountFile = fopen("../output/burstByteCount.out", "w");
                }
                FILE * TCPStreams = fopen("../config/TCPStreamRecorded", "r");
                
                while (1) {
                    uint32_t srcIP, dstIP;
                    uint16_t srcPort, dstPort;
                    uint32_t a, b, c, d;
                    int ret = fscanf(TCPStreams, "%u.%u.%u.%u:%d", &a, &b, &c, &d, &srcPort);
                    // std::cerr << a << ' ' << b << " " << c << " " << d << " " << srcPort << '\n';
                    if (ret < 0) break;
                    srcIP = (a<<24) | (b<<16) | (c<<8) | d;
                    fscanf(TCPStreams, "%u.%u.%u.%u:%d", &a, &b, &c, &d, &dstPort);
                    dstIP = (a<<24) | (b<<16) | (c<<8) | d;
                    // std::cerr << a << ' ' << b << " " << c << " " << d << " " << dstPort << '\n';

                    fiveTuple_t ft = TCPStreamFiveTupleConvert(fiveTuple_t(srcIP, dstIP,
                        srcPort, dstPort, ProtoType::IP));
                    TCPStreamRecorded.insert(ft);
                }
                // for (auto &ft: TCPStreamRecorded)
                //     std::cerr << ft.srcIP << ' ' << ft.srcPort << ' ' << ft.dstIP << " " << ft.dstPort << ' ' << ft.protocol << '\n';

            }
            else if(tmp_name == "sketchTopK"){
                configInfo["sketchTopK"] = (bool)tmp_val;
                if(configInfo["sketchTopK"]){
                    
                    std::ifstream sketch_fp("../config/sketchConfig");
                    std::string str;
                    uint32_t partSize, threshold, topk;
                    sketch_fp >> str >> partSize >> str >> threshold >> str >> topk;
                    sketch = ElasticSketch(partSize, threshold);
                    topK = topk;
                    fileManager.topkFlowFile = fopen("../output/topkFlow.out", "w");
                }
            }
            else if(tmp_name == "sketchTopKDetail"){
                configInfo["sketchTopKDetail"] = (bool)tmp_val;
                if(configInfo["sketchTopKDetail"]){
                    std::ifstream file("../output/topkFlow.out");
                    if (!file.is_open()) {
                        printf("Error opening file: ../output/topkFlow.out");
                        return 0;
                    }

                    std::string line;
                    // 忽略第一行
                    std::getline(file, line);
                    
                    // 遍历文件的每一行
                    int id = 1;
                    while (std::getline(file, line)) {
                        std::istringstream iss(line);

                        fiveTuple_t entry;
                        iss >> entry.srcIP >> entry.srcPort >> entry.dstIP >> entry.dstPort >> entry.protocol;
                        topkFlow.insert(entry);
                        topkFlowDetail[entry] = std::vector<packetDetail>();
                        topkFlowHash[entry] = id++;
                    }

                    file.close();

                    fileManager.topkFlowDetailFile = fopen("../output/topkFlowDetail.out", "w");
                }
            }
        }
    }
    
    void Recorder::CreateNewRecord(fiveTuple_t tmp_ft,Packet* pkt){
        fiveTupleRecord[tmp_ft] = fiveTupleInfo();
        fiveTupleRecord[tmp_ft].bgts_sec = pkt->rawData.pktHeader.ts_sec;
        fiveTupleRecord[tmp_ft].bgts_usec = pkt->rawData.pktHeader.ts_usec;
        if(configInfo["flowInterArrivalTime"]) {
            if(last_flow_ts_sec == 0 && last_flow_ts_usec == 0) {
                last_flow_ts_sec = pkt->rawData.pktHeader.ts_sec;
                last_flow_ts_usec = pkt->rawData.pktHeader.ts_usec;
            }
            else {
                uint64_t tmp_ts = TS_diff_microSec(pkt->rawData.pktHeader.ts_sec, pkt->rawData.pktHeader.ts_usec, last_flow_ts_sec, last_flow_ts_usec);
                if(trafficRecord.flowInterArrivalTime.find(tmp_ts) != trafficRecord.flowInterArrivalTime.end()) {
                    trafficRecord.flowInterArrivalTime[tmp_ts] += 1;
                }
                else {
                    trafficRecord.flowInterArrivalTime[tmp_ts] = 1;
                }
                last_flow_ts_sec = pkt->rawData.pktHeader.ts_sec;
                last_flow_ts_usec = pkt->rawData.pktHeader.ts_usec;
            }
        }
        return;
    }
    void Recorder::InsertToRecord(fiveTuple_t tmp_ft,Packet* pkt){
        fiveTupleRecord[tmp_ft].packetTotalNum += 1;
        fiveTupleRecord[tmp_ft].packetTotalSize += pkt->pkt_len;
        fiveTupleRecord[tmp_ft].edts_sec = pkt->rawData.pktHeader.ts_sec;
        fiveTupleRecord[tmp_ft].edts_usec = pkt->rawData.pktHeader.ts_usec;
        fiveTupleRecord[tmp_ft].lst_pkt_size = pkt->pkt_len;
    }

    void Recorder::GetPacketToRecord(fiveTuple_t tmp_ft,Packet* pkt) {
        if(pkt->layer3Protocol == ProtoType::IPv4 && (pkt->layer4Protocol == ProtoType::TCP || pkt->layer4Protocol == ProtoType::TCPData || pkt->layer4Protocol == ProtoType::UDP || pkt->layer4Protocol == ProtoType::UDPData)) {
            if(fiveTupleRecord.find(tmp_ft) == fiveTupleRecord.end()) {
                CreateNewRecord(tmp_ft,pkt);
            }
            
            // cout << pkt->layer4Protocol << endl;
            if (configInfo["TCPCongestionInfo"] == true && pkt->layer4Protocol == ProtoType::TCP) {
                fiveTuple_t TCPStreamFT = TCPStreamFiveTupleConvert(tmp_ft);
                if (TCPStreamRecorded.empty() || TCPStreamRecorded.find(TCPStreamFT) != TCPStreamRecorded.end()) {
                    // insert tcp congestion info
                    // std::cerr << pkt->ptr_ipv4->srcIP << ' ' << pkt->ptr_ipv4->dstIP << '\n';
                    // std::cerr << pkt->ptr_tcp->srcPort << ' ' << pkt->ptr_tcp->dstPort << '\n';
                    // std::cerr << TCPStreamFT.srcIP << ' ' << TCPStreamFT.srcPort << '\n';
                    // std::cerr << TCPStreamFT.dstIP << ' ' << TCPStreamFT.dstPort << '\n';
                    // std::cerr << TCPStreamFT.protocol << '\n';
                    TCP_h *tcp_hdr = pkt->ptr_tcp;
                    TCPCongestionRecord[TCPStreamFT].push_back(TCPPktInfo(pkt->rawData.pktHeader.ts_sec,
                        pkt->rawData.pktHeader.ts_usec, tcp_hdr->seqNum, tcp_hdr->ackNum,
                        tcp_hdr->flags, tcp_hdr->windowSize, pkt->pkt_cnt));
                }
            }
        }
        else if(pkt->layer3Protocol == ProtoType::IPv4) {
            // cout << "!!!!!!!!" << endl;
            if(configInfo["IPv4DstByteCount"]) {
                if(trafficRecord.IPv4DstByteCount.find(pkt->ptr_ipv4->dstIP) != trafficRecord.IPv4DstByteCount.end()) {
                    trafficRecord.IPv4DstByteCount[pkt->ptr_ipv4->dstIP] += pkt->pkt_len;
                }
                else {
                    trafficRecord.IPv4DstByteCount[pkt->ptr_ipv4->dstIP] = pkt->pkt_len;
                }
            }
        }
        if(configInfo["burstDuration"] || configInfo["burstPacketNum"] || configInfo["burstByteCount"] || configInfo["burst"])
            burstCount(pkt, tmp_ft);
        if(configInfo["sketchTopK"] || configInfo["sketchTopKDetail"])
            sketch.insert(tmp_ft, pkt->pkt_len);
        if(pkt->layer3Protocol == ProtoType::IPv4 && (pkt->layer4Protocol == ProtoType::TCP || pkt->layer4Protocol == ProtoType::TCPData || pkt->layer4Protocol == ProtoType::UDP || pkt->layer4Protocol == ProtoType::UDPData))
            InsertToRecord(tmp_ft,pkt);  
        pkt->pkt_cnt++;
        return;
    }

    void Recorder::UpdateProtocolCount(Packet* pkt) {
        if(configInfo["protocolCount"]) {
            trafficRecord.totalPacketNum += 1;
            trafficRecord.totalThroughput += pkt->pkt_len;
            if(pkt->layer3Protocol == ProtoType::IPv4) {
                trafficRecord.ipv4PacketNum += 1;
                trafficRecord.ipv4Throughput += pkt->pkt_len;
            }
            else if(pkt->layer3Protocol == ProtoType::IPv6) {
                trafficRecord.ipv6PacketNum += 1;
                trafficRecord.ipv6Throughput += pkt->pkt_len;
            }

            if(pkt->layer3Protocol == ProtoType::IPv4&&(pkt->layer4Protocol == ProtoType::TCP || pkt->layer4Protocol == ProtoType::TCPData)) {
                trafficRecord.ipv4_tcpPacketNum += 1;
                trafficRecord.ipv4_tcpThroughput += pkt->pkt_len;
            }
            else if(pkt->layer3Protocol == ProtoType::IPv4&&(pkt->layer4Protocol == ProtoType::UDP || pkt->layer4Protocol == ProtoType::UDPData)) {
                trafficRecord.ipv4_udpPacketNum += 1;
                trafficRecord.ipv4_udpThroughput += pkt->pkt_len;
            }
            else if(pkt->layer3Protocol == ProtoType::IPv6&&(pkt->layer4Protocol == ProtoType::TCP || pkt->layer4Protocol == ProtoType::TCPData)) {
                trafficRecord.ipv6_tcpPacketNum += 1;
                trafficRecord.ipv6_tcpThroughput += pkt->pkt_len;
            }
            else if(pkt->layer3Protocol == ProtoType::IPv6&&(pkt->layer4Protocol == ProtoType::UDP || pkt->layer4Protocol == ProtoType::UDPData)) {
                trafficRecord.ipv6_udpPacketNum += 1;
                trafficRecord.ipv6_udpThroughput += pkt->pkt_len;
            }

            if(pkt->layer5Protocol == ProtoType::DNS) {
                trafficRecord.layer5Count["DNS"].first += 1;
                trafficRecord.layer5Count["DNS"].second += pkt->pkt_len;
            }
            else if(pkt->layer5Protocol == ProtoType::FTP) {
                trafficRecord.layer5Count["FTP"].first += 1;
                trafficRecord.layer5Count["FTP"].second += pkt->pkt_len;
            }
            else if(pkt->layer5Protocol == ProtoType::SMTP) {
                trafficRecord.layer5Count["SMTP"].first += 1;
                trafficRecord.layer5Count["SMTP"].second += pkt->pkt_len;
            }
            else if(pkt->layer5Protocol == ProtoType::POP3) {
                trafficRecord.layer5Count["POP3"].first += 1;
                trafficRecord.layer5Count["POP3"].second += pkt->pkt_len;
            }
            else if(pkt->layer5Protocol == ProtoType::IMAP) {
                trafficRecord.layer5Count["IMAP"].first += 1;
                trafficRecord.layer5Count["IMAP"].second += pkt->pkt_len;
            }
            else if(pkt->layer5Protocol == ProtoType::DHCP) {
                trafficRecord.layer5Count["DHCP"].first += 1;
                trafficRecord.layer5Count["DHCP"].second += pkt->pkt_len;
            }
            else if(pkt->layer5Protocol == ProtoType::HTTP) {
                trafficRecord.layer5Count["HTTP"].first += 1;
                trafficRecord.layer5Count["HTTP"].second += pkt->pkt_len;
            }
            else if(pkt->layer5Protocol == ProtoType::HTTPS) {
                trafficRecord.layer5Count["HTTPS"].first += 1;
                trafficRecord.layer5Count["HTTPS"].second += pkt->pkt_len;
            }
            else {
                trafficRecord.layer5Count["Unknown"].first += 1;
                trafficRecord.layer5Count["Unknown"].second += pkt->pkt_len;
            }
        }
        return;
    }
    void Recorder::UpdateArrivalTime(fiveTuple_t tmp_ft,Packet* pkt){
        if(configInfo["pktInterArrivalTime"]) {
            if(last_pkt_ts_sec == 0 && last_pkt_ts_usec == 0) {
                last_pkt_ts_sec = pkt->rawData.pktHeader.ts_sec;
                last_pkt_ts_usec = pkt->rawData.pktHeader.ts_usec;
            }
            else {
                uint64_t tmp_ts = TS_diff_microSec(pkt->rawData.pktHeader.ts_sec, pkt->rawData.pktHeader.ts_usec, last_pkt_ts_sec, last_pkt_ts_usec);
                if(trafficRecord.pktInterArrivalTime.find(tmp_ts) != trafficRecord.pktInterArrivalTime.end()) {
                    trafficRecord.pktInterArrivalTime[tmp_ts] += 1;
                }
                else {
                    trafficRecord.pktInterArrivalTime[tmp_ts] = 1;
                }
                last_pkt_ts_sec = pkt->rawData.pktHeader.ts_sec;
                last_pkt_ts_usec = pkt->rawData.pktHeader.ts_usec;
            }
        }
        return;
    }

    void Recorder::UpdateThroughOutput(fiveTuple_t tmp_ft,Packet* pkt){
        // if(tmp_ft.protocol == 0){
        //     printf("unknown protocol\n");
        //     return ;
        // }

        auto calc_usec = [](uint32_t ts_sec, uint32_t ts_usec) {
            return (uint64_t)ts_sec * 1000000 + ts_usec;
        };

        if(configInfo["throughput"]){
            uint64_t current_interval = calc_usec(current_ts_sec, current_ts_usec) / recordInterval;
            uint64_t last_interval = calc_usec(last_interval_ts_sec, last_interval_ts_usec) / recordInterval;
            
            if(current_interval != last_interval){
                if(last_interval_ts_sec != 0 && last_interval_ts_usec != 0){
                    for(uint64_t i = last_interval; i < current_interval; i++){
                        trafficRecord.throughput.push_back(0);
                        int tt = trafficRecord.throughput.size();
                        // printf("%d %d %d\n", current_interval, trafficRecord.throughput.size(), trafficRecord.throughput[tt - 2]);
                    }
                }
            }
            if(trafficRecord.throughput.size() == 0)
                trafficRecord.throughput.push_back(0);
            trafficRecord.throughput.back() += pkt->pkt_len;
            // last_interval_ts_sec = current_ts_sec;
            // last_interval_ts_usec = current_ts_usec;
        }
        return;
    }

    void Recorder::UpdateActiveFlowNum(fiveTuple_t tmp_ft,Packet* pkt){
        
        // if (tmp_ft.protocol == 0) return ;

        auto calc_usec = [](uint32_t ts_sec, uint32_t ts_usec) {
            return (uint64_t)ts_sec * 1000000 + ts_usec;
        };

        if(configInfo["activeFlowNum"]) {
            uint64_t current_interval = calc_usec(current_ts_sec, current_ts_usec) / recordInterval;
            uint64_t last_interval = calc_usec(last_interval_ts_sec, last_interval_ts_usec) / recordInterval;
            
            if(current_interval != last_interval){ // 不在同一秒内
                // printf("currentInterval: %lld\n", current_interval);
                if(last_interval_ts_sec != 0 && last_interval_ts_usec != 0) {
                    // 如果相邻两个包跨越了很多秒，也会在这些没有数据包的时间内补上0
                    for (uint64_t i = last_interval; i < current_interval; i++){
                        trafficRecord.activeFlow.push_back(0);
                    }
                }
                std::unordered_set<fiveTuple_t, FiveTupleHash>* interval_activeFlow = actFlowFlag % 2 ? &interval_activeFlow1 : &interval_activeFlow2;
                std::unordered_set<fiveTuple_t, FiveTupleHash>* pre_activeFlow = actFlowFlag % 2 ? &interval_activeFlow2 : &interval_activeFlow1;
                if(actFlowFlag >= 1){
                    for(auto ft : (*interval_activeFlow)){
                        if((*pre_activeFlow).find(ft) == (*pre_activeFlow).end()){
                            if(!trafficRecord.usrTrafficDetail[ft.srcIP].actAdd.size() || trafficRecord.usrTrafficDetail[ft.srcIP].actAdd.back().first != last_interval){
                                trafficRecord.usrTrafficDetail[ft.srcIP].actAdd.push_back((puu)make_pair(last_interval, 0));
                            }
                            trafficRecord.usrTrafficDetail[ft.srcIP].actAdd.back().second++;
                            if(!trafficRecord.actAdd.size() || trafficRecord.actAdd.back().first != last_interval)
                                trafficRecord.actAdd.push_back((puu)make_pair(last_interval, 0));
                            trafficRecord.actAdd.back().second++;
                            trafficRecord.activeFlowDetail.push_back(actDetail(ft, last_interval, 1));
                        }
                    }
                    for(auto ft : (*pre_activeFlow)){
                        if((*interval_activeFlow).find(ft) == (*interval_activeFlow).end()){ 
                            if(!trafficRecord.usrTrafficDetail[ft.srcIP].actRmv.size() || trafficRecord.usrTrafficDetail[ft.srcIP].actRmv.back().first != last_interval)
                                trafficRecord.usrTrafficDetail[ft.srcIP].actRmv.push_back((puu)make_pair(last_interval, 0));
                            trafficRecord.usrTrafficDetail[ft.srcIP].actRmv.back().second++;
                            if(!trafficRecord.actRmv.size() || trafficRecord.actRmv.back().first != last_interval)
                                trafficRecord.actRmv.push_back((puu)make_pair(last_interval, 0));
                            trafficRecord.actRmv.back().second++;
                            trafficRecord.activeFlowDetail.push_back(actDetail(ft, last_interval, 0));
                        }
                    }
                }
                (*pre_activeFlow).clear(); // 过去的set用来清空用来当作新的时间间隔的set
                actFlowFlag++;
            }
            std::unordered_set<fiveTuple_t, FiveTupleHash>* interval_activeFlow = actFlowFlag % 2 ? &interval_activeFlow1 : &interval_activeFlow2;
            if ((*interval_activeFlow).find(tmp_ft) == (*interval_activeFlow).end()) {
                if (trafficRecord.activeFlow.size() == 0){
                    trafficRecord.activeFlow.push_back(0);
                }
                if(trafficRecord.usrTrafficDetail.find(tmp_ft.srcIP) == trafficRecord.usrTrafficDetail.end()){
                    trafficRecord.usrTrafficDetail[tmp_ft.srcIP] = usr_traffic_detail(tmp_ft.srcIP);
                }
                if(trafficRecord.usrTrafficDetail[tmp_ft.srcIP].activeFlow.size() == 0 || trafficRecord.usrTrafficDetail[tmp_ft.srcIP].activeFlow.back().first != current_ts_sec)
                    trafficRecord.usrTrafficDetail[tmp_ft.srcIP].activeFlow.push_back((puu)make_pair(current_ts_sec, 0));
                trafficRecord.activeFlow.back()++;
                trafficRecord.usrTrafficDetail[tmp_ft.srcIP].activeFlow.back().second++;
                (*interval_activeFlow).insert(tmp_ft);
            }
        }
        return;
    }

    void Recorder::UpdatePacketSize(fiveTuple_t tmp_ft,Packet* pkt){
        if(configInfo["packetSize"]) {
            if(trafficRecord.packetSizeCount.find(pkt->pkt_len) != trafficRecord.packetSizeCount.end()) {
                trafficRecord.packetSizeCount[pkt->pkt_len] += 1;
            }
            else {
                trafficRecord.packetSizeCount[pkt->pkt_len] = 1;
            }
        }
        return;
    }

    void Recorder::UpdateFlushTime(){
        if(last_flush_interval_ts_sec == 0 && last_flush_interval_ts_usec == 0) {
            last_flush_interval_ts_sec = current_ts_sec;
            last_flush_interval_ts_usec = current_ts_usec;
        }
        else {
            if(TS_diff_microSec(current_ts_sec, current_ts_usec, last_flush_interval_ts_sec, last_flush_interval_ts_usec) >= recordFlushCacheInterval) {
                flushCache();
                last_flush_interval_ts_sec = current_ts_sec;
                last_flush_interval_ts_usec = current_ts_usec;
            }
        }
    }

    void Recorder::UpdateTrafficRecord(fiveTuple_t tmp_ft,Packet* pkt) {
        current_ts_sec = pkt->rawData.pktHeader.ts_sec;
        current_ts_usec = pkt->rawData.pktHeader.ts_usec;
        if(configInfo["pktInterArrivalTime"]) UpdateArrivalTime(tmp_ft,pkt);
        if(configInfo["throughput"]) UpdateThroughOutput(tmp_ft,pkt);
        if(configInfo["activeFlowNum"]) UpdateActiveFlowNum(tmp_ft,pkt);
        if(configInfo["throughput"] || configInfo["activeFlowNum"]){
            last_interval_ts_sec = current_ts_sec;
            last_interval_ts_usec = current_ts_usec;
        }
        if(configInfo["packetSize"]) UpdatePacketSize(tmp_ft,pkt);
        if(configInfo["protocolCount"]) UpdateProtocolCount(pkt);
        UpdateFlushTime();
        pkt->pkt_cnt++;
        return;
    }

    void Recorder::pass(Packet * pkt) {
        if(pkt == nullptr) {
            return;
        }

        if(!TS_biggerEqual_than(pkt->rawData.pktHeader.ts_sec, pkt->rawData.pktHeader.ts_usec, current_ts_sec, current_ts_usec)) {
            // printf("1\n");
            return;
        }

        fiveTuple_t tmp_ft{};
        // printf("%d\n", pkt->layer4Protocol);
        if(pkt->layer3Protocol == ProtoType::IPv4 && (pkt->layer4Protocol == ProtoType::TCP)) {
            // cout << "TCP" << endl;
            tmp_ft = fiveTuple_t(pkt->ptr_ipv4->srcIP, pkt->ptr_ipv4->dstIP, pkt->ptr_tcp->srcPort, pkt->ptr_tcp->dstPort, pkt->ptr_ipv4->protocol);
        }
        if(pkt->layer3Protocol == ProtoType::IPv4 && (pkt->layer4Protocol == ProtoType::TCPData)) {
            // cout << "TCP" << endl;
            tmp_ft = fiveTuple_t(pkt->ptr_ipv4->srcIP, pkt->ptr_ipv4->dstIP, 0, 0, 0);
        }
        else if(pkt->layer3Protocol == ProtoType::IPv4 && pkt->layer4Protocol == ProtoType::UDP) {
            // cout << "UDP" << endl;
            tmp_ft = fiveTuple_t(pkt->ptr_ipv4->srcIP, pkt->ptr_ipv4->dstIP, pkt->ptr_udp->srcPort, pkt->ptr_udp->dstPort, pkt->ptr_ipv4->protocol);
        }
        else if(pkt->layer3Protocol == ProtoType::IPv4 && pkt->layer4Protocol == ProtoType::UDPData) {
            // cout << "UDP" << endl;
            tmp_ft = fiveTuple_t(pkt->ptr_ipv4->srcIP, pkt->ptr_ipv4->dstIP, 0, 0, 0);
        }
        if(configInfo["sketchTopKDetail"]){
            // return ;
            if(topkFlow.count(tmp_ft)){
                // cout << pkt->layer3Protocol << ' ' << pkt->layer4Protocol << endl;
                if(pkt->layer3Protocol == ProtoType::IPv4 && (pkt->layer4Protocol == ProtoType::TCP || pkt->layer4Protocol == ProtoType::TCPData))
                    // cout << pkt->ptr_tcp->seqNum << ' ' << pkt->ptr_tcp->windowSize << endl;
                    topkFlowDetail[tmp_ft].push_back(packetDetail(pkt->rawData.pktHeader.ts_sec, pkt->rawData.pktHeader.ts_usec, pkt->pkt_len, pkt->ptr_tcp->windowSize, pkt->ptr_tcp->seqNum));
            }
            // 为了加速topkdetail统计的过程，这里直接返回，避免后续的一系列操作
            return ;
        }
        if((configInfo["sketchTopK"] || configInfo["sketchTopKDetail"]) && pkt->layer3Protocol == ProtoType::IPv4 && (pkt->layer4Protocol == ProtoType::TCP || pkt->layer4Protocol == ProtoType::TCPData)){
            sketch.insert(tmp_ft, pkt->pkt_len);
        }
        GetPacketToRecord(tmp_ft,pkt);
        UpdateTrafficRecord(tmp_ft,pkt);
    }

    void Recorder::finish() {
        printf("finishing...\n");
        // return ;
        flowTraverse();
        if(configInfo["sketchTopKDetail"]){
            for(auto flow : topkFlowDetail){
                fprintf(fileManager.topkFlowDetailFile, "%u\t%u\t%u\t%u\t%u\n", flow.first.srcIP, flow.first.srcPort, flow.first.dstIP, flow.first.dstPort, flow.first.protocol);
                for(auto pkt: flow.second)
                    fprintf(fileManager.topkFlowDetailFile, "%u\t%u\t%u\t%u\t%u\n", pkt.arrTime_sec, pkt.arrTime_usec, pkt.byteCount, pkt.window_size, pkt.seq_num);
                fprintf(fileManager.topkFlowDetailFile, "---------------------------------------------------------\n");
            }
        }    
        if(configInfo["sketchTopK"]){
            cout << "sketchTopk fprint" << endl;
            auto tmp = sketch.findTopK(topK);
            fprintf(fileManager.topkFlowFile, "srcIP\tsrcPort\tdstIP\tdstPort\tprotocol\n");
            // cout << "srcIP\tsrcPort\tdstIP\tdstPort\tprotocol\n";
            for(auto flow : tmp){
                // printf("%u\t%u\t%u\t%u\t%u\n", flow.srcIP, flow.srcPort, flow.dstIP, flow.dstPort, flow.protocol);
                fprintf(fileManager.topkFlowFile, "%u\t%u\t%u\t%u\t%u\n", flow.srcIP, flow.srcPort, flow.dstIP, flow.dstPort, flow.protocol);
            }
        }
        if(configInfo["burstDuration"]){
            cout << "burstDuration" << endl;
            std::unordered_map<uint32_t, uint32_t> burstDurationCount;
            cout << "ftsize: " << fiveTupleRecord.size() << endl;
            for(auto ft : fiveTupleRecord){
                // cout << "ft:" << ft.first.srcIP << endl;
                for(auto burst : ft.second.burstStatics){
                    // cout << burst.byteCount << endl;
                    uint32_t gap = (burst.timeSpan_sec.second - burst.timeSpan_sec.first) * 1e6 + burst.timeSpan_usec.second - burst.timeSpan_usec.first;
                    gap /= 1e3;
                    burstDurationCount[gap]++;
                }
            }
            for(auto gap : burstDurationCount){
                printf("%u %u\n", gap.first, gap.second);
                fprintf(fileManager.burstDurationFile, "%u %u\n", gap.first, gap.second);
            }
        }
        if(configInfo["burstPacketNum"]){
            std::unordered_map<uint32_t, uint32_t> burstPacketNum;
            for(auto ft : fiveTupleRecord){
                for(auto burst : ft.second.burstStatics){
                    burstPacketNum[burst.packetNum]++;
                }
            }
            for(auto num : burstPacketNum)
                fprintf(fileManager.burstPacketNumFile, "%u %u\n", num.first, num.second);
        }

        if(configInfo["burstByteCount"]){
            std::unordered_map<uint32_t, uint32_t> burstByteCount;
            for(auto ft : fiveTupleRecord){
                for(auto burst : ft.second.burstStatics){
                    burstByteCount[burst.byteCount]++;
                }
            }
            for(auto byte : burstByteCount)
                fprintf(fileManager.burstByteCountFile, "%u %u\n", byte.first, byte.second);
        }

        if(configInfo["throughput"]) {
            // cout << "throughput: " << trafficRecord.throughput.size() << endl;
            for(uint32_t & tp: trafficRecord.throughput) {
                fprintf(fileManager.throughputFile, "%u\n", tp);
            }
        }
        if(configInfo["activeFlowNum"]) {
            for(uint32_t & af: trafficRecord.activeFlow) fprintf(fileManager.activeFlowFile, "%u\n", af);
            for(puu & ad : trafficRecord.actAdd) fprintf(fileManager.activeFlowAddFile, "%u %u\n", ad.first, ad.second);
            for(puu & rm : trafficRecord.actRmv) fprintf(fileManager.activeFlowRmvFile, "%u %u\n", rm.first, rm.second);
            for(auto usrDetail : trafficRecord.usrTrafficDetail){
                fprintf(fileManager.activeFlowUserFile, "%u %u\n", usrDetail.first, usrDetail.second.activeFlow.size());
                for(auto af: usrDetail.second.activeFlow) fprintf(fileManager.activeFlowUserFile, "%u %u\n", af.first, af.second);
            }
            for(auto usrDetail : trafficRecord.usrTrafficDetail){
                fprintf(fileManager.activeFlowAddUserFile, "%u %u\n", usrDetail.first, usrDetail.second.activeFlow.size());
                for(auto af: usrDetail.second.actAdd) fprintf(fileManager.activeFlowAddUserFile, "%u %u\n", af.first, af.second);
            }
            for(auto usrDetail : trafficRecord.usrTrafficDetail){
                fprintf(fileManager.activeFlowRmvUserFile, "%u %u\n", usrDetail.first, usrDetail.second.activeFlow.size());
                for(auto af: usrDetail.second.actRmv) fprintf(fileManager.activeFlowAddUserFile, "%u %u\n", af.first, af.second);
            }
            fprintf(fileManager.activeFlowDetailFile, "srcIP\tdstIP\tsrcPort\tdstPort\tprotocol\ttimeStamp\tadd/rmv\n");
            for(auto detail : trafficRecord.activeFlowDetail){
                fprintf(fileManager.activeFlowDetailFile, "%u\t%u\t%u\t%u\t%u\t%llu\t%u\n", detail.ft.srcIP, detail.ft.dstIP, detail.ft.srcPort, detail.ft.dstPort, detail.ft.protocol, detail.timeStamp, detail.flag);
            }
        }
        if(configInfo["protocolCount"]) {
            fprintf(fileManager.protocolCountFile, "name\t\tpktNum\t\tsize\n");
            fprintf(fileManager.protocolCountFile, "total\t\t%llu\t\t%llu\n", trafficRecord.totalPacketNum, trafficRecord.totalThroughput);
            fprintf(fileManager.protocolCountFile, "ipv4\t\t%llu\t\t%llu\n", trafficRecord.ipv4PacketNum, trafficRecord.ipv4Throughput);
            fprintf(fileManager.protocolCountFile, "ipv6\t\t%llu\t\t%llu\n", trafficRecord.ipv6PacketNum, trafficRecord.ipv6Throughput);
            fprintf(fileManager.protocolCountFile, "ipv4-tcp\t%llu\t\t%llu\n", trafficRecord.ipv4_tcpPacketNum, trafficRecord.ipv4_tcpThroughput);
            fprintf(fileManager.protocolCountFile, "ipv4-udp\t%llu\t\t%llu\n", trafficRecord.ipv4_udpPacketNum, trafficRecord.ipv4_udpThroughput);
            fprintf(fileManager.protocolCountFile, "ipv6-tcp\t%llu\t\t%llu\n", trafficRecord.ipv6_tcpPacketNum, trafficRecord.ipv6_tcpThroughput);
            fprintf(fileManager.protocolCountFile, "ipv6-udp\t%llu\t\t%llu\n", trafficRecord.ipv6_udpPacketNum, trafficRecord.ipv6_udpThroughput);

            for(auto & p: trafficRecord.layer5Count) {
                fprintf(fileManager.layer5ProtoCountFile, "%s %llu %llu\n", p.first.c_str(), p.second.first, p.second.second);
            }
        }
        if(configInfo["packetSize"]) {
            for(auto & p: trafficRecord.packetSizeCount) {
                fprintf(fileManager.packetSizeFile, "%u %u\n", p.first, p.second);
            }
        }
        if(configInfo["heavyLittleFlowSize"]) {
            fprintf(fileManager.heavyLittleFlowSizeFile, "heavy %llu %llu\n", trafficRecord.heavyFlowPacketNum, trafficRecord.heavyFlowThroughput);
            fprintf(fileManager.heavyLittleFlowSizeFile, "little %llu %llu\n", trafficRecord.littleFlowPacketNum, trafficRecord.littleFlowThroughput);
        }
        if(configInfo["heavyLittleFlowInfo"]) {
            for(auto & p: trafficRecord.heavyFlowPacketNumCount) {
                fprintf(fileManager.heavyFlowPktSizeFile, "%u %u\n", p.first, p.second);
            }
            for(auto & p: trafficRecord.littleFlowPacketNumCount) {
                fprintf(fileManager.littleFlowPktSizeFile, "%u %u\n", p.first, p.second);
            }
            for(uint64_t d: trafficRecord.heavyFlowDuration) {
                fprintf(fileManager.heavyFlowDurationFile, "%llu\n", d);
            }
            for(uint64_t d: trafficRecord.littleFlowDuration) {
                fprintf(fileManager.littleFlowDurationFile, "%llu\n", d);
            }
        }
        if(configInfo["flowInterArrivalTime"]) {
            for(auto & p: trafficRecord.flowInterArrivalTime) {
                fprintf(fileManager.flowInterTimeFile, "%u %u\n", p.first, p.second);
            }
        }
        if(configInfo["pktInterArrivalTime"]) {
            for(auto & p: trafficRecord.pktInterArrivalTime) {
                fprintf(fileManager.pktInterTimeFile, "%u %u\n", p.first, p.second);
            }
        }
        if(configInfo["IPv4DstByteCount"]) {
            for(auto & p: trafficRecord.IPv4DstByteCount) {
                if(p.second >= IPv4DstRecordThreshold) {
                    fprintf(fileManager.IPv4DstByteFile, "%u %llu\n", p.first, p.second);
                }
            }
        }
        if (configInfo["TCPCongestionInfo"]) {
            for (auto &pr: TCPCongestionRecord) {
                fprintf(fileManager.TCPCongestionInfoFile, "%u %d %u %d\n", pr.first.srcIP, pr.first.srcPort, 
                    pr.first.dstIP, pr.first.dstPort);
                fprintf(fileManager.TCPCongestionInfoFile, "%d\n", pr.second.size());
            }
        }

    }

    Recorder::~Recorder() {
        cout << "deconstruct recorder" << endl;
        if(fileManager.throughputFile != nullptr) {
            fclose(fileManager.throughputFile);
        }
        if(fileManager.activeFlowFile != nullptr) {
            fclose(fileManager.activeFlowFile);
            fileManager.activeFlowFile = nullptr;
        }
        if(fileManager.protocolCountFile != nullptr) {
            fclose(fileManager.protocolCountFile);
            fileManager.protocolCountFile = nullptr;
            fclose(fileManager.layer5ProtoCountFile);
            fileManager.layer5ProtoCountFile = nullptr;
        }
        if(fileManager.packetSizeFile != nullptr) {
            fclose(fileManager.packetSizeFile);
            fileManager.packetSizeFile = nullptr;
        }
        if(fileManager.heavyLittleFlowSizeFile != nullptr) {
            fclose(fileManager.heavyLittleFlowSizeFile);
            fileManager.heavyLittleFlowSizeFile = nullptr;
        }
        if(fileManager.fiveTupleFile != nullptr) {
            fclose(fileManager.fiveTupleFile);
            fileManager.fiveTupleFile = nullptr;
        }
        if(fileManager.heavyFlowPktSizeFile != nullptr) {
            fclose(fileManager.heavyFlowPktSizeFile);
            fileManager.heavyFlowPktSizeFile = nullptr;
        }
        if(fileManager.littleFlowPktSizeFile != nullptr) {
            fclose(fileManager.littleFlowPktSizeFile);
            fileManager.littleFlowPktSizeFile = nullptr;
        }
        if(fileManager.heavyFlowDurationFile != nullptr) {
            fclose(fileManager.heavyFlowDurationFile);
            fileManager.heavyFlowDurationFile = nullptr;
        }
        // cout << 9;
        if(fileManager.littleFlowDurationFile != nullptr) {
            fclose(fileManager.littleFlowDurationFile);
            fileManager.littleFlowDurationFile = nullptr;
        }
        if(fileManager.pktInterTimeFile != nullptr) {
            fclose(fileManager.pktInterTimeFile);
            fileManager.pktInterTimeFile = nullptr;
        }
        if(fileManager.flowInterTimeFile != nullptr) {
            fclose(fileManager.flowInterTimeFile);
            fileManager.flowInterTimeFile = nullptr;
        }
        if(fileManager.IPv4DstByteFile != nullptr) {
            fclose(fileManager.IPv4DstByteFile);
            fileManager.IPv4DstByteFile = nullptr;
        }
        if(fileManager.burstByteCountFile != nullptr){
            fclose(fileManager.burstByteCountFile);
            fileManager.burstByteCountFile = nullptr;
        }
        if(fileManager.burstDurationFile != nullptr){
            fclose(fileManager.burstDurationFile);
            fileManager.burstDurationFile = nullptr;
        }
        if(fileManager.burstPacketNumFile != nullptr){
            fclose(fileManager.burstPacketNumFile);
            fileManager.burstPacketNumFile = nullptr;
        }
        if(fileManager.topkFlowDetailFile != nullptr){
            fclose(fileManager.topkFlowDetailFile);
            fileManager.topkFlowDetailFile = nullptr;
        }
        if(fileManager.topkFlowFile != nullptr){
            fclose(fileManager.topkFlowFile);
            fileManager.topkFlowFile = nullptr;
        }
    }

    void Recorder::flowTraverse() {
        if(needTraverseAfter) {
            cout << "ftsize" << fiveTupleRecord.size() << endl;
            
            for(auto & p: fiveTupleRecord) {
                // printf("%lld %lld %lld %lld %lld\n", p.first.srcIP, p.first.srcPort, p.first.dstIP, p.first.dstPort, p.first.protocol);
                int tag = 0;
                if(p.second.packetTotalSize >= heavyHitterThreshold) {
                    tag = 1;
                    p.second.isHeavyHitter = true;

                    if(configInfo["heavyLittleFlowSize"]) {
                        trafficRecord.heavyFlowPacketNum += p.second.packetTotalNum;
                        trafficRecord.heavyFlowThroughput += p.second.packetTotalSize;
                    }
                    if(configInfo["heavyLittleFlowInfo"]) {
                        if(trafficRecord.heavyFlowPacketNumCount.find(p.second.packetTotalNum) != trafficRecord.heavyFlowPacketNumCount.end()) {
                            trafficRecord.heavyFlowPacketNumCount[p.second.packetTotalNum] += 1;
                        }
                        else {
                            trafficRecord.heavyFlowPacketNumCount[p.second.packetTotalNum] = 1;
                        }
                        // printf("%lld %lld %lld %lld %lld ", p.first.srcIP, p.first.srcPort, p.first.dstIP, p.first.dstPort, p.first.protocol);
                        trafficRecord.heavyFlowDuration.push_back(p.second.getDurationMicroSec());
                    }
                }
                else {
                    tag = 0;
                    if(configInfo["heavyLittleFlowSize"]) {
                        trafficRecord.littleFlowPacketNum += p.second.packetTotalNum;
                        trafficRecord.littleFlowThroughput += p.second.packetTotalSize;
                    }
                    if(configInfo["heavyLittleFlowInfo"]) {
                        if(trafficRecord.littleFlowPacketNumCount.find(p.second.packetTotalNum) != trafficRecord.littleFlowPacketNumCount.end()) {
                            trafficRecord.littleFlowPacketNumCount[p.second.packetTotalNum] += 1;
                        }
                        else {
                            trafficRecord.littleFlowPacketNumCount[p.second.packetTotalNum] = 1;
                        }
                        if(p.second.packetTotalNum > 1) {
                            trafficRecord.littleFlowDuration.push_back(p.second.getDurationMicroSec());
                        }
                    }
                }
                if(configInfo["fiveTupleFlowInfo"]) {
                    if(p.second.packetTotalNum > 1) {
                            fprintf(fileManager.fiveTupleFile, "%u %u %llu\n", p.second.packetTotalNum, p.second.packetTotalSize, p.second.getDurationMicroSec());
                    }
                }
            }
            printf("traverse\n");
        }
    }

    void Recorder::flushCache() {
        printf("begin flush recorder infrequent flows\n");
        uint32_t t = 0;
        int allFlow = 0;
        for(auto p = fiveTupleRecord.begin(); p != fiveTupleRecord.end();) {
            allFlow++;
            if(TS_diff_microSec(current_ts_sec, current_ts_usec, (*p).second.edts_sec, (*p).second.edts_usec) >= recordFlushFlowThreshold) {
                int tag = 0;
                if((*p).second.packetTotalSize >= heavyHitterThreshold) {
                    tag = 1;
                    (*p).second.isHeavyHitter = true;

                    if(configInfo["heavyLittleFlowSize"]) {
                        trafficRecord.heavyFlowPacketNum += (*p).second.packetTotalNum;
                        trafficRecord.heavyFlowThroughput += (*p).second.packetTotalSize;
                    }
                    if(configInfo["heavyLittleFlowInfo"]) {
                        if(trafficRecord.heavyFlowPacketNumCount.find((*p).second.packetTotalNum) != trafficRecord.heavyFlowPacketNumCount.end()) {
                            trafficRecord.heavyFlowPacketNumCount[(*p).second.packetTotalNum] += 1;
                        }
                        else {
                            trafficRecord.heavyFlowPacketNumCount[(*p).second.packetTotalNum] = 1;
                        }
                        trafficRecord.heavyFlowDuration.push_back((*p).second.getDurationMicroSec());
                    }
                }
                else {
                    tag = 0;
                    if(configInfo["heavyLittleFlowSize"]) {
                        trafficRecord.littleFlowPacketNum += (*p).second.packetTotalNum;
                        trafficRecord.littleFlowThroughput += (*p).second.packetTotalSize;
                    }
                    if(configInfo["heavyLittleFlowInfo"]) {
                        if(trafficRecord.littleFlowPacketNumCount.find((*p).second.packetTotalNum) != trafficRecord.littleFlowPacketNumCount.end()) {
                            trafficRecord.littleFlowPacketNumCount[(*p).second.packetTotalNum] += 1;
                        }
                        else {
                            trafficRecord.littleFlowPacketNumCount[(*p).second.packetTotalNum] = 1;
                        }
                        if((*p).second.packetTotalNum > 1) {
                            trafficRecord.littleFlowDuration.push_back((*p).second.getDurationMicroSec());
                        }
                    }
                }
                if(configInfo["fiveTupleFlowInfo"]) {
                    if((*p).second.packetTotalNum > 1) {
                        fprintf(fileManager.fiveTupleFile, "%u %u %llu\n", (*p).second.packetTotalNum, (*p).second.packetTotalSize, (*p).second.getDurationMicroSec());
                    }
                }
                ++t;
                p = fiveTupleRecord.erase(p);       //delete infreq flows
            }
            else{
                p++;
            }
        }
        printf("flush %u flows. Total %u flows.\n", t, allFlow);
    }
    void PacketQueue::push(Packet* item){
        std::unique_lock<std::mutex> lock{mutex_};
        queue_.emplace(std::forward<Packet* >(item));
    }
    Packet* PacketQueue::pop(){
        std::unique_lock<std::mutex> lock{mutex_};
        if(queue_.empty()){
            std::cout<<"No Packet in Queue!\n";
            return nullptr;
        }
        Packet* front = std::move(queue_.front());
        queue_.pop();
        return front;
    }
    int PacketQueue::size(){
        std::unique_lock<std::mutex> lock{mutex_};
        return queue_.size();
    }
    bool PacketQueue::empty(){
        std::unique_lock<std::mutex> lock{mutex_};
        return queue_.empty();
    }
    Packet* PacketQueue::top(){
        std::unique_lock<std::mutex> lock{mutex_};
        return queue_.front();
    }
    void PacketQueue::still_working(){
        std::unique_lock<std::mutex> lock{mutex_};
        working = 1;
        return;
    }
    void PacketQueue::stop_working(){
        std::unique_lock<std::mutex> lock{mutex_};
        working = 0;
        return;
    }
    bool PacketQueue::is_working(){
        std::unique_lock<std::mutex> lock{mutex_};
        return working;
    }
    void PacketQueue::Unlock(){
        std::unique_lock<std::mutex> unlock{mutex_};
    }
}
