/*
* author: Wenrui Liu
* last edit time: 2023-4-21
*/
#include<packet.h>
#include<set>
namespace MiniTAP{
    class PcapWriter {
        public:
            PcapWriter(const std::string & _config_path);
            int pass(const Packet * pkt);
            int finish();
            ~PcapWriter();
            void initTargetIP(const std::string & _config_path);

        private:
            int writeFiveTuple(const Packet * pkt);
            int writePcap(const Packet * pkt);
            int writeDNSPcap(const Packet * pkt);
            int writeSpecificIP(const Packet * pkt);
            std::string m_writeDirName;
            uint32_t bgts_sec;
            uint32_t bgts_usec;
            uint32_t edts_sec;
            uint32_t edts_usec;
            std::set<uint32_t> targetIP;
    };
}