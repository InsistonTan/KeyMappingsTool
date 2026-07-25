#pragma once

#include <onnxruntime_cxx_api.h>

#include <QString>
#include <vector>


class OcrEngine
{

public:
    OcrEngine();

    // 加载模型;
    // modelPath 模型路径;
    // dictPath 模型输出类别字典路径;
    bool loadModel(const QString& modelPath, const QString& dictPath);

    // 推理
    bool recognize(std::vector<float>& input, const int& inputImageHeight, const int& inputImageWidth, QString& output);

private:
    // 对输出的结果 decode (去重, 去空白, 得到最终的结果)
    QString decode(float* data, int time, int classes);

private:
    Ort::Env env;
    std::unique_ptr<Ort::Session> session;
    std::vector<QString> dict;

};
