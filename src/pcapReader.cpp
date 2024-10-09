/*
* author: Wenrui Liu
* last edit time: 2023-4-10
*/
#include<pcapReader.h>
MiniTAP::pcapFileHeader_t g_pcapFileHdr{};

// main namespace of this project
namespace MiniTAP{
#if defined(_WIN32)
    FileMapHandler::FileMapHandler(bool fastReadByFileMapping): m_fileHandle(nullptr), m_fileMapHandle(nullptr), m_fileSize{},
    m_mapData(nullptr), m_dataPointer(0), m_fastReadByFileMapping(fastReadByFileMapping){}
    FileMapHandler::~FileMapHandler() {
        if(m_fastReadByFileMapping) {
            UnmapViewOfFile(m_mapData);
            CloseHandle(m_fileHandle);
            CloseHandle(m_fileMapHandle);
            m_mapData = nullptr;
            m_fileHandle = nullptr;
            m_fileMapHandle = nullptr;
        }
    }
#elif defined(__linux__)
    FileMapHandler::FileMapHandler(bool fastReadByFileMapping): fd(0), m_fileSize(), m_mapData(nullptr), 
    m_dataPointer(0), m_fastReadByFileMapping(fastReadByFileMapping){}
    FileMapHandler::~FileMapHandler(){
        munmap((void *)m_mapData, m_fileSize.QuadPart);
        close(fd);
    }
#endif


    /*
    * File Iterator functions
    */
    FileIterator::FileIterator(const std::string & _fileName, bool _fastRead): m_fileName(_fileName),m_fileMapHandler(_fastRead){
        size_t retVal = 0;

        if(m_fileMapHandler.m_fastReadByFileMapping) {       //open file by file map
#if defined(_WIN32)
            m_fileMapHandler.m_fileHandle = CreateFile(m_fileName.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

            GetFileSizeEx(m_fileMapHandler.m_fileHandle, &(m_fileMapHandler.m_fileSize));
            
            m_fileMapHandler.m_fileMapHandle = CreateFileMapping(m_fileMapHandler.m_fileHandle,
                NULL,
                PAGE_READWRITE,
                NULL,
                NULL,
                "Resource");

            m_fileMapHandler.m_mapData = (uint8_t *)MapViewOfFile(m_fileMapHandler.m_fileMapHandle,
                FILE_MAP_ALL_ACCESS,
                NULL,
                NULL,
                NULL);

#elif defined(__linux__)
            struct stat statbuf;
	        stat(m_fileName.c_str(), &statbuf);
            m_fileMapHandler.m_fileSize.QuadPart = statbuf.st_size;
            m_fileMapHandler.fd = open(m_fileName.c_str(), O_RDWR);
            m_fileMapHandler.m_mapData = (uint8_t*)mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, m_fileMapHandler.fd, 0);
            
#endif
            memcpy(&m_fileHeader, m_fileMapHandler.m_mapData, sizeof(pcapFileHeader_t));
            memcpy(&g_pcapFileHdr, m_fileMapHandler.m_mapData, sizeof(pcapFileHeader_t));
            m_fileMapHandler.m_dataPointer += sizeof(pcapFileHeader_t);
        }
        else {  //open file by IO
            m_fp = fopen(m_fileName.c_str(), "rb");
            retVal = fread(&m_fileHeader, sizeof(m_fileHeader), 1, m_fp);
            memcpy(&g_pcapFileHdr, &m_fileHeader, sizeof(pcapFileHeader_t));
            if(retVal == 0) {
                fclose(m_fp);
                m_fp = nullptr;
            }
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
        if(m_fileMapHandler.m_fastReadByFileMapping) {
            if(m_fileMapHandler.m_mapData == nullptr) {
                return 0;
            }
            if(m_fileMapHandler.m_dataPointer + sizeof(rawData->pktHeader) < m_fileMapHandler.m_fileSize.QuadPart) {
                memcpy(&(rawData->pktHeader), m_fileMapHandler.m_mapData+m_fileMapHandler.m_dataPointer, sizeof(rawData->pktHeader));
                m_fileMapHandler.m_dataPointer += sizeof(rawData->pktHeader);
                retVal = rawData->fastReadPktData(m_fileMapHandler.m_mapData, m_fileMapHandler.m_dataPointer);
            }
            else {
                retVal = 0;
            }
        }
        else {
            if(m_fp == nullptr) {
                return 0;
            }
            retVal = fread(&(rawData->pktHeader), sizeof(rawData->pktHeader), 1, m_fp);
            if(retVal == 0) {
                memset(&(rawData->pktHeader), 0, sizeof(rawData->pktHeader));
                return 0;
            }

            retVal = rawData->readPktData(m_fp);
        }
        return retVal;
    }

    FileIterator::~FileIterator() {
        if(!m_fileMapHandler.m_fastReadByFileMapping){
            fclose(m_fp);
            m_fp = nullptr;
        }
    }


    /*
    * Directory Iterator functions
    */
    std::string DirIterator::getNextFilename() {
        if(m_finishFlag == 1) {
            return std::string("");
        }
        std::string currentFilename = m_fileNameQueue.top();
        m_fileNameQueue.pop();
        if(m_fileNameQueue.empty()) {
            m_finishFlag = 1;
        }
        m_curFileNum++;
        if(m_curFileNum*100/m_totalFileNum - m_readProgress >= 1) {
            m_readProgress = m_curFileNum*100/m_totalFileNum;
            printf("[%d%%]Read file nums: %d, total file nums: %d, current file name: %s\n", m_readProgress, m_curFileNum, m_totalFileNum, currentFilename.c_str());
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


    DirIterator::DirIterator(const std::string & _dirName, int dirIndex): m_dirPath(_dirName), m_dirIndex(dirIndex),
        m_fileNameQueue(), m_currentFile(nullptr), m_finishFlag(0), m_cur_ts_sec(0), m_cur_ts_usec(0), m_totalFileNum(0),
        m_curFileNum(0), m_readProgress(0){

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
        else {
            m_totalFileNum = m_fileNameQueue.size();
        }
    }
    

    size_t DirIterator::next(RawData * rawData) {
        size_t retVal = 0;

        if(m_currentFile == nullptr) {
            std::string fileName = getNextFilename();
            if(fileName == "") {
                return 0;
            }

            m_currentFile = new FileIterator(fileName, g_fastReadByFileMapping);
        }

        do {        //ignore out-of-order packets
            retVal = m_currentFile->next(rawData);
            if(retVal == 0) {       //current file read finished, read next
                delete m_currentFile;
                m_currentFile = nullptr;
                std::string nextFile = getNextFilename();
                if(nextFile == "") {
                    return 0;
                }
                m_currentFile = new FileIterator(nextFile, g_fastReadByFileMapping);
                retVal = m_currentFile->next(rawData);
            }
        } while(TSLessThan(rawData, m_cur_ts_sec, m_cur_ts_usec));
        m_cur_ts_sec = rawData->pktHeader.ts_sec;
        m_cur_ts_usec = rawData->pktHeader.ts_usec;
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


    size_t DirIterator::readUntil(RawData * rawData, uint32_t ts_sec, uint32_t ts_usec) {
        size_t retVal = 0;
        while(TSLessThan(rawData, ts_sec, ts_usec)) {
            retVal = next(rawData);
            if(retVal == 0) {
                return 0;
            }
        }
        return retVal;
    }


    DirIterator::~DirIterator() {
        if(m_currentFile != nullptr) {
            delete m_currentFile;
            m_currentFile = nullptr;
        }
    }


    /*
    * Directory Reader functions
    */
    int DirReader::parseConfig(const std::string & _config_fileName){
        std::string tmp_path;
        
        std::ifstream configFile(_config_fileName);
        while(configFile>>tmp_path) {
//            printf("%s\n", tmp_path.c_str());
            m_allDirIterators.push_back(DirIterator(tmp_path, m_totalDirNum++));
        }
        return 0;
    }


    DirReader::DirReader(const std::string & _config_fileName): m_allDirDataQueue(), m_allDirIterators(),
        m_cur_ts_sec(0), m_cur_ts_usec(0), m_finishFlag(0), m_totalDirNum(0){
        uint32_t first_ts_sec = 0, first_ts_usec = 0;
        parseConfig(_config_fileName);

        //find first common timestamp
        for(int i = 0 ; i < m_totalDirNum; ++i) {
            RawData * tmp_data = new RawData(i);
            size_t retVal = m_allDirIterators[i].next(tmp_data);
            if(retVal != 0) {
                if(!TSLessThan(tmp_data, first_ts_sec, first_ts_usec)) {
                    first_ts_sec = tmp_data->pktHeader.ts_sec;
                    first_ts_usec = tmp_data->pktHeader.ts_usec;
                }
                m_allDirDataQueue.push(tmp_data);
            }
        }

        m_cur_ts_sec = first_ts_sec;
        m_cur_ts_usec = first_ts_usec;

        for(int i = 0; i < m_totalDirNum-1; ++i) {
            RawData * tmp_data = m_allDirDataQueue.top();
            m_allDirDataQueue.pop();
            delete tmp_data;
        }

        int first_index = m_allDirDataQueue.top()->dirIndex;
        for(int i = 0; i < m_totalDirNum; ++i) {
            if(i == first_index) {
                continue;
            }
            else {
                RawData * tmp_data = new RawData(i);
                size_t retVal = m_allDirIterators[i].readUntil(tmp_data, m_cur_ts_sec, m_cur_ts_usec);
                if(retVal != 0) {
                    m_allDirDataQueue.push(tmp_data);
                }
            }
        }
    }

    //must: rawData = nullptr
    size_t DirReader::next(RawData * & rawData) {
        size_t retVal = 0;
        if(m_finishFlag == 1) {
            return retVal;
        }
        
        rawData = m_allDirDataQueue.top();
        m_allDirDataQueue.pop();
        
        RawData* tmp_data = new RawData(rawData->dirIndex);
        retVal = m_allDirIterators[rawData->dirIndex].next(tmp_data);
        if(retVal == 0) {
            m_finishFlag = 1;
        }
        else {
            m_allDirDataQueue.push(tmp_data);
            m_cur_ts_sec = m_allDirDataQueue.top()->pktHeader.ts_sec;
            m_cur_ts_usec = m_allDirDataQueue.top()->pktHeader.ts_usec;
        }

        retVal = rawData->pktHeader.caplen;
        return retVal;
    }

    DirReader::~DirReader() {
        while(!m_allDirDataQueue.empty()) {
            RawData * tmp = m_allDirDataQueue.top();
            m_allDirDataQueue.pop();
            delete tmp;
        }
    }
}