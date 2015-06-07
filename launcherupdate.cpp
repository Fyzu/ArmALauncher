#include "launcherupdate.h"
#include "ui_launcherupdate.h"

launcherUpdate::launcherUpdate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::launcherUpdate)
{
    ui->setupUi(this);
}

launcherUpdate::~launcherUpdate()
{
    delete ui;
}

void launcherUpdate::newVersion(Settings settings, QString version) {

    // Применение стиля
    if(settings.style == 0) {
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/myresources/IMG/refresh57.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->updateNow->setIcon(icon1);
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/myresources/IMG/update8.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->updateAfter->setIcon(icon2);
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/myresources/IMG/power buttons1.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->updateLater->setIcon(icon3);
    } else {
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/myresources/IMG/darkstyle/refresh57.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->updateNow->setIcon(icon1);
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/myresources/IMG/darkstyle/update8.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->updateAfter->setIcon(icon2);
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/myresources/IMG/darkstyle/power buttons1.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->updateLater->setIcon(icon3);
    }

    ui->label->setText("Изменения в новой версии " + version);

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadPatchnotesFinished(QNetworkReply*)));
    manager->get(QNetworkRequest(QUrl("http://launcher.our-army.su/download/updater/patchnotes")));
}

void launcherUpdate::downloadPatchnotesFinished(QNetworkReply *reply) {
    if(reply->error()) {
        ui->textBrowser->setText(reply->errorString());
    } else {
        ui->textBrowser->setText(reply->readAll());
    }
    this->show();
}

void launcherUpdate::on_updateNow_clicked() {

    emit result(0);
    this->close();
}

void launcherUpdate::on_updateAfter_clicked() {

    emit result(1);
    this->close();
}

void launcherUpdate::on_updateLater_clicked() {

    this->close();
}
