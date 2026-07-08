#ifndef UTILS_H
#define UTILS_H

#include<string>
#include <windows.h>
#include<mapping_relation.h>


class Utils{
public:
    std::string static convertWcharToString(wchar_t *wText){
        DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,wText,-1,NULL,0,NULL,FALSE);//WideCharToMultiByte的运用

        char *psText;  // psText为char*的临时数组，作为赋值给std::string的中间变量

        psText = new char[dwNum];

        WideCharToMultiByte (CP_OEMCP,NULL,wText,-1,psText,dwNum,NULL,FALSE);//WideCharToMultiByte的再次运用

        std::string szDst = psText;// std::string赋值

        delete []psText;// psText的清除

        return szDst;
    }
};

#endif // UTILS_H
