#include "launchersettings.h"
#include "ui_launchersettings.h"

launcherSettings::launcherSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::launcherSettings)
{
    qInfo() << "launcherSettings::launcherSettings: constructor";
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags().operator ^=(Qt::WindowContextHelpButtonHint));
    mgr = new QNetworkAccessManager(this);
    connect(mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(genApiKey(QNetworkReply*)));
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
    settings.tushinoApiKey = ui->tushinoApiKey->text();

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
    ui->tushinoApiKey->setText(settings.tushinoApiKey);
    this->open();
}

// Нажата кнопка принудительного обновления
void launcherSettings::on_launchCheckUpdate_clicked() {
    qInfo() << "launcherSettings::on_launchCheckUpdate_clicked: Check update clicked";

    emit checkUpdate();
}

// Нажата кнопка генерации ID
void launcherSettings::on_genApiKey_button_clicked()
{
    qDebug() << "launcherSettings::on_genApiKey_button_clicked: called";

    mgr->get(QNetworkRequest(QUrl(QString("http://bystolen.ru/tsgames_ex/getid.php"))));
}

void launcherSettings::genApiKey(QNetworkReply * reply) {
    if (reply->error() == QNetworkReply::NoError) {
        ui->tushinoApiKey->setText(QJsonDocument::fromJson(reply->readAll()).object().value("id").toString());
    } else {
        qDebug() << "launcherSettings::on_genApiKey_button_clicked: Failure" <<reply->errorString();
    }
    delete reply;
}
