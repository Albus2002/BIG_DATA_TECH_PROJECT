/*
* author: Wenrui Liu
* last edit time: 2023-4-9
*/
#include<util.h>
#include<crc32.h>

namespace MiniTAP{
    std::size_t FiveTupleHash::operator()(const fiveTuple_t & ft) const{
        return crc32_1byte(&(ft), sizeof(fiveTuple_t));
    }

    std::size_t TwoTupleHash::operator()(const twoTuple_t & tt) const{
        return crc32_1byte(&(tt), sizeof(fiveTuple_t));
    }

#ifdef _WIN32
    std::wstring s2ws(const std::string& s)
    {
        int len;
        int slength = (int)s.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
    }
    
    LPWSTR ws2LPWSTR(const std::wstring& ws) {
        return const_cast<LPWSTR>(ws.c_str());
    }

    LPWSTR s2LPWSTR(const std::string& s) {
        std::wstring tmp = s2ws(s);
        return ws2LPWSTR(tmp);
    }
#endif

}
