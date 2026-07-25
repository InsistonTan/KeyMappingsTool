#include "OcrWorker.h"
#include "DxgiCapture.h"
#include "OcrEngine.h"
#include "common/StringConstants.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <qdebug.h>
#include <thread>

#include <common/Global.h>

OcrWorker::OcrWorker(int x, int y, int width, int height, QObject *parent) : QObject{parent}
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
}

void OcrWorker::updateParams(int x, int y, int width, int height)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
}

void OcrWorker::start()
{
    running = true;

    // 预处理后的图片数据
    std::vector<float> inputData;
    // 固定高度
    int imageHeight = 48;
    // 宽度随比例缩放
    int imageWidth = 0;

    // 图像处理器
    ImageProcessor imageProcessor(imageHeight);

    // dxgi屏幕捕获
    DxgiCapture dxgiCapture;
    // 根据屏幕名称获取dxgi的outputIndex
    int outputIndex = dxgiCapture.findDxgiOutputIndex(x, y);

    // 获取outputIndex失败
    if(outputIndex < 0){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::findScreenFailed.arg(QString::number(x), QString::number(y)));
        running = false;
        emit ocrError();
    }

    // 初始化失败
    if(running && dxgiCapture.initialize(outputIndex) == false){
        running = false;
        emit ocrError();
    }

    // ocr引擎
    OcrEngine ocr;
    // 加载模型和模型输出的类别字典
    bool ok = ocr.loadModel(
        QCoreApplication::applicationDirPath() + "/models/RapidOCR/latin_PP-OCRv5_rec_mobile.onnx",
        QCoreApplication::applicationDirPath() + "/models/RapidOCR/latin_dict.txt"
        );
    // 加载模型失败
    if(!ok)
    {
        running = false;
        emit ocrError();
    }


    // 设置系统计时器精度
    Global::setSystemTimePeriod_1ms();

    // 高精度时钟, 用于固定while循环的执行频率
    using clock = std::chrono::high_resolution_clock;
    // 本轮执行的开始时间
    auto currentExcuteTime = clock::now();
    // 执行频率 hz
    auto excuteFrequency = 60;
    // 每轮执行的最大时间 毫秒
    auto each_mstime = 1000 / excuteFrequency;

    // 模型推理结果
    QString ocrOutput;
    // 车速字符串
    QString speedNumber;
    // 匹配车速的正则
    QRegularExpression re(R"((\d+))");
    QRegularExpressionMatch match;

    // 截图失败计数器, 连续失败超过指定次数将结束运行
    int failedCounterDxgi = 0;
    // ocr失败计数器
    int failedCounterOcr = 0;
    // 最大连续失败次数
    int maxFailedCount = 30;

    while (running){
        auto now = clock::now();
        // 如果本轮执行的开始时间 已经滞后, 重置本轮执行的开始时间
        if(currentExcuteTime < now){
            currentExcuteTime = now;
        }

        // 清空
        inputData.clear();

        // DXGI截图 并 预处理
        auto dxgiRes = dxgiCapture.captureRegionAndPreprocess({x,y,x+width,y+height}, inputData, imageWidth, imageProcessor);

        // 截图成功
        if(dxgiRes == DxgiCapture::RES::Succeeded){
            failedCounterDxgi = 0;

            // ONNX推理
            bool ocrOk = ocr.recognize(inputData, imageHeight, imageWidth, ocrOutput);

            // ocr推理成功
            if(ocrOk){
                failedCounterOcr = 0;

                // 提取出车速数字
                match = re.match(ocrOutput);
                if (match.hasMatch())
                {
                    speedNumber = match.captured(1);
                    // 车速字符串不为空
                    if(speedNumber.isEmpty() == false){
                        // 发送信号
                        emit ocrResult(speedNumber);
                    }
                }
            }else{
                failedCounterOcr++;
            }
        }
        // 截图失败
        else if(dxgiRes == DxgiCapture::RES::Failed){
            failedCounterDxgi++;
        }

        // 截图连续失败次数达到上限, 结束循环
        if(failedCounterDxgi >= maxFailedCount){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::dxgiFailedMax);
            running = false;
            emit ocrError();
        }
        // ocr连续失败次数达到上限, 结束循环
        if(failedCounterOcr >= maxFailedCount){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::ocrFailedMax);
            running = false;
            emit ocrError();
        }

        // 处理事件队列
        QCoreApplication::processEvents();

        // 本轮执行的结束时间 = 开始时间 + 固定的每轮执行时间
        currentExcuteTime += std::chrono::milliseconds(each_mstime);

        // 休眠至设置的 本次执行的结束时间
        std::this_thread::sleep_until(currentExcuteTime);
    }

    // 恢复默认系统计时器精度
    Global::restoreSystemTimePeriod();

    emit finished();
}

void OcrWorker::stop()
{
    running = false;
}
