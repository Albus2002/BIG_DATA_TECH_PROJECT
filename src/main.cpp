/*
* author: Wenrui Liu
* last edit time: 2023-4-13
*/
#include <pcapReader.h>
#include <pcapWriter.h>
#include <switch.h>
#include <recorder.h>
#include <csignal>
#include <unistd.h>
#include <thread>
#include <unistd.h>
#include <iostream>
#include <filesystem>

#ifdef _RECORD_MODE_
    MiniTAP::Recorder recorder;
#endif

void handle_sigint(int sig);

int reading = 0;
int processing = 0;
int test_read_order = 0;
void get_data(MiniTAP::DirReader & reader,MiniTAP::PacketQueue & pktque){
    pktque.still_working();
    printf("pcapReader thread begin\n");
    MiniTAP::RawData * rawData = nullptr;
    MiniTAP::Packet * pkt = nullptr;
    while(reader.next(rawData)) {
        reading++;
        pkt = new MiniTAP::Packet(rawData);
        pkt->Parse();
        if(!pkt->parse_error){
            pktque.push(pkt);
        }
        if(pktque.size() > 2000000){
            while(1){
                pktque.Unlock();
                sleep(5);
                if(pktque.size()==0) break;
            }
            
            fprintf(stdout,"%d\n",pktque.size());
        }
        delete rawData;
        rawData = nullptr;
        
        if(reading % 1000000 == 0) {
            //fprintf(stdout,"size: %llu",sizeof(pktque));
            fprintf(stdout,"reading...\n");
        }
    }
    fprintf(stdout,"1\n");
    pktque.stop_working();
    return;
}

void record_data(MiniTAP::PacketQueue & pktque,MiniTAP::Recorder & recorder){
    printf("Recorder thread begin\n");
    MiniTAP::Packet* pkt = nullptr;
    time_t bg_time;
    bg_time = time(nullptr);
    while(true){
        bool flag = pktque.is_working();
        if(flag && pktque.empty()){
            pktque.Unlock();
            sleep(1);
            continue;
        }
        if(!flag && pktque.empty()){
            break;
        }
         // if(pktque.size() >= PktQueueLength) {
        //     printf("recorder still working\n");
        // }
        processing++;
        pkt = pktque.top();
        recorder.pass(pkt);
        pktque.pop();
        delete pkt;
        pkt = nullptr;
        if(processing % 1000000 == 0){ 
            fprintf(stdout,"processing...");
            printf("Process 100w packet time cost: %d(s)\n", time(nullptr) - bg_time);
            bg_time = time(nullptr);
        }
    }
    fprintf(stdout,"Recorder thread end\n");
    return;
}

void write_data(MiniTAP::PacketQueue & pktque,MiniTAP::PcapWriter & writer){
    printf("Writer thread begin\n");
    MiniTAP::Packet* pkt = nullptr;
    time_t bg_time;
    bg_time = time(nullptr);
    while(true){
        bool flag = pktque.is_working();
        if(flag && pktque.empty()){
            pktque.Unlock();
            sleep(1);
            continue;
        }
        if(!flag && pktque.empty()){
            break;
        }
         // if(pktque.size() >= PktQueueLength) {
        //     printf("recorder still working\n");
        // }
        processing++;
        pkt = pktque.top();
        writer.pass(pkt);
        pktque.pop();
        delete pkt;
        pkt = nullptr;
        if(processing % 1000000 == 0){ 
            fprintf(stdout,"processing...");
            printf("Process 100w packet time cost: %d(s)\n", time(nullptr) - bg_time);
            bg_time = time(nullptr);
        }
    }
    fprintf(stdout,"Writer thread end\n");
    return;
}

int main() {
    std::printf("Hello from miniTAP\n");\
    std::printf("The file mapping is %s\n",g_fastReadByFileMapping?"open":"not open");
    time_t bg_time, ed_time;
    bg_time = time(nullptr);
    cout << std::filesystem::current_path() << endl;
    MiniTAP::DirReader reader("../config/srcAddrConfig");
    MiniTAP::PacketQueue pktque;
    // signal(SIGINT, handle_sigint);
#ifdef _WRITE_MODE_
    MiniTAP::PcapWriter writer("../config/writeAddrConfig");
    writer.initTargetIP("../config/targetIP");
#endif

#ifdef _RECORD_MODE_
    cout << "recorder.init" << endl;
    recorder.init("../config/recorderConfig");
#endif
#ifdef _RECORD_MODE_
    std::thread t1(get_data,std::ref(reader),std::ref(pktque));
    std::thread t2(record_data,std::ref(pktque),std::ref(recorder));
#endif

#ifdef _WRITE_MODE_
    std::thread t1(get_data,std::ref(reader),std::ref(pktque));
    std::thread t2(write_data,std::ref(pktque),std::ref(writer));
    // writer.pass(pkt);
#endif

    // ed_time = time(nullptr);
    // std::printf("Pcap reading spend time: %d(s)\n", ed_time - bg_time);

#ifdef _RECORD_MODE_
    t1.join();
    t2.join();
    recorder.finish();
#endif
#ifdef _WRITE_MODE_
    t1.join();
    t2.join();
    printf("write finish st");
    writer.finish();
    printf("write finish en");
#endif

    ed_time = time(nullptr);
    std::printf("Total spend time: %lld(s)\n", (int)(ed_time - bg_time));
    std::printf("read %d ,process %d\n",reading,processing);
    return 0;
}

// packet相当于自己写的一个包含包所有信息的类
// pass相当于一个中间件，每来一个包都会进行处理
// 

void handle_sigint(int sig){
    printf("sig%d\n", sig);
    #ifdef _RECORD_MODE_
    recorder.finish();
    #endif
    exit(sig);
}