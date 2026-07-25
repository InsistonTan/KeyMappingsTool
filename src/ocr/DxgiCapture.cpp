#include "DxgiCapture.h"
#include "ImageProcessor.h"
#include "common/Global.h"
#include "common/StringConstants.h"
#include "services/LogService.h"

#include <wrl.h>

using Microsoft::WRL::ComPtr;

DxgiCapture::DxgiCapture()
{}

DxgiCapture::~DxgiCapture(){
    if(stagingTexture){
        stagingTexture->Release();
        stagingTexture = nullptr;
    }

    if(duplication){
        duplication->Release();
        duplication = nullptr;
    }

    if(context){
        context->Release();
        context = nullptr;
    }

    if(device){
        device->Release();
        device = nullptr;
    }
}

bool DxgiCapture::initialize(int outputIndex){
    // 防止重复初始化
    if(isInited)
        return true;

    D3D_FEATURE_LEVEL level;

    // 创建d3d11设备
    HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &device,
            &level,
            &context
        );


    if(FAILED(hr)){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::d3d11CreateFailed);
        return false;
    }


    IDXGIDevice* dxgiDevice=nullptr;
    device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));

    IDXGIAdapter* adapter=nullptr;
    dxgiDevice->GetAdapter(&adapter);

    IDXGIOutput* output=nullptr;
    adapter->EnumOutputs(outputIndex, &output);

    output->GetDesc(&outputDesc);

    screenWidth = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    screenHeight = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;


    IDXGIOutput1* output1=nullptr;
    output->QueryInterface(IID_PPV_ARGS(&output1));

    // 创建duplication
    hr = output1->DuplicateOutput(device, &duplication);

    // 释放com引用计数, 不影响后续使用duplication获取屏幕画面
    output1->Release();
    output->Release();
    adapter->Release();
    dxgiDevice->Release();

    auto res = SUCCEEDED(hr);

    if(res == true){
        isInited = true;
        lastOutputIndex = outputIndex;
    }else{
        Global::showErrorMsgBoxAndPushToLog(StringConstants::duplicationFailed);
    }

    return res;
}

int DxgiCapture::findDxgiOutputIndex(int x, int y){
    ComPtr<IDXGIFactory1> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    if(FAILED(hr))
        return -1;

    for(UINT adapterIndex = 0;; adapterIndex++){
        ComPtr<IDXGIAdapter1> adapter;

        if(factory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND){
            break;
        }

        for(UINT outputIndex = 0;; outputIndex++){
            ComPtr<IDXGIOutput> output;

            if(adapter->EnumOutputs(outputIndex, &output) == DXGI_ERROR_NOT_FOUND){
                break;
            }

            DXGI_OUTPUT_DESC desc{};
            output->GetDesc(&desc);

            RECT rect = desc.DesktopCoordinates;

            // xy坐标点在该显示器范围
            if(x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom){
                return outputIndex;
            }
        }
    }

    return -1;
}

bool DxgiCapture::createStagingTexture(int width, int height){
    // 如果区域宽高没变, 不需要重复创建
    if(lastWidth == width && lastHeight == height)
        return true;

    // 释放旧的
    if(stagingTexture){
        stagingTexture->Release();
        stagingTexture=nullptr;
    }

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    HRESULT hr = device->CreateTexture2D(
            &desc,
            nullptr,
            &stagingTexture
        );

    auto res = SUCCEEDED(hr);

    // 创建成功, 记录本次宽高
    if(res == true){
        lastWidth = width;
        lastHeight = height;
    }else{
        LogService::parseErrorLog(StringConstants::stagingTextureFailed.arg(QString::number(hr)));
    }

    return res;
}

void DxgiCapture::releaseFrame()
{
    if(duplication){
        duplication->ReleaseFrame();
    }
}

void DxgiCapture::unmappedData()
{
    if(context && stagingTexture){
        context->Unmap(stagingTexture, 0);
    }
}

DxgiCapture::RES DxgiCapture::captureRegionAndPreprocess(RECT roi, std::vector<float>& output, int& outputWidth, ImageProcessor& imageProcessor){
    ImageView image;

    // ocr识别区域的坐标转换
    // 因为 ocr识别区域 的坐标是虚拟桌面坐标, 副屏在虚拟桌面坐标里的原点不是(0,0)
    // 而dxgi捕获的屏幕画面坐标是(0,0)为原点的, 所以坐标需要减去当前屏幕的原点
    roi.left -= outputDesc.DesktopCoordinates.left;
    roi.right -= outputDesc.DesktopCoordinates.left;
    roi.top -= outputDesc.DesktopCoordinates.top;
    roi.bottom -= outputDesc.DesktopCoordinates.top;

    auto result = captureRegion(roi, image);

    // 截图失败
    if(result == false){
        return DxgiCapture::RES::Failed;
    }

    // 图像为空, 说明画面没有发生改变
    if(image.empty()){
        return DxgiCapture::RES::NoChanged;
    }

    // 图像预处理
    imageProcessor.preprocess(image, output, outputWidth);

    return DxgiCapture::RES::Succeeded;
}

bool DxgiCapture::captureRegionAndSavePNG(RECT roi, const wchar_t *savePNGFileName)
{
    ImageView image;
    captureRegion(roi, image);

    if(image.empty()){
        return false;
    }

    // 保存为PNG图片
    ImageProcessor::savePNG(savePNGFileName, image);

    return true;
}

bool DxgiCapture::captureRegion(RECT roi, ImageView &image)
{
    // 初始化失败
    if(isInited == false && initialize() == false){
        return false;
    }

    DXGI_OUTDUPL_FRAME_INFO frameInfo{};

    IDXGIResource* resource=nullptr;

    if(duplication == nullptr){
        LogService::parseErrorLog("duplication == nullptr");
        return false;
    }

    // 获取下一帧画面
    HRESULT hr = duplication->AcquireNextFrame(16, &frameInfo, &resource);

    if(hr == DXGI_ERROR_WAIT_TIMEOUT){
        // 没有新画面，不算失败
        return true;
    }
    else if(hr == DXGI_ERROR_ACCESS_LOST){
        // dxgi状态改变, 需要重新初始化 duplication
        LogService::parseErrorLog("DXGI_ERROR_ACCESS_LOST, reinitialize");
        duplication->Release();
        duplication = nullptr;
        if(initialize(lastOutputIndex) == false){
            return false;
        }
    }
    else if(FAILED(hr)){
        LogService::parseErrorLog(StringConstants::getNextFrameFailed.arg(QString::number(hr)));
        return false;
    }

    ID3D11Texture2D* desktopTexture=nullptr;
    resource->QueryInterface(IID_PPV_ARGS(&desktopTexture));

    int width = roi.right-roi.left;
    int height = roi.bottom-roi.top;

    // 创建cpu可读的texture
    if(createStagingTexture(width, height) == false){
        releaseFrame();
        return false;
    }

    // GPU裁剪
    D3D11_BOX box{};
    box.left = roi.left;
    box.top = roi.top;
    box.right = roi.right;
    box.bottom = roi.bottom;
    box.front = 0;
    box.back = 1;

    if(context == nullptr || stagingTexture == nullptr){
        LogService::parseErrorLog(context == nullptr ? "context == nullptr" : "stagingTexture == nullptr");
        releaseFrame();
        return false;
    }
    context->CopySubresourceRegion(
        stagingTexture,
        0,
        0,
        0,
        0,
        desktopTexture,
        0,
        &box
        );

    D3D11_MAPPED_SUBRESOURCE mapped{};

    if(context == nullptr || stagingTexture == nullptr){
        LogService::parseErrorLog(context == nullptr ? "context == nullptr" : "stagingTexture == nullptr");
        releaseFrame();
        return false;
    }
    // CPU读取
    hr = context->Map(
        stagingTexture,
        0,
        D3D11_MAP_READ,
        0,
        &mapped
        );

    if(SUCCEEDED(hr))
    {
        image.width = width;
        image.height = height;
        image.stride = mapped.RowPitch;
        image.data = static_cast<const uint8_t*>(mapped.pData);
        image.format = ImageView::Format::BGRA;
        // 分配内存
        image.buffer.resize(image.stride * image.height);
        // 一行一行复制图像数据
        for(int y = 0; y < image.height; y++){
            memcpy(
                image.buffer.data() + y * image.stride,
                static_cast<uint8_t*>(mapped.pData) + y * mapped.RowPitch,
                image.stride
            );
        }
        // 指向image自己的内存
        image.data = image.buffer.data();

        // 必须在处理完图像后释放mapped数据
        unmappedData();
    }else{
        LogService::parseErrorLog(StringConstants::mappedFailed);
    }

    desktopTexture->Release();
    resource->Release();
    releaseFrame();

    return SUCCEEDED(hr);
}




