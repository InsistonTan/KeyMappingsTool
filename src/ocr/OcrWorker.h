#pragma once

#include <QObject>

class OcrWorker : public QObject
{
    Q_OBJECT

public:
    explicit OcrWorker(int x, int y, int width, int height, QObject *parent = nullptr);
    void updateParams(int x, int y, int width, int height);

public slots:
    void start();
    void stop();

signals:
    void finished();
    void ocrResult(QString text);
    void ocrError();

private:
    bool running = false;

    // ocr识别区域
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};
