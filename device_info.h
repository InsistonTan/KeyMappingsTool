#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <string>


class DeviceInfo{
private:
    std::string device_name;
    std::string device_path;
    short vid;
    short pid;
public:
    DeviceInfo(){}
    DeviceInfo(std::string device_name,std::string device_path,short vid,short pid){
        this->device_name = device_name;
        this->device_path = device_path;
        this->vid = vid;
        this->pid = pid;
    }

    std::string getDeviceName(){
        return device_name;
    }

    std::string getDevicePath(){
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
