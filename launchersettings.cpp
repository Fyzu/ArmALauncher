#include "launchersettings.h"
#include "ui_launchersettings.h"

launcherSettings::launcherSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::launcherSettings)
{
    qInfo() << "launcherSettings::launcherSettings: constructor";
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags().operator ^=(Qt::WindowContextHelpButtonHint));

}

launcherSettings::~launcherSettings()
{
    delete ui;
}

// Отправляем данные
void launcherSettings::on_buttonBox_accepted() {
    qInfo() << "launcherSettings::on_buttonBox_accepted: start";

    // Собираем данные для отправки
    settings.style = ui->styleBox->currentIndex();
    settings.documentMode = ui->documentMode->isChecked();
    settings.launch = ui->launchBox->currentIndex();
    settings.updateTime = ui->updateTimeSpin->value();
    settings.state = ui->stateBox->currentIndex();
    settings.checkUpdateAfterStart = ui->checkUpdateAfterStart->isChecked();

    // Отправляем данные
    emit sendData(settings);
}

// Принимаем данные
void launcherSettings::reciveData(Settings launcherS) {
    qInfo() << "launcherSettings::reciveData: start";

    // Применяем изменения
    settings = launcherS;
    ui->styleBox->setCurrentIndex(settings.style);
    ui->documentMode->setChecked(settings.documentMode);
    ui->launchBox->setCurrentIndex(settings.launch);
    ui->updateTimeSpin->setValue(settings.updateTime);
    ui->stateBox->setCurrentIndex(settings.state);
    ui->checkUpdateAfterStart->setChecked(settings.checkUpdateAfterStart);
    this->open();
}

// Нажата кнопка принудительного обновления
void launcherSettings::on_launchCheckUpdate_clicked() {
    qInfo() << "launcherSettings::on_launchCheckUpdate_clicked: Check update clicked";

    emit checkUpdate();
}
