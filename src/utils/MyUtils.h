#pragma once

#include <windows.h>

#include <QMap>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <string>


class MyUtils{
public:
    // 宽字符 转 std::string
    std::string static convertWcharToString(wchar_t *wText){
        DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,wText,-1,NULL,0,NULL,FALSE);//WideCharToMultiByte的运用

        char *psText;  // psText为char*的临时数组，作为赋值给std::string的中间变量

        psText = new char[dwNum];

        WideCharToMultiByte (CP_OEMCP,NULL,wText,-1,psText,dwNum,NULL,FALSE);//WideCharToMultiByte的再次运用

        std::string szDst = psText;// std::string赋值

        delete []psText;// psText的清除

        return szDst;
    }

    // map转json字符串
    // static QString mapParseToJson(const QMap<QString, QVariant> &map, int spacing)
    // {
    //     QString jsonString = "{";

    //     auto it = map.begin();
    //     while (it != map.end())
    //     {
    //         // 如果缩进为 0, 不需要换行
    //         if(spacing > 0){
    //             jsonString.append("\n");
    //         }

    //         // 缩进
    //         for(int i= 0; i < spacing; i++){
    //             jsonString.append("\t");
    //         }

    //         // "key": value
    //         jsonString.append("\"")
    //             .append(it.key())
    //             .append("\": ");

    //         // map的值
    //         const QVariant& v = it.value();

    //         // 根据值类型做不同的处理
    //         switch (v.typeId())
    //         {
    //         case QMetaType::Bool:{
    //             jsonString.append(v.toBool() ? "true" : "false");
    //             break;
    //         }

    //         case QMetaType::Int:{
    //             jsonString.append(QString::number(v.toUInt()));
    //             break;
    //         }

    //         case QMetaType::Double:{
    //             jsonString.append(QString::number(v.toDouble()));
    //             break;
    //         }

    //         case QMetaType::QVariantMap:{
    //             // 递归, 缩进到下一层
    //             jsonString.append(mapParseToJson(v.toMap(), spacing + 1));
    //             break;
    //         }

    //         case QMetaType::QStringList:{
    //             jsonString.append("[");
    //             auto list = v.toStringList();
    //             if(list.size() > 0){
    //                 jsonString.append("\"");
    //                 jsonString.append(list.join("\",\""));
    //                 jsonString.append("\"");
    //             }
    //             jsonString.append("]");

    //             break;
    //         }

    //         // 默认字符串类型
    //         default:
    //             jsonString.append("\"")
    //                 .append(v.toString())
    //                 .append("\"");

    //             break;
    //         }

    //         ++it;
    //         // 添加分隔符
    //         if (it != map.end())
    //             jsonString.append(",");
    //     }

    //     if(spacing > 0){
    //         jsonString.append("\n");
    //     }

    //     // 缩进
    //     for(int i= 0; i < spacing - 1; i++){
    //         jsonString.append("\t");
    //     }
    //     jsonString.append("}");

    //     return jsonString;
    // }

};

