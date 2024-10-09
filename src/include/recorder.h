/*
* author: Wenrui Liu
* last edit time: 2023-5-7
*/
#ifndef _MINITAP_RECORD_H_
#define _MINITAP_RECORD_H_

#include<switch.h>
#include<unordered_map>
#include<unordered_set>
#include<util.h>
#include<queue>
#include<mutex>
#include<optional>
#include"BOBHash32.h"
#include<set>
// #include <clickhouse/client.h>
// using namespace clickhouse;

namespace MiniTAP{
    struct heavyBucket{
        uint32_t posVote;
        uint32_t negVote;
        bool flag;
        std::optional<fiveTuple> flow;

        heavyBucket(uint32_t pos, uint32_t neg, bool f)
        : posVote(pos), negVote(neg), flag(f), flow{}{
        }
        heavyBucket(){};
    };

    class ElasticSketch {
        public:
            ElasticSketch(uint32_t size, uint32_t threshold);
            ElasticSketch(){};
            void insert(fiveTuple pkt, uint32_t pktSize);
            void evict(fiveTuple flow, uint32_t flowSize);
            std::vector<fiveTuple> findTopK(uint32_t k);
            
        private:
            std::vector<heavyBucket> heavyPart;
            uint32_t heavyPartSize;
            uint32_t threshold;
            BOBHash32 bobHash;
            
    };

    class Recorder {
        public:
            Recorder();
            int init(const std::string & configFileName);
            
            void pass(Packet * pkt);
            void finish();
            ~Recorder();
        private:
            //traffic characteristic record
            std::unordered_map<fiveTuple_t, fiveTupleInfo_t, FiveTupleHash> fiveTupleRecord;
            // std::unordered_map<twoTuple_t, twoTupleInfo_t, TwoTupleHash> twoTupleRecord;
            trafficInfo_t trafficRecord;
            // TCP Congestion Info Recorder
            std::unordered_map<fiveTuple_t, std::vector<TCPPktInfo_t>, FiveTupleHash> TCPCongestionRecord;
            std::unordered_set<fiveTuple_t, FiveTupleHash> TCPStreamRecorded;
            //output file manager
            FileManager fileManager;

            // clickhouse client
            bool clickhouseStore;

            //configuration manager
            std::unordered_map<std::string, bool> configInfo;

            //used for interval data
            uint32_t current_ts_sec;
            uint32_t current_ts_usec;
            uint32_t last_interval_ts_sec;
            uint32_t last_interval_ts_usec;
            uint32_t last_flush_interval_ts_sec;
            uint32_t last_flush_interval_ts_usec;
            uint32_t last_pkt_ts_sec;
            uint32_t last_pkt_ts_usec;
            uint32_t last_flow_ts_sec;
            uint32_t last_flow_ts_usec;
            uint32_t interval_pktSize;
            ElasticSketch sketch;
            uint32_t topK;
            std::unordered_set<fiveTuple_t, FiveTupleHash> interval_activeFlow1;
            std::unordered_set<fiveTuple_t, FiveTupleHash> interval_activeFlow2;
            std::vector<pair<uint32_t, uint32_t>> actAdd;
            std::vector<pair<uint32_t, uint32_t>> actRmv;
            uint16_t actFlowFlag = 0;
            std::unordered_map<fiveTuple_t, std::vector<packetDetail>, FiveTupleHash> topkFlowDetail;
            std::unordered_map<fiveTuple_t, uint32_t, FiveTupleHash> topkFlowHash;
            std::unordered_set<fiveTuple_t, FiveTupleHash> topkFlow;

            const uint32_t burstGap = 1; // ms

            bool needTraverseAfter;

            void flowTraverse();
            void flushCache();
            
            void CreateNewRecord(fiveTuple_t tmp_ft,Packet* pkt);
            void InsertToRecord(fiveTuple_t tmp_ft,Packet* pkt);

            void GetPacketToRecord(fiveTuple_t tmp_ft,Packet* pkt);
            
            void UpdateProtocolCount(Packet* pkt);
            void UpdateArrivalTime(fiveTuple_t tmp_ft,Packet* pkt);//all update function contain "if config[xxx] sentence"
            void UpdateThroughOutput(fiveTuple_t tmp_ft,Packet* pkt);
            void UpdateActiveFlowNum(fiveTuple_t tmp_ft,Packet* pkt);
            void UpdatePacketSize(fiveTuple_t tmp_ft,Packet* pkt);
            void UpdateFlushTime();
            void UpdateTrafficRecord(fiveTuple_t tmp_ft,Packet* pkt);
            void burstCount(Packet *pkt, fiveTuple_t ft);
            

    };
    class PacketQueue{
        public:
            void push(Packet* item);
            Packet* pop();
            int size();
            bool empty();
            Packet* top();
            bool is_working();
            void Unlock();
            void stop_working();
            void still_working();
        private:
            std::queue<Packet*> queue_;
            std::mutex mutex_;
            bool working = false;
    };
}

#endif
