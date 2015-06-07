#include "launcher.h"
#include "ui_launcher.h"

/*
 * Обновление информации...
 */

//...в списке аддонов
void launcher::updateInformationInAddonList() {

    qDebug() << "Debug-updInf: Upd. Inf. In Adoon List";

    int addonIndex = 0;                                 // Объявляем счетчик аддонов
    QList<QStringList> listDir;                         // Объявляем 2мерный массив папок аддонов
    QString fileName;                                   // Объявляем путь к информации аддонов
    QStringList addonsNames;                            // Объявляем список имен аддонов
    QList<QTreeWidgetItem *> Items;                     // Объявляем список итемов TreeWidget
    QFile file;                                         // Объвление ссылки на файл
    QString temp;

    //Очищаем виджет, для обновления информации в нем
    ui->addonTree->clear();

    // Проверка, содержит ли listDir папку основного директории армы
    QString tempPath = pathFolder;
    tempPath.remove("/arma3.exe");
    if(!listDirs.contains(tempPath)) {
        listDirs.append(tempPath);
    }

    // Получения информации о аддонах и добавление её в виджет
    for (int i = 0; i < listDirs.size(); i++) {
        // Получаем список папко по i директории
        listDir.append(QDir(listDirs[i]).entryList(QDir::Dirs));
        for(int j=0;j<listDir[i].size();j++)
            if(listDir[i].at(j) == "." || listDir[i].at(j) == ".." || listDir[i].at(j)[0] != '@' || listDir[i].at(j).size() == 1)
                listDir[i].removeAt(j--);

        // Получаем имя аддона
        for(int j=0; j<listDir[i].size();j++) {

            temp.clear();
            // Получаем путь к информации
            fileName = listDirs[i] + "/" + listDir[i][j]+"/mod.cpp";
            file.setFileName(fileName);
            // Открываем файл и получаем информацию
            if (file.open(QFile::ReadOnly | QFile::Text)) {
                QTextCodec* defaultTextCodec = QTextCodec::codecForName("CP1251");
                QTextDecoder *decoder = new QTextDecoder(defaultTextCodec);
                QString str = decoder->toUnicode(file.readAll());
                int index = str.indexOf(QString("name = "));
                for (index = index+8;str[index] != '"';index++)
                    temp.append(str[index]);
                file.close();
            }
            addonsNames.append(temp);

            // Добавление информации в виджет
            Items.append( new QTreeWidgetItem(ui->addonTree));
            Items.at(addonIndex)->setCheckState(0, Qt::Unchecked);
            Items.at(addonIndex)->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            Items.at(addonIndex)->setText(0, addonsNames[addonIndex]);
            Items.at(addonIndex)->setText(1, listDir[i][j]);
            Items.at(addonIndex)->setText(2,listDirs[i]);
            // Добавляем dir аддона, если его ещё нет в списке приоритетов
            if(!listPriorityAddonsDirs.contains(listDir[i][j])) {
                listPriorityAddonsDirs.append(listDir[i][j]);
            }
            addonIndex++;
         }
    }

    qDebug() << "Debug-updInf: Upd. Inf. In Adoon List - succ";
}

// Обновление информации в памяти
// Перенос информации из виджетов в память программы..
//.. Перенос: Путь исполняемого файл | список директорий для поиска | список папок и путей приор. в сорт. по приоритету
//.. | список активированных аддонов ;
void launcher::updateInformationInMem() {

    qDebug() << "Debug-updInf: Upd. Inf. In Mem";
    // Получаем путь исполняемого файла
    pathFolder = ui->pathFolder->text();

    // Очищаем списки
    checkAddons.clear();

    // Получаем список активированных адонов
    for(int i=0; i <ui->addonTree->topLevelItemCount();i++) {
        if(ui->addonTree->topLevelItem(i)->checkState(0) == Qt::Checked)
            checkAddons.append(ui->addonTree->topLevelItem(i)->text(2)+"/"+ui->addonTree->topLevelItem(i)->text(1));
    }
    qDebug() << "Debug-updInf: Upd. Inf. In Mem - succ";
}

// Обновление информации в конфиге
// Перенос всей нужной информации из виджетов в файл конфига:
// Список: pathFolder, listDirs, listPriorityAddonsDirs, listPriorityAddonsFolders, favServers, parameters, checkAddons
// Примечание: Некоторые структуру имеют перегруженые операции ввода\вывода потока - все в cfg.h
void launcher::updateInformationInCfg() {

    qDebug() << "Debug-updInf: Upd. Inf. In Cfg";

    // Предварительно соберем нужную информацию
    updateInfoParametersInMem();
    updateInformationInMem();

    QFile file(DocumentsLocation + "/Arma 3 - Other Profiles/armalauncher.cfg");
    if(file.open(QIODevice::WriteOnly))     //Если файл открыт успешно
    {
        qDebug() << "Debug-updInf: Save inf. in cfg - " << file.fileName();
        QDataStream out(&file);             //Создаем поток для записи данных в файл

        //В поток
         out << pathFolder << listDirs << listPriorityAddonsDirs
             << favServers << parameters
             << checkAddons << this->size() << repositories << settings;

         qDebug() << "Debug-updInf: Upd. Inf. In Cfg - succ";
    } else {
        qDebug() << "Debug-updInf: Upd. Inf. In Cfg - fail";
    }
}

// Обновление информации в виджете
// Перенос имеющийся информации в памяти, заполняем виджеты, без дополнений
void launcher::updateInformationInWidget() {

    qDebug() << "Debug-updInf: Upd. Inf. In Widget";

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
    for(int i = 0; i<listDirs.count();i++)
        ui->addonsFolders->addItem(listDirs[i]);

    //...дерева избранных серверов
    for(int i = 0; i<favServers.count();i++) {
        item = new QTreeWidgetItem(ui->serversTree);
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        item->setText(1, favServers.at(i).serverIP);
        item->setText(2, favServers.at(i).serverPort);
        item->setText(3, favServers.at(i).serverPass);

        // Добавление онлайн информации
        QIcon icon;
        if(!favServers.at(i).ping.isEmpty()) { // Если есть пинг у сервера, то заполняем онлайн информацией
            item->setText(0, favServers.at(i).HostName);
            item->setText(4, favServers.at(i).NumPlayers + "/" + favServers.at(i).MaxPlayers);
            item->setText(5, favServers.at(i).ping);
            icon.addFile(QStringLiteral(":/myresources/serverOn.png"), QSize(), QIcon::Normal, QIcon::Off);
            item->setIcon(5, icon);
        }
        else { // Если не пингуется, то заполняем стандартной информацией
            item->setText(0, favServers.at(i).serverName);
            item->setText(4, "-/-");
            item->setText(5, " - ");
            icon.addFile(QStringLiteral(":/myresources/serverOff.png"), QSize(), QIcon::Normal, QIcon::Off);
            item->setIcon(5, icon);
            item->setIcon(5, icon);
        }
        // Даем элементу индекс, который соответствует индуксу в памяти
        // (для более простой работы)
        item->setData(0, Qt::UserRole, i);
    }
    //..комбо-бокса, избранными серверами
    // Нулевой элемент, означает не выбранность сервера
    ui->selectServer->addItem(QString("- Сервер не выбран -"),-1);
    //Заполняем комбо-бокс вариантами серверов
    for (int i = 0;i<favServers.count();i++) {
        // Заполняем по возможности онлайн информацией
        QIcon icon;
        if(!favServers.at(i).ping.isEmpty()) { // Если есть пинг у сервера, то заполняем онлайн информацией
            ui->selectServer->addItem(favServers.at(i).HostName);
            icon.addFile(QStringLiteral(":/myresources/serverOn.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->selectServer->setItemIcon(i+1, icon);
        }
        else {  // Если не пингуется, то заполняем стандартной информацией
            ui->selectServer->addItem(favServers.at(i).serverName);
            icon.addFile(QStringLiteral(":/myresources/serverOff.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->selectServer->setItemIcon(i+1, icon);
        }
    }
    ui->selectServer->setCurrentIndex(0);

    //Проверка, все ли аддоны в списке приоритетов
    for(int i = 0;i<ui->addonTree->topLevelItemCount();i++) {
        if(!listPriorityAddonsDirs.contains(ui->addonTree->topLevelItem(i)->text(1))) {
           listPriorityAddonsDirs.append(ui->addonTree->topLevelItem(i)->text(1));
        }
    }

    // Обновление списка репозиториев
    int repoListRow = ui->repoList->currentRow();
    ui->repoList->clear();
    for(int i = 0;i<repositories.size();i++) {
        ui->repoList->addItem(repositories[i].name);
        QIcon icon;
        if(repositories[i].type == 0)
            icon.addFile(QStringLiteral(":/repositories/IMG/yoma2009.ico"), QSize(), QIcon::Normal, QIcon::Off);
        else
            icon.addFile(QStringLiteral(":/repositories/IMG/arma3sync.ico"), QSize(), QIcon::Normal, QIcon::Off);

        ui->repoList->item(i)->setIcon(icon);
    }
    ui->repoList->setCurrentRow(repoListRow);

    // Говорим сигналам, что мы закончили изменения
    updateInfoInWidget = false;

    // Радуемся, что не словили ошибку сегментации
    qDebug() << "Debug-updInf: Upd. Inf. In Widget - Succ";
}

// Обновление информации параметров в памяти
void launcher::updateInfoParametersInMem() {

    qDebug() << "Debug-updInf: Upd. Inf. Param. In Mem";
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

    qDebug() << "Debug-updInf: Upd. Inf. Param. In Mem - succ";
    // Заполняем виджет с запускаемыми параметрами (просто потому, что я могу)
    updateInfoInRunParametersWidget();
}

// Обновление информации параметров в виджете
// Заполняем виджеты параметров информацие из памяти
void launcher::updateInfoParametersInWidget() {

    qDebug() << "Debug-updInf: Upd. Inf. Param. In Widget";

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

    qDebug() << "Debug-updInf: Upd. Inf. Param. In Widget - succ";
    // Заполняем виджет с запускаемыми параметрами (просто потому, что я могу)
    updateInfoInRunParametersWidget();
}

// Обновление информации запускаемых параметров в виджете
void launcher::updateInfoInRunParametersWidget() {

    qDebug() << "Debug-updInf: Upd. Inf. In Ru Param. Widget";

    // Получения списка параметров запуска
    QStringList args = getLaunchParam();
    // Очищаем виджет
    ui->runParameters->clear();
    // Заполняем виджет
    for(int i = 0; i<args.count();i++)
      ui->runParameters->append(args[i]);
    qDebug() << "Debug-updInf: Upd. Inf. In Ru Param. Widget - succ";
}
