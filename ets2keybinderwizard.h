#ifndef ETS2KEYBINDERWIZARD_H
#define ETS2KEYBINDERWIZARD_H

#include <QWizard>
#include <dinput.h>
#include <string.h>
#include <windows.h>

namespace Ui {
class ETS2KeyBinderWizard;
}

class ETS2KeyBinderWizard : public QWizard {
    Q_OBJECT

public:
    explicit ETS2KeyBinderWizard(QWidget* parent = nullptr);
    ~ETS2KeyBinderWizard();

    QStringList getDeviceNameGameList();

private slots:

    void on_comboBox_activated(int index);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::ETS2KeyBinderWizard* ui;

    std::string deviceName; // 设备名称

    QString selectedProfilePath; // 选择的配置文件路径

    QList<QPair<QString, QDateTime>> steamProfileFolders;
    QList<QPair<QString, QDateTime>> profileFolders; // 所有配置文件列表

    LPDIRECTINPUT8 pDirectInput = NULL;
    LPDIRECTINPUTDEVICE8 pDevice = NULL;
    int lastDeviceIndex = -99;
    DIDEVCAPS capabilities;

    bool hasLastDevInCurrentDeviceList(std::string lastDeviceName);
    bool openDiDevice(int deviceIndex, HWND hWnd);
    bool initDirectInput();

    void updateUserProfile();
    bool backupProfile();
};

#endif // ETS2KEYBINDERWIZARD_H
