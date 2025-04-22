#pragma once
#define ENABLE_QT true

#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>

using std::deque;
using std::istream;
using std::ostream;
using std::string;
using std::vector;

#if ENABLE_QT
#include <QDebug>
#endif

// 参考开源库
// 高精度浮点类：https://gitee.com/chenjjian/bigfloat/blob/master/BigFloat.h

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
    friend ostream& operator<<(ostream&, const BigKey&);  // 输出重载
    friend istream& operator>>(istream&, BigKey&);        // 输入重载
#if ENABLE_QT
    friend QDebug operator<<(QDebug dbg, const BigKey& p);  // 调试重载   （注意，不是返回引用）
#endif

public:
    BigKey();
    BigKey(const BigKey&);                //
    BigKey(const string&);                // 用一个字符串构造
    BigKey(BigKey&&) noexcept;            // 移动构造
    BigKey operator=(const BigKey&);      // 赋值函数
    BigKey operator=(BigKey&&) noexcept;  // 移动赋值

    // 类型转换
    operator bool() const;  // 转换为bool类型

    // 转换为字符串
    string toString() const;              // decimalNum 用于控制小数位数，赋值为0时小数部分全部输出
    void setBit(size_t pos, bool value);  // 设置按键值
    void clear();                         // 清空按键值

    static const BigKey& ZERO() {
        static BigKey zero;
        return zero;
    };

#define BIGKEY_ZERO BigKey::ZERO()

private:
#define BIGKEY_NUMBER 2  // 128位按键值，使用数组存储
#define BIGKEY_TYPE_INTERNAL uint64_t  // 内部存储类型

#define MAX_BUTTONS (BIGKEY_NUMBER * (sizeof(BIGKEY_TYPE_INTERNAL) * 8))

    BIGKEY_TYPE_INTERNAL key[BIGKEY_NUMBER];  // 128位按键值，使用数组存储
};

inline BigKey::BigKey()  // 默认构造函数
{
    this->clear();  // 清空按键值
}

inline BigKey::BigKey(const BigKey& num) {
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        key[i] = num.key[i];  // 直接赋值
    }
}

// 移动构造
inline BigKey::BigKey(BigKey&& num) noexcept {
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        key[i] = num.key[i];  // 直接赋值
    }
}
// 赋值（拷贝）操作
inline BigKey BigKey::operator=(const BigKey& num) {
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        key[i] = num.key[i];  // 直接赋值
    }
    return *this;
}

inline BigKey BigKey::operator=(BigKey&& num) noexcept {
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        key[i] = num.key[i];  // 直接赋值
    }
    return *this;
}

inline BigKey::operator bool() const {
    // 转换为bool类型
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        if (key[i]) {
            return true;  // 有1
        }
    }
    return false;  // 全为0
}

inline BigKey operator&=(BigKey& num1, const BigKey& num2) {
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        num1.key[i] &= num2.key[i];  // 按位与
    }
    return num1;
}

inline BigKey operator|=(BigKey& num1, const BigKey& num2) {
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        num1.key[i] |= num2.key[i];  // 按位或
    }
    return num1;
}

inline const BigKey operator&(const BigKey& num1,
                              const BigKey& num2)  // 按位与重载
{
    BigKey temp;
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        temp.key[i] = num1.key[i] & num2.key[i];  // 按位与
    }
    return temp;
}

inline const BigKey operator|(const BigKey& num1,
                              const BigKey& num2)  // 按位或重载
{
    BigKey temp(num1);
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        temp.key[i] = num1.key[i] | num2.key[i];  // 按位或
    }
    return temp;
}

inline const BigKey operator~(BigKey& num1) {
    // 取反重载
    BigKey temp(num1);
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        temp.key[i] = ~num1.key[i];  // 取反
    }
    return temp;
}

inline void BigKey::setBit(size_t pos, bool value) {
    if (pos > MAX_BUTTONS) {
        throw std::out_of_range("Position out of range");
    }
    // 设置按键值
    if (value) {
        key[pos / (sizeof(BIGKEY_TYPE_INTERNAL) * 8)] |= ((BIGKEY_TYPE_INTERNAL)1 << (pos % (sizeof(BIGKEY_TYPE_INTERNAL) * 8)));  // 设置为1
    } else {
        key[pos / (sizeof(BIGKEY_TYPE_INTERNAL) * 8)] &= ~((BIGKEY_TYPE_INTERNAL)1 << (pos % (sizeof(BIGKEY_TYPE_INTERNAL) * 8)));  // 设置为0
    }
}

inline void BigKey::clear() {
    // 清空按键值
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        key[i] = 0;  // 初始化为0
    }
}

inline bool operator&&(const BigKey& num1, const BigKey& num2) {
    // 逻辑与重载
    bool flag1 = false, flag2 = false;
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        if (num1.key[i]) {
            flag1 = true;  // num1有1
        }
        if (num2.key[i]) {
            flag2 = true;  // num2有1
        }
    }
    return flag1 && flag2;
}

inline bool operator&&(const BigKey& num1, const bool& num2) {
    // 逻辑与重载
    if (num2 == false) {
        return false;  // 如果num2为false，直接返回false
    }
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        if (num1.key[i]) {
            return true;  // num1有1
        }
    }
    return false;
}

inline bool operator&&(const bool& num1, const BigKey& num2) {
    return num2 && num1;  // 调用上面的重载函数
}

inline string BigKey::toString() const {
    // 转换为字符串
    string str = "";
    // 显示为2进制，前面要补0
    for (int i = BIGKEY_NUMBER - 1; i >= 0; i--) {
        string temp = std::bitset<sizeof(BIGKEY_TYPE_INTERNAL) * 8>(key[i]).to_string();  // 转换为2进制
        str += temp;                                                                      // 拼接字符串
    }
    return str;
}

inline BigKey::BigKey(const string& num)  // 用字符串初始化
{
    if (num.size() > MAX_BUTTONS) {
        std::string errMsg = "String size exceeds maximum buttons " + std::to_string(num.size());
        throw std::out_of_range(errMsg);
    }
    this->clear();  // 清空按键值
    for (size_t i = 0; i < num.size(); i++) {
        if (num[i] != '0') {
            this->setBit(num.size() - i - 1, true);  // 设置按键值
        }
    }
}

inline ostream& operator<<(ostream& out, const BigKey& num)  // 输出重载
{
    out << num.toString();  // 打印时没有双引号
    return out;
}

#if ENABLE_QT
inline QDebug operator<<(QDebug dbg, const BigKey& num)  // 输出重载 （注意，不是返回引用）
{
    QString str = num.toString().data();  // 转换为QString
    dbg << qPrintable(str);               // 打印时没有双引号
    return dbg;
}
#endif

inline istream& operator>>(istream& in, BigKey& num)  // 输入重载
{
    string str;
    in >> str;
    num = BigKey(str);
    return in;
}

inline bool operator==(const BigKey& num1, const BigKey& num2)  // 等于重载
{
    for (int i = 0; i < BIGKEY_NUMBER; i++) {
        if (num1.key[i] != num2.key[i]) {
            return false;  // 不相等
        }
    }
    return true;  // 相等
}

inline bool operator!=(const BigKey& num1, const BigKey& num2)  // 不等于重载
{
    return !(num1 == num2);  // 调用等于重载函数
}
