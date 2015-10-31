#ifndef LAUNCHERSETTINGS_H
#define LAUNCHERSETTINGS_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>

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

    void on_genApiKey_button_clicked();

    void genApiKey(QNetworkReply * reply);

private:
    Ui::launcherSettings *ui;

    Settings settings;

    QNetworkAccessManager *mgr;
};

#endif // LAUNCHERSETTINGS_H
