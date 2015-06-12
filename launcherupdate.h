#ifndef LAUNCHERUPDATE_H
#define LAUNCHERUPDATE_H

#include <QDialog>

// httpDownload
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "xmlparser.h"

namespace Ui {
    class launcherUpdate;
}

class launcherUpdate : public QDialog
{
    Q_OBJECT

public:
    explicit launcherUpdate(QWidget *parent = 0);
    ~launcherUpdate();
signals:
    void result(int res);
private slots:
    void newVersion(Settings settings, QString version);
    void downloadPatchnotesFinished(QNetworkReply *reply);
    void on_updateNow_clicked();
    void on_updateAfter_clicked();
    void on_updateLater_clicked();
private:
    Ui::launcherUpdate *ui;
    QNetworkAccessManager *manager;
};

#endif // LAUNCHERUPDATE_H
