//reorder.h
#include<cstdint>
#include<cstdio>
#include<cstring>
#include<string>
#include<vector>
#include<queue>
#include<fstream>
#include<unordered_map>
#include<regex>
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


// raw data of a pcap packet
class RawData{
    public:
        pcapPacketHeader_t pktHeader;
        uint8_t * pktData;
        int dirIndex;
        
        RawData(const int _dirIndex);
        RawData(const pcapPacketHeader_t & _pktHeader, const int _dirIndex);
        size_t readPktData(FILE * fp);
        size_t fastReadPktData(uint8_t * mapData, uint64_t & mapPointer);
        ~RawData();
};

struct RawDataTSLess{
    bool operator()(const RawData * r1, const RawData * r2);
};

struct RawDataTSGreater{
    bool operator()(const RawData * r1, const RawData * r2);
};


/*
* an abstract class for read pcap file step by step
*/
class FileIterator{
    public:
        FileIterator(const std::string & _fileName);
        size_t next(RawData * rawData);
        ~FileIterator();
        FILE * m_fp;
        const std::string m_fileName;
        pcapFileHeader_t m_fileHeader;
};

/*
* an abstract class to read a directory with pcap files
*/
class DirIterator{
    public:
        DirIterator(const std::string & _dirName);
        size_t next(RawData * rawData, int & isNewFile);
        int isFinished();
        void getPcapHeader(pcapFileHeader_t & pcapHdr);
        ~DirIterator();

    private:
        //linux read file with wrong order, use priority_queue to promise right order
        // std::queue<std::string> m_fileNameQueue;
        std::priority_queue<std::string, std::vector<std::string>, std::greater<std::string>> m_fileNameQueue;
        const std::string m_dirPath;
        int m_finishFlag;
        FileIterator * m_currentFile;

        std::string getNextFilename();      //get and pop
};


//reorder.cpp
const int TAP_BIG_ENDIAN = 0;
const int TAP_LITTLE_ENDIAN = 1;
const size_t writeBufferSize = 505*1024*1024;
int g_pcapByteOrder = TAP_BIG_ENDIAN;
bool g_pcapByteOrderDecided = false;
bool g_pcapByteOrderError = false;
int reorder_mode = 0;

uint8_t writeBuffer[writeBufferSize] = {};
size_t bufferPointer = 0;
std::priority_queue<RawData *, std::vector<RawData *>, RawDataTSGreater> pktQueue;
uint32_t curFileBgn_sec = 0, curFileBgn_usec = 0;
uint32_t curFileEnd_sec = 0, curFileEnd_usec = 0;
uint32_t bg_run_sec = 0, bg_run_usec = 0;
pcapFileHeader_t pcapFileHdr;
std::string srcDirName{};
std::string dstDirName{};

RawData::RawData(const int _dirIndex): dirIndex(_dirIndex) {
    pktHeader.ts_sec = 0;
    pktHeader.ts_usec = 0;
    pktHeader.caplen = 0;
    pktHeader.len = 0;
    pktData = nullptr;
}


RawData::RawData(const pcapPacketHeader_t & _pktHeader, const int _dirIndex): dirIndex(_dirIndex) {
    pktHeader.ts_sec = _pktHeader.ts_sec;
    pktHeader.ts_usec = _pktHeader.ts_usec;
    pktHeader.caplen = _pktHeader.caplen;
    pktHeader.len = _pktHeader.len;
    pktData = static_cast<uint8_t*>(malloc(pktHeader.caplen));
}

size_t RawData::readPktData(FILE * fp) {
    size_t retVal = 0;
    if(pktHeader.caplen == 0) {
        return 0;
    }
    
    if(pktData != nullptr) {
        free(pktData);
        pktData = nullptr;
    }

    pktData = static_cast<uint8_t*>(malloc(pktHeader.caplen));
    retVal = fread(pktData, sizeof(uint8_t), pktHeader.caplen, fp);
    return retVal;
}

RawData::~RawData() {
    if(pktData != nullptr) {
        free(pktData);
    }
    pktData = nullptr;
}


bool RawDataTSLess::operator()(const RawData * r1, const RawData * r2) {
    if(r1->pktHeader.ts_sec == r2->pktHeader.ts_sec) {
        return r1->pktHeader.ts_usec < r2->pktHeader.ts_usec;
    }
    else {
        return r1->pktHeader.ts_sec < r2->pktHeader.ts_sec;
    }
}


bool RawDataTSGreater::operator()(const RawData * r1, const RawData * r2) {
    if(r1->pktHeader.ts_sec == r2->pktHeader.ts_sec) {
        return r1->pktHeader.ts_usec > r2->pktHeader.ts_usec;
    }
    else {
        return r1->pktHeader.ts_sec > r2->pktHeader.ts_sec;
    }
}


/*
* File Iterator functions
*/
FileIterator::FileIterator(const std::string & _fileName): m_fileName(_fileName){
    size_t retVal = 0;

    m_fp = fopen(m_fileName.c_str(), "rb");
    retVal = fread(&m_fileHeader, sizeof(m_fileHeader), 1, m_fp);
    if(retVal == 0) {
        fclose(m_fp);
        m_fp = nullptr;
    }
    
    if(!g_pcapByteOrderDecided) {
        if(m_fileHeader.magic == 0xA1B2C3D4) {
            g_pcapByteOrderDecided = true;
            g_pcapByteOrder = TAP_BIG_ENDIAN;
        }
        else if(m_fileHeader.magic == 0xD4C3B2A1){
            g_pcapByteOrderDecided = true;
            g_pcapByteOrder = TAP_LITTLE_ENDIAN;
        }
    }
    else {
        if(m_fileHeader.magic == 0xA1B2C3D4 && g_pcapByteOrder == TAP_LITTLE_ENDIAN) {
            g_pcapByteOrderError = true;
        }
        else if(m_fileHeader.magic == 0xD4C3B2A1 && g_pcapByteOrder == TAP_BIG_ENDIAN) {
            g_pcapByteOrderError = true;
        }
    }
}


size_t FileIterator::next(RawData * rawData) {
    size_t retVal = 0;
    if(m_fp == nullptr) {
        return 0;
    }
    retVal = fread(&(rawData->pktHeader), sizeof(rawData->pktHeader), 1, m_fp);
    if(retVal == 0) {
        memset(&(rawData->pktHeader), 0, sizeof(rawData->pktHeader));
        return 0;
    }

    retVal = rawData->readPktData(m_fp);
    return retVal;
}

FileIterator::~FileIterator() {
    fclose(m_fp);
    m_fp = nullptr;
}

    /*
    * Directory Iterator functions
    */
    std::string DirIterator::getNextFilename() {
        if(m_finishFlag == 1) {
            return std::string("");
        }
        std::string currentFilename = m_fileNameQueue.top();
        // printf("begin read: %s, buffer size: %u\n",currentFilename.c_str(), bufferPointer);
        m_fileNameQueue.pop();
        if(m_fileNameQueue.empty()) {
            m_finishFlag = 1;
        }
        return currentFilename;
    }


    bool TSLessThan(RawData * rawData, uint32_t ts_sec, uint32_t ts_usec) {
        if(rawData->pktHeader.ts_sec == ts_sec) {
            return rawData->pktHeader.ts_usec < ts_usec;
        }
        else {
            return rawData->pktHeader.ts_sec < ts_sec;
        }
    }

    bool TSLessThan(uint32_t ts_sec_1, uint32_t ts_usec_1, uint32_t ts_sec_2, uint32_t ts_usec_2) {
        if(ts_sec_1 < ts_sec_2) {
            return true;
        }
        else {
            return ts_usec_1 < ts_usec_2;
        }
    }


DirIterator::DirIterator(const std::string & _dirName): m_dirPath(_dirName), 
    m_fileNameQueue(), m_currentFile(nullptr), m_finishFlag(0){

#if defined(_WIN32)
    struct _finddata_t fileInfo;
    intptr_t hFile = 0;
    std::string p;

    if((hFile = _findfirst(p.assign(m_dirPath).append("\\*").c_str(), &fileInfo)) != -1) {
        do {
            if((fileInfo.attrib & _A_SUBDIR)) {     //dir, continue
                continue;
            }
            else {  //file
                if(std::strcmp(fileInfo.name, ".") != 0 && std::strcmp(fileInfo.name, "..") != 0) {
                    m_fileNameQueue.push(p.assign(m_dirPath).append("\\").append(fileInfo.name));
                }
            }
        } while (_findnext(hFile, &fileInfo) == 0);
        _findclose(hFile);
    }
#elif defined(__linux__)
    DIR *pDir;
    struct dirent *ptr;
    if((pDir = opendir(m_dirPath.c_str())) != nullptr) {
        while ((ptr = readdir(pDir)) != 0) {
            // strcmp是C语言里的，只导入string,然后std::strcmp都是没有的
            if (std::strcmp(ptr->d_name, ".") != 0 && std::strcmp(ptr->d_name, "..") != 0) {
                m_fileNameQueue.push(m_dirPath + "/" + ptr->d_name);  // 可以只保留名字
            }
        }
        closedir(pDir);
    }
    
#endif

    if(m_fileNameQueue.empty()) {
        m_finishFlag = 1;
    }
}

void DirIterator::getPcapHeader(pcapFileHeader_t & pcapHdr) {
    memcpy(&pcapHdr, &(m_currentFile->m_fileHeader), sizeof(pcapFileHeader_t));
    return;
}

size_t DirIterator::next(RawData * rawData, int & isNewFile) {
    size_t retVal = 0;

    if(m_currentFile == nullptr) {
        std::string fileName{};
        if(reorder_mode == 0) {
            fileName = getNextFilename();
        }
        else if(reorder_mode == 1) {
            while(true) {
                fileName = getNextFilename();
                if(fileName == "") {
                    return 0;
                }
                std::regex pattern("pa_\\d+_(\\d+)_(\\d+)\\.pcap");
                std::smatch match;
                if (std::regex_search(fileName, match, pattern) && match.size() > 2) {
                    uint32_t ed_sec = static_cast<uint32_t>(std::stoull(match[1].str()));
                    uint32_t ed_usec = static_cast<uint32_t>(std::stoull(match[2].str()));
                    if(!TSLessThan(ed_sec, ed_usec, bg_run_sec, bg_run_usec)) {
                        printf("file %s match, begin read\n", fileName.c_str());
                        break;
                    }
                        printf("file %s match, pass\n", fileName.c_str());
                }
                else {
                    return 0;
                }
            }
        }
        isNewFile = 1;    
        if(fileName == "") {
            return 0;
        }
        m_currentFile = new FileIterator(fileName);
    }

    retVal = m_currentFile->next(rawData);
    if(retVal == 0) {       //current file read finished, read next
        delete m_currentFile;
        m_currentFile = nullptr;
        std::string nextFile = getNextFilename();
        isNewFile = 1;
        if(nextFile == "") {
            return 0;
        }
        m_currentFile = new FileIterator(nextFile);
        retVal = m_currentFile->next(rawData);
    }
    return retVal;
}


int DirIterator::isFinished() {
    if(m_currentFile == nullptr && m_finishFlag == 1) {
        return 1;
    } 
    else {
        return 0;
    }
}

DirIterator::~DirIterator() {
    if(m_currentFile != nullptr) {
        delete m_currentFile;
        m_currentFile = nullptr;
    }
}



void popDataQueueUntil(uint32_t pktTS_sec, uint32_t pktTS_usec) {
    while(!pktQueue.empty() && TSLessThan(pktQueue.top(), pktTS_sec, pktTS_usec)) {
        RawData * tmp = pktQueue.top();
        pktQueue.pop();
        if(bufferPointer >= 500*1024*1024) {
            std::string path = dstDirName + "/pa_"+std::to_string(curFileBgn_sec)+"_"+std::to_string(curFileBgn_usec)
                +"_"+std::to_string(curFileEnd_sec)+"_"+std::to_string(curFileEnd_usec)+".pcap";
            printf("write file: %s, size: %lu\n", path.c_str(), bufferPointer);
            FILE * fp = fopen(path.c_str(), "wb");
            fwrite(writeBuffer, bufferPointer, sizeof(uint8_t), fp);
            fclose(fp);
            bufferPointer = 0;
        }
        if(bufferPointer == 0) {
            memset(writeBuffer, 0, writeBufferSize);
            memcpy(writeBuffer, &pcapFileHdr, sizeof(pcapFileHeader_t));
            bufferPointer += sizeof(pcapFileHeader_t);
            curFileBgn_sec = tmp->pktHeader.ts_sec;
            curFileBgn_usec = tmp->pktHeader.ts_usec;
        }

        memcpy(writeBuffer+bufferPointer, &(tmp->pktHeader), sizeof(pcapPacketHeader_t));
        bufferPointer += sizeof(pcapPacketHeader_t);
        memcpy(writeBuffer+bufferPointer, tmp->pktData, tmp->pktHeader.caplen);
        bufferPointer += tmp->pktHeader.caplen;

        curFileEnd_sec = tmp->pktHeader.ts_sec;
        curFileEnd_usec = tmp->pktHeader.ts_usec;
        delete tmp;
    }
}

int main(int argc, char * argv[]) {
    printf("begin reorder\n");
    if(argc != 3 && argc != 5) {
        printf("./reorder srcDir dstDir (ts_sec ts_usec)\n");
        return 0;
    }
    srcDirName = std::string{argv[1]};
    dstDirName = std::string{argv[2]};
    if(argc == 5) {
        reorder_mode = 1;
        bg_run_sec = static_cast<uint32_t>(std::stoull(argv[3]));
        bg_run_usec = static_cast<uint32_t>(std::stoull(argv[4]));
        printf("reorder mode 1, bg: %lu %lu\n", bg_run_sec, bg_run_usec);
    }
    printf("srcDir: %s, dstDir: %s\n", srcDirName.c_str(), dstDirName.c_str());
    RawData * tmp_data = nullptr;

    DirIterator dirIter(srcDirName);
    int isNewFile = 0;
    size_t retVal = 1;

    while(retVal) {
        tmp_data = new RawData(0);

        retVal = dirIter.next(tmp_data, isNewFile);
        if(retVal == 0) {
            break;
        }
        else {
            if(isNewFile) {
                dirIter.getPcapHeader(pcapFileHdr);
                popDataQueueUntil(tmp_data->pktHeader.ts_sec, tmp_data->pktHeader.ts_usec);
            }
            if(reorder_mode == 0) {
                pktQueue.push(tmp_data);
                tmp_data = nullptr;
            }
            else if(reorder_mode == 1){
                if(TSLessThan(tmp_data, bg_run_sec, bg_run_usec)) {
                    delete tmp_data;
                }
                else {
                    pktQueue.push(tmp_data);
                    reorder_mode = 0;
                }
                tmp_data = nullptr;
            }
        }
        isNewFile = 0;
    }

    popDataQueueUntil(0xffffffff, 0xffffffff);
    if(bufferPointer > 0) {
        std::string path = dstDirName + "/pa_"+std::to_string(curFileBgn_sec)+"_"+std::to_string(curFileBgn_usec)
            +"_"+std::to_string(curFileEnd_sec)+"_"+std::to_string(curFileEnd_usec)+".pcap";
        FILE * fp = fopen(path.c_str(), "wb");
        fwrite(writeBuffer, sizeof(uint8_t), bufferPointer, fp);
        fclose(fp);
    }
    
    return 0;
}