#include "launcher.h"
#include "ui_launcher.h"

/*
 * Обновление информации...
 */

//...в списке аддонов
void launcher::updateInformationInAddonList() {

    qInfo() << "launcher::updateInformationInAddonList: start";

    QStringList listDir;                         // Объявляем 2мерный массив папок аддонов
    QString fileName;                                   // Объявляем путь к информации аддонов
    QFile file;                                         // Объвление ссылки на файл
    QString addonName;

    //Очищаем виджет, для обновления информации в нем
    ui->addonTree->clear();

    // Проверка, содержит ли listDir папку основного директории армы
    QString tempPath = pathFolder;
    tempPath.remove("/arma3.exe");
    if(!listDirs.contains(tempPath)) {
        listDirs.append(tempPath);
    }

    // Получения информации о аддонах и добавление её в виджет
    auto end = listDirs.constEnd();
    for (auto it = listDirs.constBegin(); it != end; ++it) {
        // Получаем список папко по i директории
        listDir = QDir((*it)).entryList(QDir::Dirs);
        //..удаляем лишние папки
        for(auto itDir = listDir.begin(); itDir != listDir.end();) {
            if((*itDir) == "." || (*itDir) == ".." || (*itDir)[0] != '@' || (*itDir).size() == 1)
                itDir = listDir.erase(itDir);
            else ++itDir;
        }

        // Получаем имя аддона
        auto endDir = listDir.constEnd();
        for(auto itDir = listDir.constBegin();itDir != endDir;++itDir) {

            addonName.clear();
            // Получаем путь к информации
            fileName = (*it) + "/" + (*itDir)+"/mod.cpp";
            file.setFileName(fileName);
            // Открываем файл и получаем информацию
            if (file.open(QFile::ReadOnly | QFile::Text)) {
                QTextCodec* defaultTextCodec = QTextCodec::codecForName("CP1251");
                QTextDecoder *decoder = new QTextDecoder(defaultTextCodec);
                QString str = decoder->toUnicode(file.readAll());
                int index = str.indexOf(QString("name = "));
                for (index = index+8;str[index] != '"';index++)
                    addonName.append(str[index]);
                file.close();
            }

            // Добавление информации в виджет
            QTreeWidgetItem *item = new QTreeWidgetItem(ui->addonTree);
            item->setCheckState(0, Qt::Unchecked);
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setText(0, addonName);
            item->setText(1, (*itDir));
            item->setText(2, (*it));
            // Добавляем dir аддона, если его ещё нет в списке приоритетов
            if(!listPriorityAddonsDirs.contains((*itDir))) {
                listPriorityAddonsDirs.append((*itDir));
            }
         }
    }
}

// Обновление информации в памяти
// Перенос информации из виджетов в память программы..
//.. Перенос: Путь исполняемого файл | список директорий для поиска | список папок и путей приор. в сорт. по приоритету
//.. | список активированных аддонов ;
void launcher::updateInformationInMem() {

    qInfo() << "launcher::updateInformationInMem: start";
    // Получаем путь исполняемого файла
    pathFolder = ui->pathFolder->text();

    // Очищаем списки
    checkAddons.clear();

    // Получаем список активированных адонов
    for(QTreeWidgetItemIterator it(ui->addonTree); (*it); ++it) {
        if((*it)->checkState(0) == Qt::Checked)
            checkAddons.append((*it)->text(2)+"/"+(*it)->text(1));
    }
}

// Обновление информации в конфиге
void launcher::updateInformationInCfg() {

    // Предварительно соберем нужную информацию
    updateInfoParametersInMem();
    updateInformationInMem();

    QFile file(DocumentsLocation + "/Arma 3 - Other Profiles/armalauncher.cfg");
    if(file.open(QIODevice::WriteOnly))     //Если файл открыт успешно
    {
        qInfo() << "launcher::updateInformationInCfg: save inf. in cfg -" << file.fileName();
        QDataStream out(&file);             //Создаем поток для записи данных в файл

        //В поток
        out << pathFolder << listDirs << listPriorityAddonsDirs
            << favServers << parameters
            << checkAddons << this->size() << repositories << settings;

    } else {
        qInfo() << "launcher::updateInformationInCfg: save inf. in cfg - fail";
    }
    file.close();
}

// Обновление информации в виджете
// Перенос имеющийся информации в памяти, заполняем виджеты, без дополнений
void launcher::updateInformationInWidget() {

    qInfo() << "launcher::updateInformationInWidget: start";

    // Тригер, что обновление информции в процессе
    // (нужно что бы не срабатывали ненужные сигналы)
    updateInfoInWidget = true;
    //bool exist;
    QTreeWidgetItem *item;

    // Обнуляем информацию в списковых виджетах
    ui->serversTree->clear();
    ui->selectServer->clear();
    ui->addonsFolders->clear();

    // Вызов ещё одного обновления информации
    updateInformationInAddonList();

    // Указываем путь исполняемого файла
    ui->pathFolder->setText(pathFolder);

    // Заполняем информацией виджет..
    //..список путей скачивания\обновления аддонов
    ui->addonsFolders->addItems(listDirs);

    //...дерева избранных серверов комбо-бокса выбранного сервера
    // Нулевой элемент, означает не выбранность сервера
    ui->selectServer->addItem(QString(tr("- Сервер не выбран -")),-1);
    ui->selectServer->setCurrentIndex(0);
    // Добавление сервера
    auto end = favServers.constEnd();
    for(auto itServer = favServers.constBegin(); itServer != end; ++itServer) {
        item = new QTreeWidgetItem(ui->serversTree);
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        item->setText(1, (*itServer).serverIP);
        item->setText(2, (*itServer).serverPort);
        item->setText(3, (*itServer).serverPass);

        // Добавление онлайн информации
        QIcon icon;
        if(!(*itServer).ping.isEmpty()) { // Если есть пинг у сервера, то заполняем онлайн информацией
            item->setText(0, (*itServer).HostName);
            item->setText(4, (*itServer).NumPlayers + "/" + (*itServer).MaxPlayers);
            item->setText(5, (*itServer).ping);
            icon.addFile(QStringLiteral(":/myresources/serverOn.png"), QSize(), QIcon::Normal, QIcon::Off);
            item->setIcon(5, icon);
        }
        else { // Если не пингуется, то заполняем стандартной информацией
            item->setText(0, (*itServer).serverName);
            item->setText(4, "-/-");
            item->setText(5, " - ");
            icon.addFile(QStringLiteral(":/myresources/serverOff.png"), QSize(), QIcon::Normal, QIcon::Off);
            item->setIcon(5, icon);
            item->setIcon(5, icon);
        }
        // Даем элементу индекс, который соответствует индуксу в памяти
        // (для более простой работы)
        int index = itServer-favServers.constBegin();
        item->setData(0, Qt::UserRole, index);

        // Заполняем комбобокс серверов
        if(!(*itServer).ping.isEmpty()) { // Если есть пинг у сервера, то заполняем онлайн информацией
            ui->selectServer->addItem((*itServer).HostName);
            icon.addFile(QStringLiteral(":/myresources/serverOn.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->selectServer->setItemIcon(index+1, icon);
        }
        else {  // Если не пингуется, то заполняем стандартной информацией
            ui->selectServer->addItem((*itServer).serverName);
            icon.addFile(QStringLiteral(":/myresources/serverOff.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->selectServer->setItemIcon(index+1, icon);
        }
    }

    //Проверка, все ли аддоны в списке приоритетов
    for(QTreeWidgetItemIterator it(ui->addonTree); (*it); ++it) {
        if(!listPriorityAddonsDirs.contains((*it)->text(1))) {
           listPriorityAddonsDirs.append((*it)->text(1));
        }
    }

    // Обновление списка репозиториев
    int repoListRow = ui->repoList->currentRow();
    ui->repoList->clear();
    auto endRepo = repositories.constEnd();
    for(auto itRepo = repositories.constBegin(); itRepo != endRepo; ++itRepo) {
        ui->repoList->addItem((*itRepo).name);
        QIcon icon;
        if((*itRepo).type == 0)
            icon.addFile(QStringLiteral(":/repositories/IMG/yoma2009.ico"), QSize(), QIcon::Normal, QIcon::Off);
        else
            icon.addFile(QStringLiteral(":/repositories/IMG/arma3sync.ico"), QSize(), QIcon::Normal, QIcon::Off);

        ui->repoList->item(itRepo-repositories.constBegin())->setIcon(icon);
    }
    ui->repoList->setCurrentRow(repoListRow);

    // Говорим сигналам, что мы закончили изменения
    updateInfoInWidget = false;
}

// Обновление информации параметров в памяти
void launcher::updateInfoParametersInMem() {

    qInfo() << "launcher::updateInfoParametersInMem: start";

    // Получаем информацию с бокса  - Настройки игры
    parameters.check_name = ui->check_name->isChecked();
    parameters.name = ui->name->currentText();
    parameters.window = ui->window->isChecked();
    parameters.noPause = ui->noPause->isChecked();
    parameters.showScriptErrors = ui->showScriptErrors->isChecked();
    parameters.noFilePatching = ui->noFilePatching->isChecked();
    parameters.battlEye = ui->battleEye->isChecked();                      // Добавлено после патча *1.44*
    // Получаем информацию с бокса - Производительность
    parameters.priorityLaunch = ui->priorityLaunch->currentIndex();
    parameters.check_maxMem = ui->check_maxMem->isChecked();
    parameters.maxMem = ui->maxMem->currentText();
    parameters.check_maxVRAM = ui->check_maxVRAM->isChecked();
    parameters.maxVRAM = ui->maxVRAM->currentText();
    parameters.check_cpuCount = ui->check_cpuCount->isChecked();
    parameters.cpuCount = ui->cpuCount->currentText();
    parameters.check_exThreads = ui->check_exThreads->isChecked();
    parameters.exThreads = ui->exThreads->currentText();
    parameters.check_malloc = ui->check_malloc->isChecked();
    parameters.malloc = ui->malloc->currentText();
    parameters.enableHT = ui->enableHT->isChecked();
    parameters.winxp = ui->winxp->isChecked();
    parameters.noCB = ui->noCB->isChecked();
    parameters.nosplash = ui->nosplash->isChecked();
    parameters.skipIntro = ui->skipIntro->isChecked();
    parameters.worldEmpty = ui->worldEmpty->isChecked();
    parameters.noLogs = ui->noLogs->isChecked();
    // Получаем строку с дополнительными параметрами
    parameters.addParam = ui->addParameters->text();

    // Заполняем виджет с запускаемыми параметрами
    updateInfoInRunParametersWidget();
}

// Обновление информации параметров в виджете
// Заполняем виджеты параметров информацие из памяти
void launcher::updateInfoParametersInWidget() {

    qInfo() << "launcher::updateInfoParametersInWidget: start";

    // Заполняем информацией блок - Настроек игры
    ui->check_name->setChecked(parameters.check_name);
    ui->name->setCurrentText(parameters.name);
    ui->window->setChecked(parameters.window);
    ui->noPause->setChecked(parameters.noPause);
    ui->showScriptErrors->setChecked(parameters.showScriptErrors);
    ui->noFilePatching->setChecked(parameters.noFilePatching);
    ui->battleEye->setChecked(parameters.battlEye);                       // Добавлено после патча *1.44*
    // Заполняем информацией блок - Производительности
    ui->priorityLaunch->setCurrentIndex(parameters.priorityLaunch);
    ui->check_maxMem->setChecked(parameters.check_maxMem);
    ui->maxMem->setCurrentText(parameters.maxMem);
    ui->check_maxVRAM->setChecked(parameters.check_maxVRAM);
    ui->maxVRAM->setCurrentText(parameters.maxVRAM);
    ui->check_cpuCount->setChecked(parameters.check_cpuCount);
    ui->cpuCount->setCurrentText(parameters.cpuCount);
    ui->check_exThreads->setChecked(parameters.check_exThreads);
    ui->exThreads->setCurrentText(parameters.exThreads);
    ui->check_malloc->setChecked(parameters.check_malloc);
    ui->malloc->setCurrentText(parameters.malloc);
    ui->enableHT->setChecked(parameters.enableHT);
    ui->winxp->setChecked(parameters.winxp);
    ui->noCB->setChecked(parameters.noCB);
    ui->nosplash->setChecked(parameters.nosplash);
    ui->skipIntro->setChecked(parameters.skipIntro);
    ui->worldEmpty->setChecked(parameters.worldEmpty);
    ui->noLogs->setChecked(parameters.noLogs);
    // Заполнен строку дополнительных параметров
    ui->addParameters->setText(parameters.addParam);

    // Заполняем виджет с запускаемыми параметрами
    updateInfoInRunParametersWidget();
}

// Обновление информации запускаемых параметров в виджете
void launcher::updateInfoInRunParametersWidget() {

    qInfo() << "launcher::updateInfoInRunParametersWidget: start";

    // Заполняем виджет
    ui->runParameters->clear();
    ui->runParameters->append(getLaunchParam().join('\n'));
}
