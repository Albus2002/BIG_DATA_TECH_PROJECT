/*
* author: Wenrui Liu
* last edit time: 2023-4-10
*/
#ifndef _MINITAP_PCAPREADER_H_
#define _MINITAP_PCAPREADER_H_

#include<util.h>
#include<packet.h>
#include <filesystem>


// main namespace of this project
namespace MiniTAP{
    //file mapping class, for plantform universality
#if defined(_WIN32)
    struct FileMapHandler{
        HANDLE m_fileHandle;
        HANDLE m_fileMapHandle;
        LARGE_INTEGER m_fileSize;
        uint8_t * m_mapData;
        uint64_t m_dataPointer;
        bool m_fastReadByFileMapping;

        FileMapHandler(bool fastReadByFileMapping);
        ~FileMapHandler();
    };
#elif defined(__linux__)
    struct my_size_t{size_t QuadPart = 0;};
    struct FileMapHandler {
        int fd;
        my_size_t m_fileSize;
        uint8_t * m_mapData;
        uint64_t m_dataPointer;
        bool m_fastReadByFileMapping;

        FileMapHandler(bool fastReadByFileMapping);
        ~FileMapHandler();
    };
#endif

    /*
    * an abstract class for read pcap file step by step
    */
    class FileIterator{
        public:
            FileIterator(const std::string & _fileName, bool _fastRead);
            size_t next(RawData * rawData);
            ~FileIterator();
        private:
            FILE * m_fp;
            const std::string m_fileName;
            pcapFileHeader_t m_fileHeader;

            //for file mapping
            FileMapHandler m_fileMapHandler;
    };

    /*
    * an abstract class to read a directory with pcap files
    */
    class DirIterator{
        public:
            DirIterator(const std::string & _dirName, int dirIndex);
            size_t next(RawData * rawData);
            size_t readUntil(RawData * rawData, uint32_t ts_sec, uint32_t ts_usec);
            int isFinished();
            ~DirIterator();


        private:
            //linux read file with wrong order, use priority_queue to promise right order
            // std::queue<std::string> m_fileNameQueue;
            std::priority_queue<std::string, std::vector<std::string>, std::greater<std::string>> m_fileNameQueue;
            const std::string m_dirPath;
            int m_finishFlag;
            const int m_dirIndex;
            FileIterator * m_currentFile;
            uint32_t m_cur_ts_sec;
            uint32_t m_cur_ts_usec;

            int m_totalFileNum;
            int m_curFileNum;
            int m_readProgress;

            std::string getNextFilename();      //get and pop
    };

    /*
    * an abstract class to read all directories 
    */

    class DirReader{
        public:
            DirReader(const std::string & _config_fileName);
            //must: rawData = nullptr
            size_t next(RawData * & rawData);
            ~DirReader();
        private:
            std::priority_queue<RawData *, std::vector<RawData *>, RawDataTSGreater> m_allDirDataQueue;
            std::vector<DirIterator> m_allDirIterators;
            uint32_t m_cur_ts_sec;
            uint32_t m_cur_ts_usec;
            int m_finishFlag;
            int m_totalDirNum;

            int parseConfig(const std::string & _config_fileName);
    };

}

#endif