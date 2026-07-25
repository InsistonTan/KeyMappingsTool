#include "OcrEngine.h"
#include "common/Global.h"
#include "common/StringConstants.h"
#include "services/LogService.h"

#include <QFile>
#include <qdebug.h>

OcrEngine::OcrEngine():
    env(
        ORT_LOGGING_LEVEL_WARNING,
        "OCR"
    ){}


bool OcrEngine::loadModel(const QString& modelPath, const QString& dictPath){
    // 创建 Session 配置
    Ort::SessionOptions options;

    // 开启模型图优化(最高级别优化)
    options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    //创建 ONNX Runtime 推理会话, 加载模型到内存
    session = std::make_unique<Ort::Session>(
                env,
                modelPath.toStdWString().c_str(),
                options
            );

    // 加载模型输出的类别字典
    QFile file(dictPath);

    if(!file.open(QIODevice::ReadOnly)){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::modelDictFailed);
        return false;
    }

    dict.clear();

    // dict[0]为 blank
    dict.push_back("");

    while(!file.atEnd()){
        QString ch = QString::fromUtf8(file.readLine());

        ch.remove('\n');
        ch.remove('\r');

        dict.push_back(ch);
    }

    return true;
}

bool OcrEngine::recognize(std::vector<float>& input, const int& inputImageHeight, const int& inputImageWidth, QString& output){
    if(input.empty() || inputImageHeight == 0 || inputImageWidth == 0){
        LogService::parseErrorLog(StringConstants::ocrInvalidParams);
        return false;
    }

    // 模型输入的shape
    std::array<int64_t,4> shape = {
        1,                  // 一张图片
        3,                  // RGB三个通道
        inputImageHeight,   // 高度
        inputImageWidth     // 宽度
    };

    //创建 CPU 内存描述
    Ort::MemoryInfo memory = Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator, // 使用 ONNX Runtime 内存池
        OrtMemTypeDefault  // 默认 CPU 内存
    );

    // 创建 ONNX Tensor
    auto tensor = Ort::Value::CreateTensor<float>(
        memory,
        input.data(),
        input.size(),
        shape.data(),
        shape.size()
    );

    // 获取输入名字
    auto inputName = session->GetInputNameAllocated(
        0,
        Ort::AllocatorWithDefaultOptions()
    );

    // 获取输出名字
    auto outputName =session->GetOutputNameAllocated(
        0,
        Ort::AllocatorWithDefaultOptions()
    );

    // 输入输出名称数组
    const char* inputs[] = { inputName.get() };
    const char* outputs[] = { outputName.get() };

    // 开始推理
    auto result = session->Run(
        Ort::RunOptions{},
        inputs,
        &tensor,
        1,          // 一个输入
        outputs,
        1           // 一个输出
    );

    if(result.empty()){
        LogService::parseErrorLog(StringConstants::ocrResultEmpty);
        return false;
    }

    // 获取输出 Tensor
    auto& outputTensor = result[0];

    // 获取输出的概率数据指针
    float* outputData = outputTensor.GetTensorMutableData<float>();

    // 获取输出 shape
    auto resShape = outputTensor.GetTensorTypeAndShapeInfo().GetShape();

    if(resShape.size() < 3)
    {
        LogService::parseErrorLog(StringConstants::ocrResultShapeInvlid);
        return false;
    }

    // 获取时间长度,例如长度为20,说明有20小段的扫描,每一个time代表 ocr横向扫描的一小段
    int timeLength = resShape[1];

    // 获取输出的类别数量
    int classes = resShape[2];

    // CTC解码, 得到识别的字符串
    output.clear();
    output.append(decode(outputData, timeLength, classes));

    return true;
}

QString OcrEngine::decode(float* data, int time, int classes){
    QString result;
    int last=-1;

    // 遍历时间轴, 每一个 t 对应 OCR 横向扫描的一小段
    for(int t=0;t<time;t++){
        // 保存当前时间点概率最大的类别编号
        int best=0;

        // 保存最高概率
        float max=-999;

        // 遍历每一个class, 找出概率最高的那个class
        for(int c=0;c<classes;c++){
            float v = data[t*classes+c];

            if(v>max){
                max=v;
                best=c;
            }
        }

        // 类别编号有效(==0代表blank), 并且跟上一个不重复
        if(best!=0 && best!=last && best < dict.size()){
            result += dict[best];
        }

        last=best;
    }


    return result;
}
