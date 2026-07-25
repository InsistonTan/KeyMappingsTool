#pragma once

#include <vector>
#include <cstdint>

// 图像
struct ImageView
{
    int width{};
    int height{};
    int stride{}; // 每行的字节数
    const uint8_t* data{};// 指向buffer首位的指针
    std::vector<uint8_t> buffer;//图像数据

    enum class Format
    {
        BGRA,
        RGBA
    };

    Format format{Format::RGBA};

    bool empty() const noexcept{
        return data == nullptr;
    }
};


// 图像处理器
class ImageProcessor
{
public:
    ImageProcessor(int inputHeight);

    // 加载png图片
    static bool loadPNG(const wchar_t* filename, ImageView& image);
    // 保存为png图片
    static bool savePNG(const wchar_t* filename, const ImageView& image);

    // 图像预处理, 将图片数据转换成模型需要的数据格式
    void preprocess(const ImageView& image, std::vector<float>& output, int& outputWidth);


private:
    // 图像固定高度
    int inputHeight;

    // 调整尺寸, 并归一化处理
    void resizeAndNormalize(const ImageView& src, std::vector<float>& dst,int inputWidth);

};
