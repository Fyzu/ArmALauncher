#include "launcher.h"
#include "ui_launcher.h"


// Конструктор
updateAddons::updateAddons(QWidget *parent) : QObject(parent) {

    qDebug() << "updateAddons::updateAddons: constructor";

    // Объявление мета типов
    qRegisterMetaType<Repository>("Repository");
    qRegisterMetaType< QList< QMap<QString, QString> > >("QList< QMap<QString, QString> >");
    qRegisterMetaType< QStringList >("QStringList");
    qRegisterMetaType< QList<int> >("QList<int>");
}

// Диструктор
updateAddons::~updateAddons() {

    qDebug() << "updateAddons::~updateAddons: destructor";

    emit finished();
}

// Сохранение временных данных о файлах
void updateAddons::saveTempFileInfo() {

    QFile file(QCoreApplication::applicationDirPath()+"/temp/fileList.temp");
    if(file.open(QIODevice::WriteOnly))  {  //Если файл открыт успешно
        QDataStream out(&file);             //Создаем поток для записи данных в файл

        //В поток
        out << yomaFileInfo << syncFileInfo << addonsPath;

        qDebug() << "updateAddons::saveTempFileInfo: Save File Info in file - succ";
    } else {
        qDebug() << "updateAddons::saveTempFileInfo: Save File Info in file - fail";
    }
}

// Прописываем данные репозитория
void updateAddons::setRepository(Repository repo) {

    qDebug() << "updateAddons::setRepository: repo type -" << repo.type;

    // Получаем параметры репозитория
    repository = repo;
    if(repository.type == 0) {
        QFileInfo info(repository.url);
        pathUrl = repository.url;
        pathUrl.remove(info.fileName());
    } else {
        repository.url.replace("/autoconfig", "/sync");
        pathUrl = repository.url;
        pathUrl.remove(".a3s/sync");
    }

    // Сброс данных
    addonsList.clear();
    modsList.clear();
    addonsFolders.clear();
    addonsPath.clear();
    otherFiles.clear();
    newFiles.clear();
    correctFiles.clear();
    notCorrectFiles.clear();
    downloadFiles.clear();
    downloadTime = 0;
    bytesBefore = 0;
    url = 0;
    manager = 0;
    reply = 0;
    filePath = 0;
    httpRequestAborted = false;
    downloadStart = false;
    downloadAddonsInProgress = false;
    downloadAddonsAbort = false;
    downloadFilesIndex = 0;

    // Считываем список файлов
    //..считывание
    QFile file(QCoreApplication::applicationDirPath()+"/temp/fileList.temp");
    if(file.open(QIODevice::ReadOnly)) {

        //открываем поток ввода
        QDataStream in(&file);

        //Из потока
        in >> yomaFileInfo >> syncFileInfo >> defaultAddonsPath;
        file.close();

        qDebug() << "updateAddons::setRepository: load file info - succ";
    } else {
        yomaFileInfo.clear();
        syncFileInfo.clear();
        qDebug() << "updateAddons::setRepository: load file info - fail";
    }
}

// Старт класса updateAddons
void updateAddons::start() {

    qDebug() << "updateAddons::start: download config";

    // Подключаем слот  к сигналу завершения загрузки, который вызовит метод завершающий запус класса
    connect(this, SIGNAL(downloadFinished(bool)), this, SLOT(startFinished(bool)), Qt::DirectConnection);

    // Скачиваем конфиг
    downloadFile(repository.url, QCoreApplication::applicationDirPath() + "/temp/");
}

// Завершающий этап запуска программы
void updateAddons::startFinished(bool downloadSuccess) {

    qDebug() << "updateAddons::startFinished: download config succ -" << downloadSuccess;

    bool success = false;

    // разрываем связь конца загрузки с Старт - закончен
    disconnect(this, SIGNAL(downloadFinished(bool)), this, SLOT(startFinished(bool)));

    // Получение путей и информации о файле
    if(downloadSuccess) {       // Проверяем, успешно ли загружен файл конфига
        QFileInfo info(repository.url);
        QString path = QCoreApplication::applicationDirPath() + "/temp/";

        // Парсим информацю
        if(repository.type == 0) {
            success = parseYomaInformation(path, info.fileName());
        } else {
            success = parseSyncInformation(path);
        }
    }

    qDebug() << "updateAddons::startFinished: parse config succ -" << success;

    // Операции после успешной
    emit started(repository, addonsList, modsList, success && downloadSuccess, defaultAddonsPath);
}

// Парсинг информации Yoma репозитория
bool updateAddons::parseYomaInformation(QString path, QString fileName) {

    // Распаковка архива
    bool success = unzipArchive(path+fileName, path);

    qDebug() << "updateAddons::parseYomaInformation: config unzip succ -" << success;

    // Парсим информацю
    if(success) { // Если распаковка прошла успешно
        qDebug() << "updateAddons::parseYomaInformation: parsing - start";

        XMLParser parser(path + "Addons.xml");
        addonsList = parser.getAddons();
        parser.setPath(path + "Mods.xml");
        modsList = parser.getMods();

        // Проверяем, успешно ли получили информацию
        if(modsList.size() == 0 || addonsList.size() == 0)
            success = false;
    }

    // Удаление считанных файлов
    QFile::remove(path+"Addons.xml");
    QFile::remove(path+"Mods.xml");
    QFile::remove(path+"Server.xml");
    QFile::remove(path+"Addons.xsd");
    QFile::remove(path+"Mods.xsd");
    QFile::remove(path+"Server.xsd");
    QString name = fileName;
    name.remove(".7z");
    QFileInfo folder(path+name);
    folder.dir().rmdir(folder.absoluteFilePath());

    qDebug() << "updateAddons::parseYomaInformation: parsing succ -" << success;

    return success;
}

// Парсинг информации Arma3Sync репозитория
bool updateAddons::parseSyncInformation(QString path) {

    bool succ = false;

    // Парсим инфу аддон синка
    if(QFile::exists(path+"sync")) {

        qDebug() << "updateAddons::parseSyncInformation: config exists";

        // Запускаем парсер
        QProcess sync;
        sync.start("ArmALauncher-SyncParser.exe 0 \"" + path + "sync\"");
        //..ожидаем завершение парсера
        sync.waitForFinished(-1);
        //..считываем вывод парсера
        QString output = sync.readAll();
        sync.deleteLater();

        qDebug() << "updateAddons::parseSyncInformation: SyncParser output: " << output;

        if(output.contains("Successfully", Qt::CaseInsensitive)) {

            // Читаем вывод парсера
            QStringList list;
            QFile inputfile(path + "sync.txt");
            if (inputfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
               QTextStream in(&inputfile);
               while (!in.atEnd())
                    list.append(in.readLine());
               succ = true;
            }

            inputfile.close();

            // Очищаем данные
            addonsList.clear();
            modsList.clear();

            // Получаем список модов
            int index;
            for(index = 0; index<list.size() && list[index] != " - - sha1 - - ";index++) {
                modsList.append(list[index]);
            }
            // Получаем список файлов и их атрибуты
            index++;
            while(index<list.size()) {
                QMap<QString, QString> file;
                file.insert("Path", list[index]);
                file.insert("Pbo",  list[index+1]);
                file.insert("Sha1", list[index+2]);
                file.insert("Size", list[index+3]);
                addonsList.append(file);
                index+=4;
            }

            // Проверяем успешность парсинга
            if(modsList.size() == 0 && addonsList.size() == 0)
                succ = false;
        }

        // Удаляем считанные файлы
        QFile::remove(path + "sync");
        QFile::remove(path + "sync.txt");
    }

    qDebug() << "updateAddons::parseSyncInformation: parse succ -" << succ;

    return succ;
}

// Получение результатов после старта UI апдейтера
void updateAddons::updaterUIStarted(QStringList folders) {

    addonsFolders = folders;
}

// Дисконект репозитория
void updateAddons::finish() {

    // Обнуление переменных
    downloadTime = 0;
    bytesBefore = 0;
    url.clear();
    manager = 0;
    reply = 0;
    filePath = 0;
    httpRequestAborted = false ;
    downloadStart = false;
    addonsList.clear();
    modsList.clear();
    addonsFolders.clear();
    addonsPath.clear();
    otherFiles.clear();
    newFiles.clear();
    correctFiles.clear();
    notCorrectFiles.clear();
    downloadFiles.clear();
    downloadAddonsInProgress = false;
    downloadAddonsAbort = false;
    pathUrl.clear();
    downloadFilesIndex=0;
    yomaFileInfo.clear();
    syncFileInfo.clear();

    qDebug() << "updateAddons::finish: clear";
}

/*
 * Функции работы с репозиторем
 */
// Проверить аддоны
void updateAddons::checkAddons(QString path) {

    qDebug() << "updateAddons::checkAddons: addons path: " << path;

    // Получаем путь к аддонам
    addonsPath = path;

    // Временные переменные
    QList< QMap<QString, QString> > allFiles;
    QList< QMap<QString, QString> > existsFiles;

    // Очищаем данные, для заполнения свежими данными
    otherFiles.clear();
    newFiles.clear();
    correctFiles.clear();
    notCorrectFiles.clear();

    // Получаем список всех файлов которые находятся в аддонах
    for(int folder = 0; folder<addonsFolders.count(); folder++) {
        getAllFilesInDir(addonsPath+"/"+addonsFolders[folder], allFiles);
    }

    qDebug() << "updateAddons::checkAddons: all files size -" << allFiles.size();

    // Получаем списки файлов которые есть в списке сервера и те которые являются лишними
    for(int i = 0; i < allFiles.size();i++) {
        int j;
        for(j = 0; j < addonsList.size(); j++) {
            if(QString::compare(allFiles[i]["Pbo"],  addonsList[j]["Pbo"], Qt::CaseInsensitive) == 0 &&
               QString::compare(allFiles[i]["Path"], addonsList[j]["Path"], Qt::CaseInsensitive) == 0) {
                existsFiles.append(addonsList[j]);
                break;
            }
        }
        if(j == addonsList.size()                   && !allFiles[i]["Pbo"].contains(".wav") &&
           !allFiles[i]["Pbo"].contains(".dll")     && !allFiles[i]["Pbo"].contains(".txt") &&
           !allFiles[i]["Pbo"].contains(".bikey")   && !allFiles[i]["Pbo"].contains(".bisign") &&
           !allFiles[i]["Pbo"].contains(".cpp")     && !allFiles[i]["Pbo"].contains(".paa") &&
           !allFiles[i]["Pbo"].contains(".exe")     && !allFiles[i]["Pbo"].contains(".bat"))
            // добавляем файл на удаление
            otherFiles.append(allFiles[i]);
    }

    qDebug() << "updateAddons::checkAddons: exists files size -" << existsFiles.size();

    // Получаем список новых файлов
    newFiles = addonsList;
    for(int index = 0; index < existsFiles.size(); index++) {
        newFiles.removeAll(existsFiles[index]);
    }

    // Получаем списки корректных файлов и нет
    if(repository.type == 0)
    // Yoma Addon Sync 2009
        for(int index = 0; index<existsFiles.size(); index++) {
            // Регистрируем прогресс в UI
            emit checkAddonsProgressUI(index, existsFiles);

            // Получаем информацию о файле
            QFileInfo info(addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"]);

            // Проверяем, есть ли файл в заранее собранном файле
            YomaFileInfo temp;
            temp.fileName = addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"];
            int tempIndex = yomaFileInfo.indexOf(temp);


            // Проверяем, схожа ли заранее подготовленая информация о файле с действительностью
            if(tempIndex != -1) {           // Если файл найден
                if(info.lastModified() == yomaFileInfo[tempIndex].fileEditDate && info.size() == yomaFileInfo[tempIndex].fileSize
                  && QString::compare(yomaFileInfo[tempIndex].fileMD5, existsFiles[index]["Md5"], Qt::CaseInsensitive) == 0) {             // Проверяем, действительная ли информация о файле
                    correctFiles.append(existsFiles[index]);
                    continue;
                } else {        // Если информация не действительна
                    yomaFileInfo.removeAt(tempIndex);
                }
            }

            // Получаем MD5 сумму файла
            QString md5 = getChecksum(addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"], QCryptographicHash::Md5);
            // Проверяем, соответсвует ли сумма на сервере, сумме локальной
            if(QString::compare(md5, existsFiles[index]["Md5"], Qt::CaseInsensitive) == 0) {
                YomaFileInfo fileInfo;
                fileInfo.fileName = (addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"]);
                fileInfo.fileMD5 = (existsFiles[index]["Md5"]);
                fileInfo.fileEditDate = (info.lastModified());
                fileInfo.fileSize = (info.size());
                yomaFileInfo.append(fileInfo);
                correctFiles.append(existsFiles[index]);
            } else {
                notCorrectFiles.append(existsFiles[index]);
                notCorrectFiles.last().insert("Md5local", md5.toUpper());
            }
        }
    else
    // Arma3 Sync
        for(int index = 0; index<existsFiles.size(); index++) {
            // Регистрируем прогресс в UI
            emit checkAddonsProgressUI(index, existsFiles);

            // Получаем информацию о файле
            QFileInfo info(addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"]);

            // Проверяем, есть ли файл в заранее собранном файле
            SyncFileInfo temp;
            temp.fileName = addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"];
            int tempIndex = syncFileInfo.indexOf(temp);


            // Проверяем, схожа ли заранее подготовленая информация о файле с действительностью
            if(tempIndex != -1) {           // Если файл найден
                if(info.lastModified() == syncFileInfo[tempIndex].fileEditDate && info.size() == syncFileInfo[tempIndex].fileSize
                  && QString::compare(syncFileInfo[tempIndex].fileSHA1, existsFiles[index]["Sha1"], Qt::CaseInsensitive) == 0) {             // Проверяем, действительная ли информация о файле
                    correctFiles.append(existsFiles[index]);
                    continue;
                } else {        // Если информация не действительна
                    syncFileInfo.removeAt(tempIndex);
                }
            }

            // Получаем MD5 сумму файла
            QString sha1 = getChecksum(addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"], QCryptographicHash::Sha1);
            if(sha1 == "da39a3ee5e6b4b0d3255bfef95601890afd80709")
                sha1 = "0";
            // Проверяем, соответсвует ли сумма на сервере, сумме локальной
            if(QString::compare(sha1, existsFiles[index]["Sha1"], Qt::CaseInsensitive) == 0) {
                SyncFileInfo fileInfo;
                fileInfo.fileName = (addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"]);
                fileInfo.fileSHA1 = (existsFiles[index]["Sha1"].toUpper());
                fileInfo.fileEditDate = (info.lastModified());
                fileInfo.fileSize = (info.size());
                syncFileInfo.append(fileInfo);
                correctFiles.append(existsFiles[index]);
            } else {
                notCorrectFiles.append(existsFiles[index]);
                notCorrectFiles.last().insert("Sha1local", sha1.toUpper());
            }
        }

    qDebug() << "updateAddons::checkAddons: correct files -" << correctFiles.size() << "; not correct files -" << notCorrectFiles.size();

    // Передаем данные UI
    emit checkAddonsFinished(repository.type, otherFiles, newFiles, correctFiles, notCorrectFiles);

    // Сохраняем информацию о файлах
    saveTempFileInfo();
}

// Рекурсивный метод, получения списка всех файлов в папке и подпапках
void updateAddons::getAllFilesInDir(const QString path, QList< QMap<QString, QString> > &allFiles) {

    // Получаем файлы в текущей папке path
    QStringList files = QDir(path).entryList(QDir::Files);
    for(int index = 0; index<files.size();index++) {
        QMap<QString, QString > file;
        QString addonPath = path;
        file.insert("Path", addonPath.remove(addonsPath+"/").replace('/', '\\'));
        file.insert("Pbo", files[index]);
        allFiles.append(file);
    }

    // Получаем папки в текущей позиции Path и очищаем от мусора
    QStringList folders = QDir(path).entryList(QDir::Dirs);
    for(int j=0;j<folders.size();j++) {
        if(folders.at(j) == "." || folders.at(j) == "..") {
            folders.removeAt(j);
            j--;
        }
     }

    // Вызываем этот же метод, на все папки в текущей позиции path
    for(int index = 0;index<folders.size();index++) {
        getAllFilesInDir(path+"/"+folders[index], allFiles);
    }
}

// Скачать/обновить аддоны
void updateAddons::downloadUpdate(QList<int> fileID) {

    qDebug() << "updateAddons::downloadUpdate: start";

    // Собираем список скачиваемых файлов
    downloadFiles.clear();
    for(int id = 0;id<fileID.size();id++) {
        if(fileID[id] >= notCorrectFiles.size()) {
            downloadFiles.append(newFiles[fileID[id]-notCorrectFiles.size()]);
        } else {
            downloadFiles.append(notCorrectFiles[fileID[id]]);
        }
    }

    // Если нет файлов для скачивания
    if(downloadFiles.size() == 0) {
        downloadAddonsFinish();
        emit downloadAddonsFinished(true);
        qWarning() << "updateAddons::downloadUpdate: downloadFiles = 0";
        return;
    }

    // Переменные загрузки
    downloadFilesIndex = 0;
    downloadAddonsInProgress = true;
    downloadAddonsAbort = false;

    // связь, конец загрузки аддонов
    connect(this, SIGNAL(downloadAddonsFinished(bool)),
            this, SLOT  (downloadAddonsFinish()));
    // Запускаем первый этап загрузки
    downloadAddonStart();
}

// Загрузка файла - первая стадия загрузки
void updateAddons::downloadAddonStart() {

    qDebug() << "updateAddons::downloadAddonStart: download file: " << downloadFiles[downloadFilesIndex];


    // связываем след этап, с концом загрузки файла
    connect(this, SIGNAL(downloadFinished(bool)), this, SLOT(downloadAddonFinish(bool)), Qt::QueuedConnection);

    //..загружаем файл
    if(repository.type == 0)
        downloadFile(pathUrl + downloadFiles[downloadFilesIndex]["Url"], QCoreApplication::applicationDirPath() + "/temp/");
    else
        downloadFile(pathUrl + downloadFiles[downloadFilesIndex]["Path"] + "/" + downloadFiles[downloadFilesIndex]["Pbo"],
                     addonsPath + "/" + downloadFiles[downloadFilesIndex]["Path"] + "/");

    // Даем сигнал UI, что начата загрузка downloadFilesIndex файла
    emit downloadAddonStarted(downloadFiles, downloadFilesIndex);
}

// Загрузка файла - финальная стадия загрузки
void updateAddons::downloadAddonFinish(bool downloadSuccess) {

    qDebug() << "updateAddons::downloadAddonFinish: download addon succ -" << downloadSuccess;

    bool success = downloadSuccess;

    // Отвязываем конец загрузки от последнего этапа загрузки
    disconnect(this, SIGNAL(downloadFinished(bool)), this, SLOT(downloadAddonFinish(bool)));

    // Распаковка скаченного архива
    if(success) {
        if(repository.type == 0) { // Если загрузка завершена успешно
            // Получение путей и информации о файле
            QFileInfo info(downloadFiles[downloadFilesIndex]["Url"]);
            success = unzipArchive(QCoreApplication::applicationDirPath() + "/temp/" + info.fileName(),
                      addonsPath + "/" + downloadFiles[downloadFilesIndex++]["Path"]);
        } else {
            downloadFilesIndex++;
        }
    }
    // Выбор решения по результатам загрузки
    //..если загрузка завершилась не успешно или она остановлена
    if((!success || downloadAddonsAbort) && downloadFilesIndex != downloadFiles.size()) {
        qDebug() << "updateAddons::downloadAddonFinish: abort download addons";
        downloadAddonsFinish();
        emit downloadAddonsFinished(false);
    }
    //..если загрузка завершена успешно
    else if(success && downloadFilesIndex == downloadFiles.size()) {
        qDebug() << "updateAddons::downloadAddonFinish: download addons finish";
        downloadAddonsFinish();
        emit downloadAddonsFinished(true);
    //..иначе, файл загружен успешно
    } else if(success) {
        // Загрузка следующего файла
        downloadAddonStart();
    }
}

// Завершение загрузки файлов
void updateAddons::downloadAddonsFinish() {

    qDebug() << "updateAddons::downloadAddonsFinish: finished download";

    downloadAddonsInProgress = false;

    disconnect(this, SIGNAL(downloadAddonsFinished(bool)),
               this, SLOT  (downloadAddonsFinish()));
}

// Удалить лишние файлы
void updateAddons::delOtherFiles() {

    qDebug() << "updateAddons::delOtherFiles: start";

    emit deleteOtherFiles(otherFiles, addonsPath);
}

// Остановить обновление аддонов
void updateAddons::stopUpdater() {

    qDebug() << "updateAddons::stopUpdater: Download stop";

    downloadAddonsAbort = true;

    // Останавливаем загрузку
    if(reply) { // Если процесс загрузки идет

        // Заканчиваем загрузку
        httpRequestAborted = true;
        // Отключаем сигнал и вызвапем метод вручную
        disconnect(reply, SIGNAL(finished()),
                   this,  SLOT  (httpDownloadFinished()));
        disconnect(reply, SIGNAL(readyRead()),
                   this,  SLOT  (httpReadyRead()));
        disconnect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                   this,  SLOT  (updateDownloadProgress(qint64,qint64)));
        //.. регистрируем завершение загрузки
        reply->abort();
        httpDownloadFinished();
    }
}

// Метод загрузки файла по Урл в нужную директорию
void updateAddons::downloadFile(QString fileUrl, QString path) {

    // Создаем менеджр сети
    manager = new QNetworkAccessManager;

    // Получаем url файла
    url.setUrl(fileUrl.replace('\\', '/'));

    // Получаем имя файла и проверяем его корректность
    QDir().mkpath(path);
    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();
    if (fileName.isEmpty()) {
        qWarning() << "updateAddons::downloadFile: file name - empty";
        return;
    }

    // Прописываем полный путь файл
    filePath = new QFile(path+fileName);
    qDebug() << "updateAddons::downloadFile: download in -" << path;
    if (filePath->exists()) {
        qDebug() << "updateAddons::downloadFile: file exists -" << filePath->fileName() << " - remove";
        filePath->remove();
    }
    if (!filePath->open(QIODevice::WriteOnly)) {
        qWarning() << "updateAddons::downloadFile: file open - fail";
        delete filePath;
        filePath = 0;
        return;
    }

    // Прописываем информацию в наш processBar_current
    httpRequestAborted = false;
    downloadStart = false;

    // Подготавливаемся считать скорость загрузки
    downloadTime=QDateTime::currentMSecsSinceEpoch();
    bytesBefore = 0;
    triger = 4;

    // Запускаем запрос загрузки
    startRequest(url);

    // Передаем сигнал UI для обновление данных
    emit downloadFile_UI(fileName);
}

// Запрос загрузки и подключение сигналов
void updateAddons::startRequest(QUrl url) {

    qDebug() << "updateAddons::startRequest: url -" << url.fileName();

    // Посылаем запрос по URL
    reply = manager->get(QNetworkRequest(url));

    //..и готовимся считывать данные по сигналу readyRead
    connect(reply, SIGNAL(readyRead()),
            this,  SLOT  (httpReadyRead()), Qt::DirectConnection);
    //..так же, регистрируем прогресс загрузки
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this,  SLOT  (updateDownloadProgress(qint64,qint64)), Qt::DirectConnection);
    //..а по завершению загрузки, взываем метод httpDownloadFinished()
    connect(reply, SIGNAL(finished()),
            this,  SLOT  (httpDownloadFinished()), Qt::DirectConnection);

}

// Метод вызывается когда QNetworkReply готов к считыанию данных
void updateAddons::httpReadyRead() {

    // Считываем входные данные в файл
    if (filePath)
        filePath->write(reply->readAll());
}

// Метод обновления данных загрузки
void updateAddons::updateDownloadProgress(qint64 bytesRead, qint64 totalBytes) {

    // Обновляем данные загрузки
    if (!httpRequestAborted) {
        downloadStart = true;
        QString speed;
        if(triger == 4) {
            triger = 0;
            // Расчитываем скорость загрузки в кб/сек
            qint64 time = QDateTime::currentMSecsSinceEpoch();
            unsigned int bytes = static_cast<unsigned int>((bytesRead - bytesBefore));
            unsigned int timeRes = static_cast<unsigned int>((time - downloadTime));
            downloadTime = time;
            bytesBefore = bytesRead;
            speed = QString::number((int) (( bytes * 1000) /( (timeRes+1) * 1024))) + " КБ/сек";
        } else
            triger++;

        // Передаем сигнал UI для обновление данных
        emit updateDownloadProgress_UI(bytesRead, totalBytes, speed);
    }
}

// Метод вызвается когда загрузка завершена
void updateAddons::httpDownloadFinished() {

    // Если нажали кнопку "Остановить загрузку"
    if (httpRequestAborted) {
        qDebug() << "updateAddons::httpDownloadFinished: download aborted";

        // Удаляем недокаченный файл
        if (filePath) {
            filePath->close();
            filePath->remove();
            filePath->deleteLater();
            filePath = 0;
        }
        // Собираем мусор
        manager->deleteLater();
        manager = 0;
        reply->deleteLater();
        reply = 0;

        // Передаем сигнал UI для обновление данных
        emit downloadFinished(false);
        return;
    }

    // Загрузка прошла успешно
    filePath->flush();
    filePath->close();
    // Проверка возникали ли проблемы в загрузке
    if (reply->error()) {
        filePath->remove();
        qDebug() << "updateAddons::httpDownloadFinished: download failed: " << reply->errorString();
    } else
       qDebug() << "updateAddons::httpDownloadFinished: download succ -" << filePath->exists();

    // Собираем мусор
    reply->deleteLater();
    reply = 0;
    filePath->deleteLater();
    filePath = 0;
    manager->deleteLater();
    manager = 0;

    // Передаем сигнал UI для обновление данных
    emit downloadFinished(true);
}

// Получаем проверочную сумму файла по пути
QString updateAddons::getChecksum(QString filePath, QCryptographicHash::Algorithm alg) {

    QFile file(filePath);
    QCryptographicHash crypto(alg);
    if(file.open(QIODevice::ReadOnly)) {
        while(!file.atEnd()){
          crypto.addData(file.read(1024 * 32));
        }
        file.close();
        return crypto.result().toHex().constData();
    }
    return "";
}

// Распокавать архив в указанное место
bool updateAddons::unzipArchive(QString archive, QString extracting) {

    if(!archive.isEmpty() && !extracting.isEmpty()) {

        qDebug() << "updateAddons::unzipArchive: unzip -" << archive;

        // Оповещаем пользователя, что распаковка началась
        QFileInfo info(archive);
        if(QFile::exists(archive))
            emit unzipStart(info.fileName());
        else {
            qWarning() << "updateAddons::unzipArchive: unzip fail - archive not found";
            return false;
        }
        // Распаковываем файли используя консольную версию 7z
        // e - параметр распковки, "путь к архиву",  -o"путь куда",  -mmt многопоточность, -y соглашаться на все
        QProcess unzip;
        unzip.start("\"" + QCoreApplication::applicationDirPath() + "/" + "7z.exe\" e \"" + archive + "\" -o\"" + extracting + "\" -mmt -y");
        unzip.waitForFinished(-1);

        // Получаем вывод 7z, для удобного дебага
        QString output = unzip.readAll();
        qDebug() << "updateAddons::unzipArchive: 7z output: " << output;
        unzip.deleteLater();
        bool succ = output.contains("Everything is Ok", Qt::CaseInsensitive);

        // Завершение распаковки
        //..удаление архива
        QFile::remove(archive);
        //..оповещаем пользователя, что распаковка удалась
        emit unzipFinished(info.fileName());

        qDebug() << "updateAddons::unzipArchive: unzip succ -" << succ;
        return succ;
    } else
        qWarning() << "updateAddons::unzipArchive: unzip fail - archive or extracting path - empty";

    return false;
}
