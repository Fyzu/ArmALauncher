#include "launcher.h"
#include "ui_launcher.h"

// Конструктор главного окна
launcher::launcher(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::launcher) {

    // Подключаю вспомогательную Dll "ArmaLauncher.dll"
    /*! Список функций ArmaLauncher.dll
     ***********************************************************************************
     *  1: Функция возвращающая подробную информацию о сервере
     * unsigned char* exchangeDataWithServer(const char* host, int port, const int timeout, const unsigned char* str, const int len, int &ping);
     *---------------------------------------------------------------------------------------------
     * in:	const char* h				- Адресс сервера
     *		int port					- Порт сервера
     *		const int timeout			- Максимальное время ответа от сервера (в милисекундах)
     *		const unsigned char* str	- Отправляемая строка
     *		const int len				- Длина отправляемой строки
     *---------------------------------------------------------------------------------------------
     * out:	unsigned char*				- Принятая строка от сервера
     *		int &ping					- Время ответа от сервера
     ***********************************************************************************
     */
    qDebug() << "Debug-launcher: Dll initialization";
    //..устанавливаем название длл
    library.setFileName("ArmaLauncher.dll");
    //..проверяем, загрузилась ли длл
    if(!library.load()) {
        QMessageBox::critical(this,"Критическая ошибка", "Не найдена библиотека \"ArmaLauncher.dll\"\nПереустановите программу с официального сайта.\nhttp://launcher.our-army.su", QMessageBox::Ok);
        parent->close();
    }
    //..проверяем, есть ли 7z.exe в папке с программой
    if(!QFile::exists(QCoreApplication::applicationDirPath() + "/7z.exe")){
        QMessageBox::critical(this,"Критическая ошибка", "Не найден исполняемый файл \"7z.exe\"\nПереустановите программу с официального сайта.\nhttp://launcher.our-army.su", QMessageBox::Ok);
        parent->close();
    }
    //..проверяем, есть ли ArmALauncher-SyncParser.exe в папке с программой
    if(!QFile::exists(QCoreApplication::applicationDirPath() + "/ArmALauncher-SyncParser.exe")){
        QMessageBox::critical(this,"Критическая ошибка", "Не найден исполняемый файл \"ArmALauncher-SyncParser.exe\"\nПереустановите программу с официального сайта.\nhttp://launcher.our-army.su", QMessageBox::Ok);
        parent->close();
    }
    //..проверяем, есть ли ArmALauncher-Updater.exe в папке с программой
    if(!QFile::exists(QCoreApplication::applicationDirPath() + "/ArmALauncher-Updater.exe")){
        QMessageBox::critical(this,"Критическая ошибка", "Не найден исполняемый файл \"ArmALauncher-Updater.exe\"\nПереустановите программу с официального сайта.\nhttp://launcher.our-army.su", QMessageBox::Ok);
        parent->close();
    }
    //..иницилизируем функции длл
    exchangeDataWithServer = (ExchangeDataWithServer)library.resolve("exchangeDataWithServer");
    //..проверяем инцилизированы ли функции
    if(!exchangeDataWithServer) {
        QMessageBox::critical(this,"Критическая ошибка", "Не найдена библиотека \"ArmaLauncher.dll\"\nПереустановите программу с официального сайта.\nhttp://launcher.our-army.su", QMessageBox::Ok);
        parent->close();
    }

    // Иницилизируем форму окна и его переменные
    qDebug() << "Debug-launcher: Form initialization";

    ui->setupUi(this);
    this->setWindowTitle("ArmA 3 Launcher by Fyzu | ver. " + QString(VER_FILEVERSION_STR));
    dxDiagIsRunning = false;
    updateAfterClose = false;
    updater = 0;

    // Стартовые параметры виджетов..
    //..начальная вкладка
    ui->tabWidget->setCurrentIndex(0);
    ui->repositoryList->setCurrentIndex(0);
    //..размер секции "название аддона"
    ui->addonTree->header()->resizeSection(0, 220);
    ui->addonTree->header()->resizeSection(1, 150);
    //..размеры секций списка серверов
    ui->serversTree->header()->resizeSection(0, 270);
    ui->serversTree->header()->resizeSection(1, 100);
    ui->serversTree->header()->resizeSection(2, 50);
    ui->serversTree->header()->resizeSection(3, 70);
    ui->serversTree->header()->resizeSection(4, 70);
    ui->serversTree->header()->resizeSection(5, 50);
    //..размер секций filesTree
    ui->filesTree->header()->resizeSection(0, 200);
    ui->filesTree->header()->resizeSection(1, 90);
    ui->filesTree->header()->resizeSection(2, 80);

    // Инициализация формы serverEdit
    edit = new serverEdit(this);
    // Инициализация формы addonsSettings
    AddonsSettings = new addonsSettings(this);

    // Инициализация формы launcherSettings
    LauncherSettings = new launcherSettings(this);

    // Инициализация формы deleteOtherFiles
    delOtherFiles = new deleteOtherFiles(this);

    // Инициализация формы repoEdit
    repositoryEdit = new repoEdit(this);

    // Инициализируем launcherUpdate
    LauncherUpdate = new launcherUpdate(this);

    // Иницилизируем updater
    updater = new updateAddons;

    //..переносим его в новый поток
    thread = new QThread;
    updater->moveToThread(thread);

    qDebug() << "Debug-updateAddons_UI: Updater move to thread" << thread;

    //..связываем updater с потоком
    connect(this,    SIGNAL(showUpdater()),   updater, SLOT(start()), Qt::DirectConnection);
    connect(updater, SIGNAL(finished()),      thread,  SLOT(quit()));
    connect(updater, SIGNAL(finished()),      updater, SLOT(deleteLater()));
    connect(thread,  SIGNAL(finished()),      thread,  SLOT(deleteLater()));

    thread->start();

    qDebug() << "Debug-launcher: Form initialization - succ";

    // Коннект..
    //..Изменения настроек сервера
    connect(ui->serversTree_edit,   SIGNAL( clicked()),
            this,                   SLOT( Send()));
    connect(this,                   SIGNAL( sendData(favServer, QList<addon>,bool,QStringList)),
            edit,                   SLOT( recieveData(favServer, QList<addon>,bool,QStringList)));
    connect(edit,                   SIGNAL( sendData(favServer,bool)),
            this,                   SLOT( recieveData(favServer,bool)));
    //..Изменения настроек аддонов
    connect(this,                   SIGNAL(addonsSettingsStart(Settings, QStringList,QStringList,QStringList)),
            AddonsSettings,         SLOT(receiveData(Settings, QStringList,QStringList,QStringList)));
    connect(AddonsSettings,         SIGNAL(sendData(QStringList,QStringList)),
            this,                   SLOT(addonsSettingsFinish(QStringList,QStringList)));
    //..Изменения настроек лаунчера
    connect(this,                   SIGNAL(launcherSettingsStart(Settings)),
            LauncherSettings,       SLOT(reciveData(Settings)));
    connect(LauncherSettings,       SIGNAL(sendData(Settings)),
            this,                   SLOT(launcherSettingsFinish(Settings)));
    //..Изменения настроек репозитория
    connect(this,                   SIGNAL(repoEditStart(Repository,int,bool)),
            repositoryEdit,         SLOT(recieveData(Repository,int,bool)));
    connect(repositoryEdit,         SIGNAL(sendData(Repository,int,bool)),
            this,                   SLOT(repoEditFinish(Repository,int,bool)));
    //..Проверки новой версии
    connect(this,                   SIGNAL(newVersion(Settings,QString)),
            LauncherUpdate,         SLOT(newVersion(Settings,QString)));
    connect(LauncherUpdate,         SIGNAL(result(int)),
            this,                   SLOT(launcherUpdateResult(int)));

    // Получаем путь к документам системы
    DocumentsLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    // Считываем cfg (настройки лаунчера)
    QFile file(DocumentsLocation + "/Arma 3 - Other Profiles/armalauncher.cfg");
    if(file.open(QIODevice::ReadOnly)) {

        //открываем поток ввода
        QDataStream in(&file);

        //Из потока
        in >> pathFolder >> listDirs >> listPriorityAddonsDirs
           >> favServers >> parameters
           >> checkAddons >> widgetSize >> repositories >> settings;
        file.close();

        // Заполняем дефолтной информацией, если слоты пустые
        //..репозитории
        if(repositories.isEmpty()) {
            //..репозиторий Тушино Главный
            Repository rep1;
            rep1.name = "Tushino Arma3 Main";
            rep1.type = 0;
            rep1.url = "ftp://armaplayer:1@46.188.16.205:21021/addons/Repo_TSG_A3/Tushino.7z";
            repositories.append(rep1);
            //..репозиторий Тушино Зеркало
            Repository rep2;
            rep2.name = "Tushino Arma3 Mirror";
            rep2.type = 0;
            rep2.url = "http://arma3.bayonetdiv.com/Tushino.7z";
            repositories.append(rep2);
            //..репозиторий Нашей Армии
            Repository rep3;
            rep3.name = "Our-Army.su [USSR] Server";
            rep3.type = 1;
            rep3.url = "http://addons.our-army.su/main/.a3s/autoconfig";
            repositories.append(rep3);
        }
        //..сервера
        if(favServers.isEmpty()) {
            //..игровой сервер тушино
            favServer favServ1;
            favServ1.serverName = "TUSHINO Serious Games Arma3 #1";
            favServ1.serverIP = "46.188.16.205";
            favServ1.serverPort = "2102";
            favServ1.serverPass = "2010";
            favServers.append(favServ1);
            //..игровой сервер Нашей Армии
            favServer favServ2;
            favServ2.serverName = "Our-Army.su [USSR] Server";
            favServ2.serverIP = "144.76.82.2";
            favServ2.serverPort = "2302";
            favServers.append(favServ2);
        }

        // Установить главному окну Size
        if (!widgetSize.isEmpty()) {
            this->resize(widgetSize);
            qDebug() << "Debug-launcher: Resize main widget - succ";
        }

        updateInformationInWidget();
    }

    // Настройки лаунчера
    launcherSettingsFinish(settings);

    // Добавление доступных профилей (игровые профили арма 3)
    QStringList names = QDir(DocumentsLocation + "/Arma 3 - Other Profiles").entryList(QDir::Dirs);
    for(int j=0;j<names.size();j++) {
        if(names.at(j) == "." || names.at(j) == "..") {
            names.removeAt(j);
            j--;
        }
     }
    ui->name->addItems(names);

    // Добавление доступных распределителей памяти в настройки (храняться там же где и исполняемый файл игры)
    QString temp = pathFolder;
    QStringList mallocs = QDir(temp.replace("arma3.exe","Dll")).entryList(QDir::Files);
    for(int j=0;j<mallocs.size();j++)
        if(mallocs[j].contains(".dll"))
            mallocs[j].remove(".dll");
        else
            mallocs.removeAt(j--);
    ui->malloc->addItems(mallocs);

    // Отмечаю включенные аддоны с последнего запуска
    for(int i = 0; i<checkAddons.count();i++)
        for(int j = 0; j<ui->addonTree->topLevelItemCount();j++)
            if(checkAddons[i] == ui->addonTree->topLevelItem(j)->text(2)+"/"+ui->addonTree->topLevelItem(j)->text(1))
                ui->addonTree->topLevelItem(j)->setCheckState(0,Qt::Checked);

    // Обновляю информацию..
    //..виджетах
    updateInfoParametersInWidget();
    //..информация о серверах
    on_serversTree_update_clicked();
    ui->filesTree->setColumnCount(1);

    // Получаем версию игры
    ui->gameVersionlabel->setText("Версия игры: " + getFileVersion(pathFolder));

    qDebug() << "Debug-launcher: Widgets initialization - succ";

    // Проверка на обновления
    checkForUpdates();
}

// Дистркутор окна
launcher::~launcher() {

    // Сохранение настроек перед закрытием программы
    updateInformationInCfg();

    thread->quit();
    thread->deleteLater();
    delete updater;
    delete repositoryEdit;
    delete delOtherFiles;
    delete AddonsSettings;
    delete LauncherSettings;
    delete LauncherUpdate;
    delete edit;
    delete ui;

    qDebug() << "Debug-launcher: Widgets destruction - succ";
}

QString launcher::getFileVersion(QString path) {

    qDebug() << "Debug-launcher: get File Version - start";

    // Получаем версию установленной игры
    if(!pathFolder.isEmpty()) {
        LPCWSTR lpszFilePath = (const wchar_t*) path.utf16();
        DWORD dwDummy;

        // Получаем размер версии файла
        DWORD dwFVISize = GetFileVersionInfoSize(lpszFilePath , &dwDummy );
        if(dwFVISize == 0) {
            qDebug() << "Debug-launcher: get File Version - error: dwFVISize = 0";
            return QString("неизвестно");
        }
        // Создаем байтовый массив с этим размером
        BYTE *lpVersionInfo = new BYTE[dwFVISize];

        // Получаем информацию о версии файла
        if(!GetFileVersionInfo( lpszFilePath , 0 , dwFVISize , lpVersionInfo )) {
            delete[] lpVersionInfo;
            qDebug() << "Debug-launcher: get File Version - error: get file version info";
            return QString("неизвестно");
        }

        // Получаем собственно версию
        UINT uLen;
        VS_FIXEDFILEINFO *lpFfi;
        VerQueryValue( lpVersionInfo , TEXT("\\") , (LPVOID *)&lpFfi , &uLen );
        DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
        DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;
        delete [] lpVersionInfo;

        // Получаем числа версий в ячейках
        DWORD dwLeftMost = HIWORD(dwFileVersionMS);
        DWORD dwSecondLeft = LOWORD(dwFileVersionMS);
        DWORD dwSecondRight = HIWORD(dwFileVersionLS);
        DWORD dwRightMost = LOWORD(dwFileVersionLS);

        // Преобразуем в строку
        return QString::number(dwLeftMost) + '.' + QString::number(dwSecondLeft) + '.' + QString::number(dwSecondRight) + '.' + QString::number(dwRightMost);
    } else {
        return QString("Неизвестно");
    }
}

void launcher::checkForUpdates() {

    qDebug() << "Debug-launcher: check For Updates - start";

    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadVersionFinished(QNetworkReply*)));
    manager->get(QNetworkRequest(QUrl("http://launcher.our-army.su/download/updater/version")));
}

void launcher::downloadVersionFinished(QNetworkReply *reply) {

    QString version;

    if(reply->error()) {
        qDebug() << "Debug-launcher: Error Check Updates" << reply->errorString();
    }
    else {
        version = QString(reply->readAll());
        qDebug() << "Debug-launcher: Version on server - " << version;
    }
    reply->deleteLater();
    manager->deleteLater();

    if(version == QString(VER_FILEVERSION_STR)) {

        qDebug() << "Debug-launcher: Launcher has latest version";

    } else {

        qDebug() << "Debug-launcher: Launcher need to be updated";
        emit newVersion(settings, version);
    }
}

void launcher::launcherUpdateResult(int result) {

    if(result == 0) {
        UpdateLauncher();
        this->close();
    } else {
        updateAfterClose = true;
    }
}

void launcher::UpdateLauncher() {
    QProcess process;
    process.startDetached("ArmALauncher-Updater.exe");
}

// Запуск Arma 3
void launcher::on_play_clicked() {

    qDebug() << "Debug-launcher: Start launch game";

    QStringList args = getLaunchParam();        // Получаем параметры запуска
    if(parameters.check_name) {
        if(favServers[ui->selectServer->currentIndex()-1].check_name == true)
            args[0] = "-name="+favServers[ui->selectServer->currentIndex()-1].name;
    } else {
        if(favServers[ui->selectServer->currentIndex()-1].check_name == true)
            args.append("-name="+favServers[ui->selectServer->currentIndex()-1].name);
    }
    QProcess process(this);                     // Создаем процесс армы
    qint64 ProcessId;                           // Пид армы
    HANDLE hProcess;                            // Хандл процесса

    if(!getHandle("Steam", false)) {
        qDebug() << "Debug-launcher: Start launch fail - steam no found";
        if(QMessageBox::Yes == QMessageBox::question(this, tr("Внимание!"), tr("Игра требует запустить Steam.\nЗапустить Steam?"))) {
            QDesktopServices::openUrl(QUrl("steam://", QUrl::TolerantMode));
            qDebug() << "Debug-launcher: Start launch fail - steam running";
        }
        return;
    }

    // Получаем путь к be *1.44 patch*
    QString pathBattleye = pathFolder;
    pathBattleye.replace("arma3.exe","arma3battleye.exe");

    // Запускаем игру с параметрами
    // Запускае Arma 3 с BE Service
    if(ui->battleEye->checkState() == Qt::Checked) {
        process.startDetached("\"" + pathBattleye + "\"" + " 0 1 " + parameters.addParam, args, QString(), &ProcessId);
        process.waitForStarted();
        hProcess = getHandle ("Arma 3", true);
    } else {
        // Запускае арму Arma 3 без BE
        process.startDetached("\"" + pathFolder + "\"" + parameters.addParam, args, QString(), &ProcessId);
        process.waitForStarted();
        // Получаем hProcess армы
        hProcess = OpenProcess(PROCESS_SET_INFORMATION, false, ProcessId);
    }
    // Даем приоритет Арме
    if(hProcess && ui->priorityLaunch->currentIndex() != 3) {
        QList<DWORD> Priority_class;
        Priority_class << REALTIME_PRIORITY_CLASS   << HIGH_PRIORITY_CLASS          << ABOVE_NORMAL_PRIORITY_CLASS <<
                          NORMAL_PRIORITY_CLASS     << BELOW_NORMAL_PRIORITY_CLASS  << IDLE_PRIORITY_CLASS;
        SetPriorityClass(hProcess, Priority_class[ui->priorityLaunch->currentIndex()]);
    }

    qDebug() << "Debug-launcher: launch game - succ";

    // Сохранение настроек после запуска игры
    updateInformationInCfg();
}

// Получение handle процесса по заголовку
HANDLE launcher::getHandle (QString titleName, bool wait) {


        qDebug() << "Debug-launcher: start find HANDLE";
        DWORD ProcessId;        // Пид процесса
        HWND hWnd = NULL;       // Хандл окна

        // Переводим QString в wchar_t*
        wchar_t * name = new wchar_t[titleName.length() + 1];
        titleName.toWCharArray(name);
        name[titleName.length()] = 0;

        // Каждые 0.5 секунды, в течении 5 секунд пытаемся получить хандл нужного процесса
        // Примечание: это нужно для того, что бы главный процесс успел запустить дочерний, т.е. arma3.exe
        if (wait)
            for(int i = 0;i<11 && !hWnd;i++) {
                Sleep(500);
                hWnd = FindWindow (NULL, _T(name));
                if (i == 10) return NULL;
            }
        else
            hWnd = FindWindow (NULL, _T(name));

        // Проверка, найден ли процесс
        if (!(GetWindowThreadProcessId(hWnd, &ProcessId)))
            return NULL;

        qDebug() << "Debug-launcher: find HANDLE - succ";
        // Возвращаем нужный нам хендл
        return OpenProcess(PROCESS_SET_INFORMATION, false, ProcessId);
}

// Обзор исполняемого файла игры Arma 3
void launcher::on_pathBrowse_clicked() {

    qDebug() << "Debug-launcher: Start Browse path game";
    // Получение нового path исполняемого файла игры
    QString tempPathGame = QFileDialog::getOpenFileName(this, QString(trUtf8("Открыть исполняемый файл ArmA 3")), QString(), QString(trUtf8("arma3.exe;;")));

    // Проверяем, является ли это путь к арма 3
    if (tempPathGame.endsWith(QString("arma3.exe"))) {

            // Удаление старого расположения аддонов
            if(ui->pathFolder->text() != "")
               for(int i = 0; i < listDirs.size(); i++)
                   if (listDirs[i] == ui->pathFolder->text().remove(QString("arma3.exe")))
                       listDirs.removeAt(i);

            // Установка новых путей..
            //..путь к игре
            ui->pathFolder->setText(tempPathGame);
            //..путь к аддонам игры
            tempPathGame = tempPathGame.remove(QString("/arma3.exe"));
            // Проверка на дубликаты путей
            bool duplicate = false;
            for(int i = 0 ; i< listDirs.size();i++)
                if(listDirs[i] == tempPathGame)
                    duplicate = true;
            if(!duplicate) {
                listDirs.append(tempPathGame);
                updateInformationInAddonList();
            }

            // Обновляем информацию в памяти программы
            updateInformationInMem();

            // Получаем версию игры
            ui->gameVersionlabel->setText("Версия игры: " + getFileVersion(pathFolder));

            // и в виджетах заполняем информацию
            updateInformationInWidget();

            qDebug() << "Debug-launcher: Start Browse path game - succ";
    } else QMessageBox::warning(this,"Ошибка!", "Указан неверный путь к исполняемому файлу arma3.exe");
}

// Получение параметров запуска игры
QStringList launcher::getLaunchParam() {

    qDebug() << "Debug-launcher: get launch param";

    // Объявляем возвращаемый список строк
    QStringList launchParam;

    // Начинаем заполнение списка строк параметров
    // Игровые настройки
    if(parameters.check_name)
        launchParam.append("-name="+parameters.name);
    if(parameters.check_malloc)
        launchParam.append("-malloc="+parameters.malloc);
    if(parameters.showScriptErrors)
        launchParam.append("-showScriptErrors");
    if(parameters.noPause)
        launchParam.append("-noPause");
    if(parameters.noFilePatching)
        launchParam.append("-noFilePathcing");
    if(parameters.window)
        launchParam.append("-window");
    if(parameters.check_maxMem)
        launchParam.append("-maxMem="+parameters.maxMem);
    if(parameters.check_maxVRAM)
        launchParam.append("-maxVRAM="+parameters.maxVRAM);
    if(parameters.check_cpuCount)
        launchParam.append("-cpuCount="+parameters.cpuCount);
    if(parameters.check_exThreads)
        launchParam.append("-exThreads="+parameters.exThreads);
    if(parameters.enableHT)
        launchParam.append("-enableHT");
    if(parameters.winxp)
        launchParam.append("-winxp");
    if(parameters.noCB)
        launchParam.append("-noCB");
    if(parameters.nosplash)
        launchParam.append("-nosplash");
    if(parameters.skipIntro)
        launchParam.append("-skipIntro");
    if(parameters.worldEmpty)
        launchParam.append("-world=empty");
    if(parameters.noLogs)
        launchParam.append("-noLogs");
    // Получаем параметры коннекта к серверу
    if(ui->serverIP->text() != "..." && ui->serverPort->text() != "") { // Если сервер введен, то
        launchParam.append("-connect=" + ui->serverIP->text());
        launchParam.append("-port=" + ui->serverPort->text());
        // Проверяем, если у сервера пароль
        if(ui->serverPassword->text() != "")
            launchParam.append("-password=" + ui->serverPassword->text());
    }
    // Выставляем параметры запуска в соответсвии с приоритетом запуска аддонов
    for(int i = 0; i<listPriorityAddonsDirs.size();i++)
        for(int j = 0; j<ui->addonTree->topLevelItemCount();j++) {
            if(listPriorityAddonsDirs[i] == ui->addonTree->topLevelItem(j)->text(1) &&
               ui->addonTree->topLevelItem(j)->checkState(0) == Qt::Checked)
                launchParam.append("-mod=" + ui->addonTree->topLevelItem(j)->text(2)+"/" + ui->addonTree->topLevelItem(j)->text(1) + ";");
        }

    qDebug() << "Debug-launcher: get launch param - succ";
    // Возвращаем параметры запуска в виде списка строк
    return launchParam;

}

// Обновлении параметров при клике на настройки
void launcher::on_tabWidget_tabBarClicked(int index) {
    // Обновление информации в виджете параметров
    if(index == 2)
        updateInfoParametersInMem();
}

// Слот при котором вызваем оптимизацию настроек
void launcher::on_optimize_clicked() {
    // Вызываем Dx diag
    if(!dxDiagIsRunning) {      // Проверяем, не обрабатывает ли DX diag
        dxDiagIsRunning = true;
        // Запускаем DX Diag
        dx = new QProcess(this);
        connect(dx, SIGNAL(finished(int)), this, SLOT(optimizeSettings()));
        dx->start("dxdiag /x dxdiag.xml");
        qDebug() << "Debug-launcher: Optimize Settings - DXDiag Start";
    }
}

// Слот после получения данных от DX diag
void launcher::optimizeSettings() {

    qDebug() << "Debug-launcher: Optimize Settings - DXDiag finish";

    // Проверяем наличие выходных данных от dxdiag'а
    QFile file("dxdiag.xml");
    if (file.exists()) {

        // Парсим выходную информацию dxdiaga
        XMLParser xml("dxdiag.xml");
        dxdiag diag = xml.getDxdiag();
        //..и удаляем выходные данные
        qDebug() << "Debug-launcher: File removes - " << file.fileName();
        file.remove();

        // Стандартные параметры
        ui->noPause->setChecked(true);
        ui->battleEye->setChecked(true);
        ui->skipIntro->setChecked(true);
        ui->nosplash->setChecked(true);
        ui->noLogs->setChecked(true);
        ui->worldEmpty->setChecked(true);
        ui->priorityLaunch->setCurrentIndex(1);

        // Заполнение информацией основнанной на процессоре
        if(!diag.Processor.isEmpty()) {
            if(diag.Processor.contains("intel", Qt::CaseInsensitive)) {
                ui->check_malloc->setChecked(true);
                ui->malloc->setCurrentText("tbb4malloc_bi");
            } else {
                ui->check_malloc->setChecked(false);
            }
            if(diag.Processor.contains("i3", Qt::CaseInsensitive)) {
                ui->check_exThreads->setChecked(true);
                ui->exThreads->setCurrentIndex(2);
            }
            if(diag.Processor.contains("i5", Qt::CaseInsensitive) || diag.Processor.contains("i7", Qt::CaseInsensitive)) {
                ui->check_exThreads->setChecked(true);
                ui->exThreads->setCurrentIndex(3);
                if(diag.Processor.contains("i7", Qt::CaseInsensitive))
                    ui->enableHT->setChecked(true);
            }
            if(diag.Processor.contains(" CPUs)")) {
                ui->check_cpuCount->setChecked(true);
                ui->cpuCount->setCurrentText(QString(diag.Processor[diag.Processor.indexOf(" CPUs)")-1]));
            }
        }
        // Заполнение информацией на основе данных о ОЗУ
        int ram = diag.Memory.remove("MB RAM").toInt();
        if (ram != 0) {
            ui->check_maxMem->setChecked(true);
                 if(ram >= 2048)
                ui->maxMem->setCurrentText("2048");
            else if(ram >= 1536)
                ui->maxMem->setCurrentText("1536");
            else if(ram >= 1024)
                ui->maxMem->setCurrentText("1024");
            else if(ram >= 512)
                ui->maxMem->setCurrentText("512");
            else if(ram >= 256)
                ui->maxMem->setCurrentText("256");
            else
                ui->check_maxMem->setChecked(false);
        }
        // Заполнение информацией на основе данных о видеокарте
        int Vram = diag.SharedMemory.remove(" MB").toInt();
        if (Vram != 0) {
            ui->check_maxVRAM->setChecked(true);
                 if(Vram >= 2048)
                ui->maxVRAM->setCurrentText("2048");
            else if(Vram >= 1536)
                ui->maxVRAM->setCurrentText("1536");
            else if(Vram >= 1024)
                ui->maxVRAM->setCurrentText("1024");
            else if(Vram >= 512)
                ui->maxVRAM->setCurrentText("512");
            else if(Vram >= 256)
                ui->maxVRAM->setCurrentText("256");
            else
                ui->check_maxVRAM->setChecked(false);
        }
        // Запускать Arma 3 с использованием только DX 9
        if(diag.DirectXVersion.contains("DirectX 9", Qt::CaseInsensitive)
        || diag.OperatingSystem.contains("windows xp", Qt::CaseInsensitive))
            ui->winxp->setChecked(true);
        else
            ui->winxp->setChecked(false);
    }
    // Отключаем тригер и уничтожаем процесс, если он все ещё запущен
    disconnect(dx, SIGNAL(finished(int)), this, SLOT(optimizeSettings()));
    delete dx;
    dx = 0;
    dxDiagIsRunning = false;
    qDebug() << "Debug-launcher: Optimize Settings - succ";

    // Обновление параметров запуска новой информацией
    updateInfoParametersInMem();
}

void launcher::on_AddonsSettings_clicked() {

    QStringList addons;
    for(int i = 0; i < ui->addonTree->topLevelItemCount();i++) {
        addons.append(ui->addonTree->topLevelItem(i)->text(1));
    }

    emit addonsSettingsStart(settings, listDirs, listPriorityAddonsDirs, addons);

}

void launcher::addonsSettingsFinish(QStringList listD, QStringList listPriorityAddonsD) {
    listDirs = listD;
    listPriorityAddonsDirs = listPriorityAddonsD;

    updateInformationInAddonList();
}

void launcher::on_launcherSettings_clicked() {
    emit launcherSettingsStart(settings);
}

void launcher::launcherSettingsFinish(Settings launcherS) {

    settings = launcherS;

    changeStyleSheet(settings.style);

    ui->tabWidget->setDocumentMode(!settings.documentMode);
}

void launcher::changeStyleSheet(int style) {
    if(style == 0) {
        qApp->setStyleSheet("");
        QIcon icon;
        icon.addFile(QStringLiteral(":/myresources/IMG/puzzleSetting37.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->AddonsSettings->setIcon(icon);
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/myresources/IMG/puzzle37.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->tabWidget->setTabIcon(0, icon1);
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/myresources/IMG/wrench10.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->launcherSettings->setIcon(icon2);
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/myresources/IMG/computer monitor1.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->optimize->setIcon(icon3);
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/myresources/IMG/file99.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->pathBrowse->setIcon(icon4);
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/myresources/IMG/settings59.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->tabWidget->setTabIcon(1, icon5);
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/myresources/IMG/add107.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree_add->setIcon(icon6);
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/myresources/IMG/delete85.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree_del->setIcon(icon7);
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/myresources/IMG/statistics13.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree_about->setIcon(icon8);
        QIcon icon9;
        icon9.addFile(QStringLiteral(":/myresources/IMG/refresh57.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree_update->setIcon(icon9);
        QIcon icon10;
        icon10.addFile(QStringLiteral(":/myresources/IMG/favourites4.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->tabWidget->setTabIcon(2, icon10);
        QIcon icon11;
        icon11.addFile(QStringLiteral(":/myresources/IMG/cloud149.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repoAdd->setIcon(icon11);
        QIcon icon12;
        icon12.addFile(QStringLiteral(":/myresources/IMG/cloud152.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repoDel->setIcon(icon12);
        QIcon icon13;
        icon13.addFile(QStringLiteral(":/myresources/IMG/cloud126.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repoEdit->setIcon(icon13);
        QIcon icon14;
        icon14.addFile(QStringLiteral(":/myresources/IMG/cloud198.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repoConnect->setIcon(icon14);
        QIcon icon15;
        icon15.addFile(QStringLiteral(":/myresources/IMG/clouds11.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repositoryList->setTabIcon(0, icon15);
        QIcon icon16;
        icon16.addFile(QStringLiteral(":/myresources/IMG/hard drive.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->checkAddons->setIcon(icon16);
        QIcon icon17;
        icon17.addFile(QStringLiteral(":/myresources/IMG/round60.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->stopUpdater->setIcon(icon17);
        QIcon icon18;
        icon18.addFile(QStringLiteral(":/myresources/IMG/delete82.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->delOtherFiles->setIcon(icon18);
        QIcon icon19;
        icon19.addFile(QStringLiteral(":/myresources/IMG/download61.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->downloadUpdate->setIcon(icon19);
        QIcon icon20;
        icon20.addFile(QStringLiteral(":/myresources/IMG/broken45.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repoDisconnect->setIcon(icon20);
        QIcon icon21;
        icon21.addFile(QStringLiteral(":/myresources/IMG/data104.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repositoryList->setTabIcon(1, icon21);
        QIcon icon22;
        icon22.addFile(QStringLiteral(":/myresources/IMG/cloud133.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->tabWidget->setTabIcon(3, icon22);
        QIcon icon23;
        icon23.addFile(QStringLiteral(":/myresources/IMG/settings59.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree_edit->setIcon(icon23);
        ui->play->setStyleSheet(QLatin1String("QPushButton { border-image: url(:/myresources/arma3.png); }\n"
        "QPushButton::pressed { border-image: url(:/myresources/arma3down.png); }"));
    } else {
        QFile f(":qdarkstyle/style.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream ts(&f);
            qApp->setStyleSheet(ts.readAll());
            f.close();
        }
        QIcon icon;
        icon.addFile(QStringLiteral(":/myresources/IMG/darkstyle/puzzleSetting37.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->AddonsSettings->setIcon(icon);
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/myresources/IMG/darkstyle/puzzle37.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->tabWidget->setTabIcon(0, icon1);
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/myresources/IMG/darkstyle/wrench10.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->launcherSettings->setIcon(icon2);
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/myresources/IMG/darkstyle/computer monitor1.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->optimize->setIcon(icon3);
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/myresources/IMG/darkstyle/file99.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->pathBrowse->setIcon(icon4);
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/myresources/IMG/darkstyle/settings59.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->tabWidget->setTabIcon(1, icon5);
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/myresources/IMG/darkstyle/add107.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree_add->setIcon(icon6);
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/myresources/IMG/darkstyle/delete85.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree_del->setIcon(icon7);
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/myresources/IMG/darkstyle/statistics13.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree_about->setIcon(icon8);
        QIcon icon9;
        icon9.addFile(QStringLiteral(":/myresources/IMG/darkstyle/refresh57.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree_update->setIcon(icon9);
        QIcon icon10;
        icon10.addFile(QStringLiteral(":/myresources/IMG/darkstyle/favourites4.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->tabWidget->setTabIcon(2, icon10);
        QIcon icon11;
        icon11.addFile(QStringLiteral(":/myresources/IMG/darkstyle/cloud149.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repoAdd->setIcon(icon11);
        QIcon icon12;
        icon12.addFile(QStringLiteral(":/myresources/IMG/darkstyle/cloud152.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repoDel->setIcon(icon12);
        QIcon icon13;
        icon13.addFile(QStringLiteral(":/myresources/IMG/darkstyle/cloud126.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repoEdit->setIcon(icon13);
        QIcon icon14;
        icon14.addFile(QStringLiteral(":/myresources/IMG/darkstyle/cloud198.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repoConnect->setIcon(icon14);
        QIcon icon15;
        icon15.addFile(QStringLiteral(":/myresources/IMG/darkstyle/clouds11.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repositoryList->setTabIcon(0, icon15);
        QIcon icon16;
        icon16.addFile(QStringLiteral(":/myresources/IMG/darkstyle/hard drive.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->checkAddons->setIcon(icon16);
        QIcon icon17;
        icon17.addFile(QStringLiteral(":/myresources/IMG/darkstyle/round60.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->stopUpdater->setIcon(icon17);
        QIcon icon18;
        icon18.addFile(QStringLiteral(":/myresources/IMG/darkstyle/delete82.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->delOtherFiles->setIcon(icon18);
        QIcon icon19;
        icon19.addFile(QStringLiteral(":/myresources/IMG/darkstyle/download61.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->downloadUpdate->setIcon(icon19);
        QIcon icon20;
        icon20.addFile(QStringLiteral(":/myresources/IMG/darkstyle/broken45.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repoDisconnect->setIcon(icon20);
        QIcon icon21;
        icon21.addFile(QStringLiteral(":/myresources/IMG/darkstyle/data104.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->repositoryList->setTabIcon(1, icon21);
        QIcon icon22;
        icon22.addFile(QStringLiteral(":/myresources/IMG/darkstyle/cloud133.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->tabWidget->setTabIcon(3, icon22);
        QIcon icon23;
        icon23.addFile(QStringLiteral(":/myresources/IMG/darkstyle/settings59.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree_edit->setIcon(icon23);
        ui->play->setStyleSheet(QLatin1String("QPushButton { border-image: url(:/myresources/IMG/darkstyle/arma3.png); }\n"
        "QPushButton::pressed { border-image: url(:/myresources/IMG/darkstyle/arma3down.png); }"));
        ui->filesTree->setStyleSheet("color: rgb(0, 0, 0);");
    }
}

void launcher::closeEvent(QCloseEvent * event) {

    if(updateAfterClose) {
        UpdateLauncher();
    }
    event->accept();
}

void launcher::on_addonTree_itemClicked(QTreeWidgetItem *item) {

    if(item->checkState(0) ==  Qt::Unchecked)
        item->setCheckState(0, Qt::Checked);
    else
        item->setCheckState(0, Qt::Unchecked);
}
