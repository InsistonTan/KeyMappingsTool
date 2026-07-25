#include "ImageProcessor.h"

#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#pragma comment(lib, "windowscodecs.lib")

using Microsoft::WRL::ComPtr;


ImageProcessor::ImageProcessor(int inputHeight)
    : inputHeight(inputHeight){}


void ImageProcessor::preprocess(const ImageView& image, std::vector<float>& output, int& outputWidth){
    if(image.empty() || image.width <= 0 || image.height <= 0){
        output.clear();
        outputWidth = 0;
        return;
    }

    // 根据固定的高度按笔计算出宽度
    float ratio = (float)image.width / image.height;
    outputWidth = static_cast<int>(inputHeight * ratio);

    // 防止过小
    if(outputWidth < 10)
        outputWidth = 10;

    // 初始化结果数组
    output.resize(3 * inputHeight * outputWidth);

    // 调整图像大小, 以及归一化处理
    resizeAndNormalize(image, output, outputWidth);
}

void ImageProcessor::resizeAndNormalize(const ImageView& src, std::vector<float>& dst, int inputWidth){

    const float mean[3]{0.485f, 0.456f, 0.406f};
    const float std[3]{0.229f, 0.224f, 0.225f};

    int area = inputHeight * inputWidth;

    for(int y=0;y<inputHeight;y++){

        float fy = (float)y * src.height / inputHeight;
        int sy = (int)fy < src.height - 1 ? (int)fy : src.height - 1;

        for(int x=0;x<inputWidth;x++){

            float fx = (float)x * src.width / inputWidth;
            int sx = (int)fx < src.width - 1 ? (int)fx : src.width - 1;

            const uint8_t* pixel = src.data + sy * src.stride + sx * 4;

            // 默认rgba排列
            float r = pixel[0] / 255.0f;
            float g = pixel[1] / 255.0f;
            float b = pixel[2] / 255.0f;

            // DXGI格式: bgra
            if(src.format == ImageView::Format::BGRA){
                b = pixel[0] / 255.0f;
                g = pixel[1] / 255.0f;
                r = pixel[2] / 255.0f;
            }

            // 转换成模型需要的数据格式: NCHW [1,3,H,W] RGB
            dst[0 * area + y * inputWidth + x] = (r - mean[0]) / std[0];
            dst[1 * area + y * inputWidth + x] = (g - mean[1]) / std[1];
            dst[2 * area + y * inputWidth + x] = (b - mean[2]) / std[2];
        }
    }
}

bool ImageProcessor::loadPNG(const wchar_t *filename, ImageView &image){
    HRESULT hr;
    ComPtr<IWICImagingFactory> factory;

    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
        );


    if(FAILED(hr))
        return false;

    ComPtr<IWICBitmapDecoder> decoder;

    hr = factory->CreateDecoderFromFilename(
        filename,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );

    if(FAILED(hr)){
        //factory->Release();
        return false;
    }

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);

    if(FAILED(hr))
        return false;

    UINT width;
    UINT height;

    frame->GetSize(&width, &height);

    // 转RGBA
    ComPtr<IWICFormatConverter> converter;
    factory->CreateFormatConverter(&converter);
    converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0,
        WICBitmapPaletteTypeCustom
        );

    uint8_t* buffer = new uint8_t[width * height * 4];
    converter->CopyPixels(
        nullptr,
        width * 4,
        width * height * 4,
        buffer
        );

    image.width = width;
    image.height = height;
    image.stride = width * 4;
    image.data = buffer;
    image.format = ImageView::Format::RGBA;

    return true;
}

bool ImageProcessor::savePNG(const wchar_t *filename, const ImageView &image)
{
    if(image.empty())
        return false;

    HRESULT hr;

    // Factory
    ComPtr<IWICImagingFactory> factory;

    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
        );

    if(FAILED(hr))
        return false;

    // Stream
    ComPtr<IWICStream> stream;
    hr = factory->CreateStream(&stream);

    if(FAILED(hr))
        return false;

    hr = stream->InitializeFromFilename(filename, GENERIC_WRITE);

    if(FAILED(hr))
        return false;


    // PNG Encoder
    ComPtr<IWICBitmapEncoder> encoder;
    hr = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);

    if(FAILED(hr))
        return false;

    hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);

    if(FAILED(hr))
        return false;


    // Frame
    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> bag;

    hr = encoder->CreateNewFrame(&frame, &bag);

    if(FAILED(hr))
        return false;

    hr = frame->Initialize(bag.Get());

    if(FAILED(hr))
        return false;


    // Size
    hr = frame->SetSize(image.width, image.height);

    if(FAILED(hr))
        return false;


    // Pixel Format
    WICPixelFormatGUID format;

    if(image.format == ImageView::Format::BGRA){
        format = GUID_WICPixelFormat32bppBGRA;
    }
    else{
        format = GUID_WICPixelFormat32bppRGBA;
    }

    hr = frame->SetPixelFormat(&format);

    if(FAILED(hr))
        return false;


    // Write Pixels
    hr = frame->WritePixels(
        image.height,
        image.stride,
        image.stride * image.height,
        const_cast<BYTE*>(
            reinterpret_cast<const BYTE*>(image.data)
        )
    );

    if(FAILED(hr))
        return false;


    // Commit
    hr = frame->Commit();

    if(FAILED(hr))
        return false;

    hr = encoder->Commit();

    if(FAILED(hr))
        return false;

    return true;
}
