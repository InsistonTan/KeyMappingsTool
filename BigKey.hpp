#pragma once
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

using std::deque;
using std::istream;
using std::ostream;
using std::string;
using std::vector;

#define MAX_BUTTONS 128

// 参考开源库 高精度浮点类：https://gitee.com/chenjjian/bigfloat/blob/master/BigFloat.h

class BigKey {
    // 基本运算符重载
    const friend BigKey operator&(const BigKey&, const BigKey&);  // 按位与重载
    const friend BigKey operator|(const BigKey&, const BigKey&);  // 按位或重载
    const friend BigKey operator~(BigKey&);                       // 取反重载

    friend BigKey operator&=(BigKey&, const BigKey&);  // 按位与重载
    friend BigKey operator|=(BigKey&, const BigKey&);  // 按位或重载

    // 比较重载
    friend bool operator==(const BigKey&, const BigKey&);  // 等于重载
    friend bool operator!=(const BigKey&, const BigKey&);  // 不等于重载

    friend bool operator&&(const BigKey&, const BigKey&);  // 逻辑与重载
    friend bool operator&&(const BigKey&, const bool&);    // 逻辑与重载
    friend bool operator&&(const bool&, const BigKey&);    // 逻辑与重载

    // 输入输出重载
    friend ostream& operator<<(ostream&, const BigKey&);    // 输出重载
    friend istream& operator>>(istream&, BigKey&);          // 输入重载
    friend QDebug operator<<(QDebug dbg, const BigKey& p);  // 调试重载 （注意，不是返回引用）

#define BIGKEY_ZERO BigKey::ZERO()

public:
    BigKey();
    BigKey(const BigKey&);                //
    BigKey(const string&);                // 用一个字符串构造
    BigKey(BigKey&&) noexcept;            // 移动构造
    BigKey operator=(const BigKey&);      // 赋值函数
    BigKey operator=(BigKey&&) noexcept;  // 移动赋值

    // 转换为字符串
    string toString() const;           // decimalNum 用于控制小数位数，赋值为0时小数部分全部输出
    void setBit(int pos, bool value);  // 设置按键值
    void clear();                      // 清空按键值

    static const BigKey& ZERO() {
        static BigKey zero;
        return zero;
    };

private:
    string key;
};

inline BigKey::BigKey()  // 默认构造函数
{
    key= string(MAX_BUTTONS, '0');  // 初始化为128个0
}

inline BigKey::BigKey(const BigKey& num) {
    key= num.key;  // 交换字符串
}

inline BigKey::BigKey(BigKey&& num) noexcept  // 移动构造
{
    key.swap(num.key);  // 交换字符串
}

inline BigKey BigKey::operator=(const BigKey& num)  // 赋值（拷贝）操作
{
    key= num.key;  // 直接赋值
    return *this;
}

inline BigKey BigKey::operator=(BigKey&& num) noexcept {
    key= num.key;  // 直接赋值
    return *this;
}

inline BigKey operator&=(BigKey& num1, const BigKey& num2) {
    for (int i= 0; i < MAX_BUTTONS; i++) {
        if (num1.key[i] == '1' && num2.key[i] == '1') {
            num1.key[i]= '1';
        } else {
            num1.key[i]= '0';
        }
    }
    return num1;
}

inline BigKey operator|=(BigKey& num1, const BigKey& num2) {
    for (int i= 0; i < MAX_BUTTONS; i++) {
        if (num1.key[i] == '1' || num2.key[i] == '1') {
            num1.key[i]= '1';
        } else {
            num1.key[i]= '0';
        }
    }
    return num1;
}

inline const BigKey operator&(const BigKey& num1, const BigKey& num2)  // 按位与重载
{
    BigKey temp;
    for (int i= 0; i < MAX_BUTTONS; i++) {
        if (num1.key[i] == '1' && num2.key[i] == '1') {
            temp.key[i]= '1';
        } else {
            temp.key[i]= '0';
        }
    }
    return temp;
}

inline const BigKey operator|(const BigKey& num1, const BigKey& num2)  // 按位或重载
{
    BigKey temp(num1);
    for (int i= 0; i < MAX_BUTTONS; i++) {
        if (num1.key[i] == '1' || num2.key[i] == '1') {
            temp.key[i]= '1';
        } else {
            temp.key[i]= '0';
        }
    }
    return temp;
}

inline const BigKey operator~(BigKey& num1) {
    // 取反重载
    BigKey temp(num1);
    for (int i= 0; i < MAX_BUTTONS; i++) {
        if (num1.key[i] == '1') {
            temp.key[i]= '0';
        } else {
            temp.key[i]= '1';
        }
    }
    return temp;
}

inline void BigKey::setBit(int pos, bool value) {
    if (pos < 0 || pos >= MAX_BUTTONS) {
        throw std::out_of_range("Position out of range");
    }
    if (value) {
        key[pos]= '1';  // 设置为1
    } else {
        key[pos]= '0';  // 设置为0
    }
}

inline void BigKey::clear() {
    // 清空按键值
    key= string(MAX_BUTTONS, '0');
}

inline bool operator&&(const BigKey& num1, const BigKey& num2) {
    // 逻辑与重载
    if (num1.key.find('1') != string::npos && num2.key.find('1') != string::npos) {
        return true;  // 都有1
    }
    return false;
}

inline bool operator&&(const BigKey& num1, const bool& num2) {
    // 逻辑与重载
    if (num2 == false) {
        return false;  // 如果num2为false，直接返回false
    }
    if (num1.key.find('1') != string::npos) {
        return true;  // 都有1
    }
    return false;
}

inline bool operator&&(const bool& num1, const BigKey& num2) {
    // 逻辑与重载
    if (num1 == false) {
        return false;  // 如果num2为false，直接返回false
    }
    if (num2.key.find('1') != string::npos) {
        return true;  // 都有1
    }
    return false;
}

inline string BigKey::toString() const {
    return key;  // 返回字符串
}

inline BigKey::BigKey(const string& num)  // 用字符串初始化
{
    key= num;  // 直接赋值
    // 检查字符串长度是否超过128位
    if (key.length() > MAX_BUTTONS) {
        throw std::out_of_range("String length exceeds maximum size of 128 bits");
    }
    key.resize(MAX_BUTTONS, '0');  // 补齐到128位
}

inline ostream& operator<<(ostream& out, const BigKey& num)  // 输出重载
{
    for (int i= MAX_BUTTONS - 1; i >= 0; i--)  // 输出整数部分
    {
        out << (char)(num.key[i]);
    }
    return out;
}

inline QDebug operator<<(QDebug dbg, const BigKey& num)  // 输出重载 （注意，不是返回引用）
{
    QString str= "";
    for (int i= MAX_BUTTONS - 1; i >= 0; i--)  // 输出整数部分
    {
        str+= (char)(num.key[i]);
    }
    dbg << qPrintable(str);  // 打印时没有双引号
    return dbg;
}

inline istream& operator>>(istream& in, BigKey& num)  // 输入重载
{
    string str;
    in >> str;
    num= BigKey(str);
    return in;
}

inline bool operator==(const BigKey& num1, const BigKey& num2)  // 等于重载
{
    return num1.key == num2.key;  // 比较字符串是否相等
}

inline bool operator!=(const BigKey& num1, const BigKey& num2)  // 不等于重载
{
    return !(num1 == num2);  // 调用等于重载函数
}
