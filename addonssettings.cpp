#include "addonssettings.h"
#include "ui_addonssettings.h"

addonsSettings::addonsSettings(QWidget *parent) :
    QDialog(parent), ui(new Ui::addonsSettings) {

    qInfo() << "addonsSettings::addonsSettings: constructor";
    ui->setupUi(this);
    this->setWindowFlags((this->windowFlags()^Qt::WindowContextHelpButtonHint));
}

addonsSettings::~addonsSettings() {

    delete ui;
}

// Получение данных из главного окна
void addonsSettings::receiveData(Settings settings, QStringList listD, QStringList listPriorityAddonsD, QStringList addons) {

    qInfo() << "addonsSettings::receiveData: show";
    // Применение стиля
    if(settings.style == 0) {

        QIcon icon1;
        icon1.addFile(QStringLiteral(":/myresources/IMG/file96.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->addonSearchDirectories_add->setIcon(icon1);
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/myresources/IMG/delete82.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->addonSearchDirectories_del->setIcon(icon2);
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/myresources/IMG/up177.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->addonsPriorities_up->setIcon(icon3);
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/myresources/IMG/down177.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->addonsPriorities_down->setIcon(icon4);
    } else {

        QIcon icon1;
        icon1.addFile(QStringLiteral(":/myresources/IMG/darkstyle/file96.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->addonSearchDirectories_add->setIcon(icon1);
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/myresources/IMG/darkstyle/delete82.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->addonSearchDirectories_del->setIcon(icon2);
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/myresources/IMG/darkstyle/up177.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->addonsPriorities_up->setIcon(icon3);
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/myresources/IMG/darkstyle/down177.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->addonsPriorities_down->setIcon(icon4);
    }

    listDirs = listD;
    listPriorityAddonsDirs = listPriorityAddonsD;

    ui->addonSearchDirectories->clear();
    ui->addonsPriorities->clear();

    //...списка путей для поиска аддонов
    for(int i = 0; i<listDirs.size();i++)
        if(!listDirs[i].isEmpty())
            ui->addonSearchDirectories->addItem(listDirs[i]);

    //...списка аддонов в сорт. по приоритету
    for(int i = 0; i <listPriorityAddonsDirs.size();i++)
        for(int j=0; j<addons.size();j++)
            if(!ListWidgetContains(ui->addonsPriorities, addons[j]) && listPriorityAddonsDirs[i] == addons[j]) {
                ui->addonsPriorities->addItem(listPriorityAddonsDirs[i]);
                break;
            }

    this->open();
}

// Проверка, содержит ли ListWidget строку str
bool addonsSettings::ListWidgetContains(QListWidget * widget, QString str) {

   // Итерируем элементы виджета
   for(int i = 0; i<widget->count();i++)
       // Если находим str, то отправляем правду
       if(widget->item(i)->text() == str)
           return true;

   // Если не находим
   return false;
}

/*
 * Слоты вкладки - настройка аддонов
 */

// Добавление и удаление директорий
void addonsSettings::on_addonSearchDirectories_add_clicked() {

    qInfo() << "addonsSettings::on_addonSearchDirectories_add_clicked: Browse path addon";

    // Запрос директории
    QString tempPathAddons = QFileDialog::getExistingDirectory (0,QObject::tr("Укажите путь к директории аддонов"), QDir::homePath(), QFileDialog::ShowDirsOnly );

    // Добавляем путь в список
    if(!tempPathAddons.isEmpty()) { // проверяем, выбран ли путь
        // Проверка на дубликаты
        if (ListWidgetContains(ui->addonSearchDirectories, tempPathAddons)) {
            QMessageBox::warning(this, tr("Ошибка!"), tr("Такая директория уже существует."));
            qWarning() << "addonsSettings::on_addonSearchDirectories_add_clicked: error: " << tempPathAddons << "- exists in list";
            return;
        }

        qInfo() << "addonsSettings::on_addonSearchDirectories_add_clicked: " << tempPathAddons << "- add in list";

        // Добавление в список виджета
        ui->addonSearchDirectories->addItem(tempPathAddons);
    }
}

//..удаление выбранной директории
void addonsSettings::on_addonSearchDirectories_del_clicked() {

    qInfo() << "addonsSettings::on_addonSearchDirectories_del_clicked: del Directories";

    // Получаем текущение положение row
    int row = ui->addonSearchDirectories->currentRow();

    // Удаляем итеи и обновляем список аддонов
    if(row != -1) { // Проверяем, выбран ли итем
        ui->addonSearchDirectories->removeItemWidget(ui->addonSearchDirectories->takeItem(row));
    }
}

// Установка приоритетов запуска..
//..передвинуть выбранный итем вверх
void addonsSettings::on_addonsPriorities_up_clicked() {

    qInfo() << "addonsSettings::on_addonsPriorities_up_clicked: move addon UP";

    // Получаем текущение положение row
    int row = ui->addonsPriorities->currentRow();

    // Перемещаем итем вверх
    if(row != 0 && row != -1) { // Проверяем, выбран ли итем и не является ли он самым верхним
        QListWidgetItem *item = ui->addonsPriorities->takeItem(row);
        ui->addonsPriorities->removeItemWidget(item);
        ui->addonsPriorities->insertItem(row-1, item);
        ui->addonsPriorities->setCurrentRow(row-1);
    }
}

//..передвинуть выбранный итем вниз
void addonsSettings::on_addonsPriorities_down_clicked() {

    qInfo() << "addonsSettings::on_addonsPriorities_down_clicked: move addon Down";

    // Получаем текущение положение row
    int row = ui->addonsPriorities->currentRow();

    // Перемещаем итем вниз
    if(row != ui->addonsPriorities->count()-1 && row != -1) { // Проверяем, выбран ли итем и не является ли он самым нижним
        QListWidgetItem *item = ui->addonsPriorities->takeItem(row);
        ui->addonsPriorities->removeItemWidget(item);
        ui->addonsPriorities->insertItem(row+1, item);
        ui->addonsPriorities->setCurrentRow(row+1);
    }
}

// Применение изменений, отправка изменений в главное окно
void addonsSettings::on_buttonBox_accepted() {

    qInfo() << "addonsSettings::on_buttonBox_accepted: accpeted - send data";

    // Обнуляем переменные перед дополнением данными
    listDirs.clear();
    listPriorityAddonsDirs.clear();

    // Получаем список директорий для поиска
    for(int i = 0; i<ui->addonSearchDirectories->count();i++)
        listDirs.append(ui->addonSearchDirectories->item(i)->text());

    // Получаем список папок и путей аддонов в сорт. по приоритету
    for(int i = 0; i <ui->addonsPriorities->count();i++) {
        listPriorityAddonsDirs.append(ui->addonsPriorities->item(i)->text());
    }

    // отправляем данные в главное окно
    emit sendData(listDirs, listPriorityAddonsDirs);
}
