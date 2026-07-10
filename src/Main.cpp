#include "common/Global.h"
#include "common/StringConstants.h"
#include "services/MessageBoxService.h"
#include "ui/mainwindow/MainWindow.h"
#include "services/ConfigService.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QMenu>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <csignal>
#include <QOperatingSystemVersion>

//错误信号处理函数
void signalHandler(int signal);
// 全局异常处理函数
void handleUncaughtException();
// 创建系统托盘图标
void createTrayIcon(QApplication* app, MainWindow* mainWindow);
// 显示主窗口, 修复闪白
void showMainWIndow(MainWindow* w);



int main(int argc, char *argv[])
{
    // 设置全局异常处理器
    std::set_terminate(handleUncaughtException);

    // 注册错误信号处理函数
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGFPE,  signalHandler);
    std::signal(SIGILL,  signalHandler);
    std::signal(SIGABRT, signalHandler);

    // 设置支持非整数倍dpi缩放（如150%）
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication a(argc, argv);
    // 隐藏窗口
    Global::hideWindow = new Global::HiddenHostWindow();


    // 设置全局字体
    QFont font;
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows11){
        font.setFamilies({
            "Segoe UI"
        });
    }
    else{
        font.setFamilies({
            "Segoe UI",
            "Microsoft YaHei UI"
        });
    }
    font.setPointSizeF(9.0);
    font.setWeight(QFont::Normal);
    qApp->setFont(font);

    qApp->setStyleSheet(QString("QWidget{color:%1;}").arg(Theme::textColor()));

    try{
        // 解析命令行参数
        QCommandLineParser parser;
        parser.setApplicationDescription(Global::APP_NAME);
        parser.addHelpOption();
        parser.addOption(QCommandLineOption("hide", "Start the application background"));
        parser.process(a);

        // 初始化用户配置服务
        ConfigService::init();

        // 创建主窗口
        MainWindow w;

        //Global::g_mainWindow = &w;

        // 创建系统托盘图标
        createTrayIcon(&a, &w);

        // 运行参数没有带 --hide
        if(parser.isSet("hide") == false){
            showMainWIndow(&w);
        }

        return a.exec();

    }catch(const std::exception& e){
        QMessageBox::critical(nullptr, StringConstants::appException, QString("Init app failed:\n%1").arg(e.what()));
    }catch (...) {
        QMessageBox::critical(nullptr, StringConstants::appException, "Init app failed: Unknown exception.");
    }

    return -1;
}

void showMainWIndow(MainWindow* w){
    // 为了修复, 窗口显示闪一下白色的问题
    // 先设置完全透明
    w->setWindowOpacity(0);
    // 显示主窗口
    w->show();
    w->activateWindow();
    QTimer::singleShot(50, w, [w](){
        // 再恢复不透明
        w->setWindowOpacity(1.0);
    });
}


void createTrayIcon(QApplication* app, MainWindow* mainWindow){
    // 创建系统托盘图标
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(app);
    // 设置图标
    trayIcon->setIcon(QIcon(":/app.ico"));
    // 设置提示文本
    trayIcon->setToolTip(Global::APP_NAME);

    // 创建托盘右键菜单
    QMenu *trayMenu = new QMenu();
    Theme::setTrayMemuStyleSheet(trayMenu);

    // 创建菜单项
    QAction *showAction = new QAction(StringConstants::showMainWindow, trayMenu);
    QAction *quitAction = new QAction(StringConstants::quit, trayMenu);

    // 菜单项添加到菜单
    trayMenu->addAction(showAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    // 将菜单设给托盘图标
    trayIcon->setContextMenu(trayMenu);

    // 显示托盘图标
    trayIcon->show();

    // 点击"显示主窗口"：显示主窗口
    QObject::connect(showAction, &QAction::triggered, mainWindow, [mainWindow](){
        showMainWIndow(mainWindow);
    });

    // 点击"退出"：退出应用程序
    QObject::connect(quitAction, &QAction::triggered, app, &QApplication::quit);

    // 鼠标左键点击托盘图标, 显示/隐藏主窗口
    QObject::connect(trayIcon, &QSystemTrayIcon::activated, [mainWindow](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            if (mainWindow->isHidden()) {
                showMainWIndow(mainWindow);
            } else {
                mainWindow->hide();
            }
        }
    });
}


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

    MessageBoxService::showError(StringConstants::appError, errorMsg);
    std::exit(EXIT_FAILURE);  // 安全退出
}


// 全局异常处理函数
void handleUncaughtException()
{
    try {
        throw; // 重新抛出当前异常
    } catch (const std::exception& e) {
        MessageBoxService::showError(StringConstants::appException,  StringConstants::appUnhandedException.arg(e.what()));
    } catch (...) {
        MessageBoxService::showError(StringConstants::appException,  StringConstants::appUnhandedUnknownException);
    }

    QCoreApplication::exit(-1);
}
