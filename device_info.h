#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <string>

using namespace std;

class DeviceInfo{
private:
    string device_name;
    string device_path;
    short vid;
    short pid;
public:
    DeviceInfo(){}
    DeviceInfo(string device_name,string device_path,short vid,short pid){
        this->device_name = device_name;
        this->device_path = device_path;
        this->vid = vid;
        this->pid = pid;
    }

    string getDeviceName(){
        return device_name;
    }

    string getDevicePath(){
        return device_path;
    }

    short getVid(){
        return vid;
    }

    short getPid(){
        return pid;
    }
};

#endif // DEVICE_INFO_H
