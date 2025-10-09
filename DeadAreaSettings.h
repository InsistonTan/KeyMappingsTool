#ifndef DEADAREASETTINGS_H
#define DEADAREASETTINGS_H

#include <QMainWindow>

#define SETTINGS_FILE_NAME "deadarea_settings.json"

namespace Ui {
class DeadAreaSettings;
}

class DeadAreaSettings : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeadAreaSettings(QWidget *parent = nullptr);
    ~DeadAreaSettings();

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_editingFinished();

    void on_pushButton_clicked();

    void on_lineEdit_2_editingFinished();

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_lineEdit_4_textChanged(const QString &arg1);

    void on_lineEdit_3_textChanged(const QString &arg1);

    void on_lineEdit_4_editingFinished();

    void on_lineEdit_3_editingFinished();

    void on_lineEdit_5_textChanged(const QString &arg1);

    void on_lineEdit_5_editingFinished();

private:
    Ui::DeadAreaSettings *ui;

    void unsave();
    void save();

    void saveSettingsToFile();
    void loadSettingsFile();
};

#endif // DEADAREASETTINGS_H
