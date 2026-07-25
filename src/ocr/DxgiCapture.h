#pragma once

#include "ImageProcessor.h"
#include <QString>
#include <d3d11.h>
#include <dxgi1_2.h>

//
// dxgi屏幕捕获
//
class DxgiCapture
{
public:
    // 截图结果枚举
    enum class RES{
        Succeeded, // 成功
        NoChanged, // 画面没变化
        Failed, // 失败
    };

public:
    DxgiCapture();

    ~DxgiCapture();

    // 初始化
    bool initialize(int outputIndex = 0);

    // 根据ocr识别区域的x,y坐标找到 outputIndex
    int findDxgiOutputIndex(int x, int y);

    // 截图指定区域, 并预处理图像, 生成模型输入数据
    RES captureRegionAndPreprocess(RECT roi, std::vector<float>& output, int& outputWidth, ImageProcessor& imageProcessor);

    // 截图指定区域, 并保存为png图片
    bool captureRegionAndSavePNG(RECT roi, const wchar_t* savePNGFileName);


private:
    // 截图指定区域
    bool captureRegion(RECT roi, ImageView& image);

    // 创建 CPU 可读 Texture
    bool createStagingTexture(int width, int height);

    void releaseFrame();
    void unmappedData();

private:
    // d3d11设备
    ID3D11Device* device = nullptr;
    // d3d11设备上下文
    ID3D11DeviceContext* context = nullptr;

    // dxgi屏幕信息
    DXGI_OUTPUT_DESC outputDesc;

    // 输出的 duplication
    IDXGIOutputDuplication* duplication = nullptr;

    // CPU 可读 Texture
    ID3D11Texture2D* stagingTexture = nullptr;

    int screenWidth = 0;
    int screenHeight = 0;

    // 是否已经初始化
    bool isInited = false;

    // 上一次初始化的stagingTexture的宽高
    int lastWidth = -1;
    int lastHeight = -1;

    // 上一次初始化使用的 outputIndex
    int lastOutputIndex = 0;
};
