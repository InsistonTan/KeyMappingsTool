#include "HidHideService.h"
#include "common/StringConstants.h"
#include "services/LogService.h"
#include "services/MessageBoxService.h"

#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QSettings>

#include <ui/widgets/CardMessageDialog.h>

#include <common/Global.h>

#include <Windows.h>
#include <SetupAPI.h>

#pragma comment(lib, "Setupapi.lib")

HidHideService::HidHideService() {}

bool HidHideService::checkHidHideDriver()
{
    QSettings settings(HIDHIDE_REG_PATH, QSettings::NativeFormat);
    if(settings.contains("Version")){
        return true;
    }else{
        //获取当前目录
        QDir dir(QCoreApplication::applicationDirPath());

        // 驱动文件的相对路径
        QString driverPath = dir.filePath("driver/HidHide_1.5.230_x64.exe");

        // 创建消息弹窗
        CardMessageDialog dialog(CardMessageDialog::Type::Warning, StringConstants::info, StringConstants::installHidHideDriverTips.arg(driverPath.data()));

        // 判断用户点击确认按钮
        if (dialog.exec() == QDialog::Accepted) {
            //std::string command = "explorer \"" + driverPath + "\"";  // 使用双引号包裹路径，处理空格
            QString command = "\"" + driverPath + "\"";  // 使用双引号包裹路径，处理空格

            QFile driverFile(driverPath);
            // 安装包不存在
            if(!driverFile.exists()){
                Global::showErrorMsgBoxAndPushToLog(StringConstants::hidHideDriverFileNotExistsTips.arg(driverPath));
                return false;
            }

            // 使用 QProcess 执行命令
            QProcess::startDetached(command);
        }

        return false;
    }
}

void HidHideService::hideMappedDevices(const QVector<DevInfo> &devs)
{
    if(isHiding || devs.isEmpty()){
        return;
    }

    // 重置缓存
    hidHideStateCache = false;
    hidHideBlacklistCache.clear();


    // 记录原本的 hidhide 的状态
    QString output;
    bool res = false;
    res = runHidHideCLI({"--cloak-state"}, &output);
    if(res == false){
        MessageBoxService::showError(StringConstants::runHidHideCLIFailed);
        return;
    }
    if(output.contains("--cloak-off")){
        hidHideStateCache = false;
    }else{
        hidHideStateCache = true;
    }


    // 获取当前软件的exe绝对路径, 添加进白名单
    res = runHidHideCLI({"--app-reg", QCoreApplication::applicationFilePath()});
    if(res == false){
        MessageBoxService::showError(StringConstants::runHidHideCLIFailed);
        return;
    }


    // 添加设备进黑名单, 并缓存操作成功的设备
    for(const auto &dev : devs){
        res = runHidHideCLI({"--dev-hide", dev.deviceInstancePath});
        if(res == false){
            MessageBoxService::showError(StringConstants::runHidHideCLIFailed);
            // 恢复原状
            stop();
            return;
        }else{
            hidHideBlacklistCache.append(dev);
            LogService::pushToLogQueue(StringConstants::addBlacklistSucceed.arg(dev.name));
        }
    }


    // 开启设备隐藏
    res = runHidHideCLI({"--cloak-on"});
    if(res == false){
        MessageBoxService::showError(StringConstants::runHidHideCLIFailed);
        // 恢复原状
        stop();
        return;
    }

    isHiding = true;
    LogService::parseSuccessLog(StringConstants::devHideSucceed);
}

void HidHideService::stop()
{
    if(isHiding == false){
        return;
    }

    // 从白名单中 移除 当前软件
    runHidHideCLI({"--app-unreg", QCoreApplication::applicationFilePath()});

    // 从黑名单中 移除 记录的设备实例ID
    for(const auto &dev : hidHideBlacklistCache){
        runHidHideCLI({"--dev-unhide", dev.deviceInstancePath});
    }

    // 停止设备隐藏
    auto res = runHidHideCLI({"--cloak-off"});

    isHiding = false;

    if(res){
        LogService::pushToLogQueue(StringConstants::devHideStopSucceed);
    }else{
        MessageBoxService::showError(StringConstants::devHideStopFailed);
    }
}

QString HidHideService::getHidHideCLIPath()
{
    QSettings settings(HIDHIDE_REG_PATH, QSettings::NativeFormat);
    if(settings.contains("Path")){
        QString installPath = settings.value("Path").toString();
        QDir dir(installPath);
        if(dir.exists()){
            return installPath + "\\x64\\HidHideCLI.exe";
        }
    }

    return "";
}

bool HidHideService::runHidHideCLI(const QStringList &arguments, QString *output)
{
    const QString cli = QDir::cleanPath(getHidHideCLIPath());

    // cli程序不存在
    if (!QFileInfo::exists(cli)) {
        LogService::parseErrorLog(StringConstants::hidHideCLINotExist + cli);
        return false;
    }

    QProcess process;
    process.setProgram(cli);
    process.setWorkingDirectory(QFileInfo(cli).absolutePath());
    process.setArguments(arguments);
    process.setProcessChannelMode(QProcess::SeparateChannels);
    // 把子进程的标准输入重定向到系统空设备, 避免子进程等待标准输入
    process.setStandardInputFile(QProcess::nullDevice());
    // 隐藏黑窗
    process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) {
            args->flags |= CREATE_NO_WINDOW;
    });
    // 启动进程
    process.start();

    if (!process.waitForStarted(1000)) {
        LogService::parseErrorLog(StringConstants::hidHideCLIStartFailed + process.errorString());
        return false;
    }

    if (!process.waitForFinished(3000)) {
        process.kill();
        process.waitForFinished();
        LogService::parseErrorLog(StringConstants::hidHideCLITimeout);
        return false;
    }

    // 输出的结果
    const QString out = QString::fromLocal8Bit(process.readAllStandardOutput());
    const QString err = QString::fromLocal8Bit(process.readAllStandardError());

    if (output)
        *output = out;

    // 执行失败
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        LogService::parseErrorLog(StringConstants::hidHideCLIExecuteFailed.arg(QString::number(process.exitCode()), out, err));
        return false;
    }

    return true;
}

QString HidHideService::getDevInstancePath(const QString &devInterfacePath)
{
    if (devInterfacePath.isEmpty())
        return {};

    // 打开所有设备类的设备信息集
    HDEVINFO deviceInfoSet = SetupDiGetClassDevsW(nullptr, nullptr, nullptr, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        LogService::parseErrorLog("SetupDiGetClassDevsW failed:" + QString::number(GetLastError()));
        return {};
    }

    QString instanceId;

    // 将已知的设备接口路径打开到设备信息集
    SP_DEVICE_INTERFACE_DATA interfaceData{};
    interfaceData.cbSize = sizeof(interfaceData);

    if (!SetupDiOpenDeviceInterfaceW(deviceInfoSet, reinterpret_cast<LPCWSTR>(devInterfacePath.utf16()), 0, &interfaceData)) {
        LogService::parseErrorLog("SetupDiOpenDeviceInterfaceW failed:" + QString::number(GetLastError()));

        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return {};
    }

    // 获取接口详细信息所需的缓冲区大小
    DWORD requiredSize = 0;
    SetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &interfaceData, nullptr, 0, &requiredSize, nullptr);

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || requiredSize == 0) {
        LogService::parseErrorLog("Failed to get interface detail size:" + QString::number(GetLastError()));

        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return {};
    }

    // 分配接口详细信息缓冲区
    QVector<BYTE> buffer(static_cast<qsizetype>(requiredSize));

    auto *detail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W *>(buffer.data());

    detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

    SP_DEVINFO_DATA deviceInfoData{};
    deviceInfoData.cbSize = sizeof(deviceInfoData);

    if (!SetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &interfaceData, detail, requiredSize, nullptr, &deviceInfoData)) {
        LogService::parseErrorLog("SetupDiGetDeviceInterfaceDetailW failed:" + QString::number(GetLastError()));

        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return {};
    }

    // 获取设备实例 ID
    DWORD charsRequired = 0;
    SetupDiGetDeviceInstanceIdW(deviceInfoSet, &deviceInfoData, nullptr, 0, &charsRequired);

    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && charsRequired > 0) {
        QVector<wchar_t> idBuffer(static_cast<qsizetype>(charsRequired));

        if (SetupDiGetDeviceInstanceIdW(deviceInfoSet, &deviceInfoData, idBuffer.data(), charsRequired, nullptr)) {
            instanceId = QString::fromWCharArray(idBuffer.data());
        } else {
            LogService::parseErrorLog("SetupDiGetDeviceInstanceIdW failed:" + QString::number(GetLastError()));
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);

    return instanceId;
}




