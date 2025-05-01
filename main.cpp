#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();

    // 解析命令行参数
    QCommandLineParser parser;
    parser.setApplicationDescription("KeyMappingsTool");
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("hide", "Start the application background"));
    parser.process(a);

    // 检查运行参数是否带 --hide
    if(parser.isSet("hide")){
        // 最小化窗口
        w.setWindowState(Qt::WindowMinimized);
    }

    return a.exec();
}


