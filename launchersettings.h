#ifndef LAUNCHERSETTINGS_H
#define LAUNCHERSETTINGS_H

#include <QDialog>
#include "repoedit.h"

namespace Ui {
class launcherSettings;
}

class launcherSettings : public QDialog
{
    Q_OBJECT

public:
    explicit launcherSettings(QWidget *parent = 0);
    ~launcherSettings();
signals:

    void sendData(Settings launcherS);
    void checkUpdate();
private slots:
    void on_buttonBox_accepted();

    void reciveData(Settings launcherS);

    void on_launchCheckUpdate_clicked();

private:
    Ui::launcherSettings *ui;

    Settings settings;
};

#endif // LAUNCHERSETTINGS_H
