#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <csignal>

// 系统错误信号处理
void signalHandler(int signal) {
    const char* errorMsg = nullptr;
    switch (signal) {
    case SIGSEGV: errorMsg = "Segmentation Fault (内存访问错误)\n\n例如空指针异常, 数组下标越界等"; break;
    case SIGFPE:  errorMsg = "Floating Point Exception (算术错误)"; break;
    case SIGILL:  errorMsg = "Illegal Instruction (非法指令)"; break;
    case SIGABRT: errorMsg = "Abort Signal (程序中止)"; break;
    default:      errorMsg = "Unknown Signal (未知错误信号)"; break;
    }

    QMessageBox::critical(nullptr, "程序错误", errorMsg);
    std::exit(EXIT_FAILURE);  // 安全退出
}

// 全局异常处理函数
void handleUncaughtException()
{
    try {
        throw; // 重新抛出当前异常
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "程序异常", QString("程序出现未处理异常:\n%1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(nullptr, "程序异常", "程序出现未知类型的未处理异常");
    }

    QCoreApplication::exit(-1);
}

int main(int argc, char *argv[])
{
    // 设置全局异常处理器
    std::set_terminate(handleUncaughtException);

    // 注册错误信号处理函数
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGFPE,  signalHandler);
    std::signal(SIGILL,  signalHandler);
    std::signal(SIGABRT, signalHandler);

     QApplication a(argc, argv);

    // 捕获异常, 防止程序闪退
    try{
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

    }catch(const std::exception& e){
        QMessageBox::critical(nullptr, "程序异常", QString("程序出现异常:\n%1").arg(e.what()));
    }catch (...) {
        QMessageBox::critical(nullptr, "初始化异常", "程序初始化时发生未知异常");
    }

    return -1;
}


