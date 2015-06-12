#include "launcherupdate.h"
#include "ui_launcherupdate.h"

launcherUpdate::launcherUpdate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::launcherUpdate) {
    qDebug() << "launcherUpdate::launcherUpdate: constructor";
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() ^ Qt::WindowContextHelpButtonHint ^ Qt::WindowCloseButtonHint);
}

launcherUpdate::~launcherUpdate() {
    delete ui;
}

// Новая версия
void launcherUpdate::newVersion(Settings settings, QString version) {
    qDebug() << "launcherUpdate::newVersion: start select";

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

// Конец загрузки патч нотов
void launcherUpdate::downloadPatchnotesFinished(QNetworkReply *reply) {
    if(reply->error()) {
        qDebug() << "launcherUpdate::downloadPatchnotesFinished: reply error" << reply->errorString();
        ui->textBrowser->setText(reply->errorString());
    } else {
        qDebug() << "launcherUpdate::downloadPatchnotesFinished: download succ";
        ui->textBrowser->setText(reply->readAll());
    }
    this->show();
}

// Обновится сейчас
void launcherUpdate::on_updateNow_clicked() {
    qDebug() << "launcherUpdate::on_updateNow_clicked: update now";

    emit result(0);
    this->close();
}

// Обновится позже
void launcherUpdate::on_updateAfter_clicked() {
    qDebug() << "launcherUpdate::on_updateAfter_clicked: update after";

    emit result(1);
    this->close();
}

// Не обновлятся
void launcherUpdate::on_updateLater_clicked() {
    qDebug() << "launcherUpdate::on_updateLater_clicked: update later";

    this->close();
}
