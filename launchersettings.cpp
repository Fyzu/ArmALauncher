#include "launchersettings.h"
#include "ui_launchersettings.h"

launcherSettings::launcherSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::launcherSettings)
{
    ui->setupUi(this);
}

launcherSettings::~launcherSettings()
{
    delete ui;
}

void launcherSettings::on_buttonBox_accepted() {

    settings.style = ui->styleBox->currentIndex();
    settings.documentMode = ui->documentMode->isChecked();
    settings.launch = ui->launchBox->currentIndex();

    emit sendData(settings);
}

void launcherSettings::reciveData(Settings launcherS) {
    settings = launcherS;

    ui->styleBox->setCurrentIndex(settings.style);
    ui->documentMode->setChecked(settings.documentMode);
    ui->launchBox->setCurrentIndex(settings.launch);

    this->open();
}
