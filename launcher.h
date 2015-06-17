#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFile>
#include <QTime>
#include <QFileDialog>
#include <QProcess>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QCryptographicHash>
#include <QCloseEvent>
#include <QListWidget>
#include <QTextCodec>
#include <QLibrary>
#include <QDebug>
#include <QThread>

#include <windows.h>
#include <tchar.h>

#include "launchersettings.h"
#include "addonssettings.h"
#include "version.h"

class launcher : public QMainWindow
{
    Q_OBJECT

public:
    explicit launcher(QWidget *parent = 0);
    ~launcher();

signals:
    // Сигналы отправки данных в формы
    void sendData(favServer server, QList<addon> addonsList, bool newServer, QStringList names);
    void addonsSettingsStart(Settings settings, QStringList listD, QStringList listPriorityAddonsD, QStringList addons);
    void launcherSettingsStart(Settings launcherS);
    void repoEditStart(Repository repo, int currentRow, bool newRepo);
    void newVersion(Settings settings, QString version);

    // Сигналы интерфейса апдейтера
    void updaterUIStarted(QStringList addonsFolders);
    void stopUpdater();
    void repositoryDisconnect();
    void updaterCheckAddons(QString addonsPath);
    void downloadUpdateStart(QList<int> fileID);
    void showUpdater();

private slots:
    // Слоты настроек игры..
    //..для получения исполняемого файла игры
    void on_pathBrowse_clicked();
    //..для оптимизации настроек игры
    void on_optimize_clicked();
    //..для установок настроек после dxdiag'а
    void optimizeSettings();

    // Слоты избраных серверов
    void on_serversTree_add_clicked();
    void on_serversTree_del_clicked();
    void on_serversTree_update_clicked();
    void Send();                            // Слот отправки данных в новую форму по клику на кнопку
    void recieveData(favServer server, bool newServer);     // Получение данных из формы

    // Слоты основой части интерфейса
    void on_selectServer_currentIndexChanged(int index);
    void on_play_clicked();
    void on_tabWidget_tabBarClicked(int index);

    // Слоты выбора репозитория
    //..кнопки
    void on_repoAdd_clicked();
    void on_repoDel_clicked();
    void on_repoEdit_clicked();
    void on_repoConnect_clicked();
    void repoEditFinish(Repository repo, int currentRow, bool newRepo);
    //..изменения состояний
    void on_repoList_currentRowChanged(int currentRow);

    // Слоты интерфейса апдейтера
    //..слот ошибок
    void errorUI(int type, QString msg);
    //..слоты основных функций апдейтера
    void updaterStarted(const Repository repository, const QList< QMap<QString, QString> > addonsList, const QStringList modsList, bool success, QString defaultAddonsPath);
    void updaterFinished();
    //..слот отключения репозитория
    void on_repoDisconnect_clicked();
    //..слоты скачивания файлов
    void downloadFile_UI(QString fileName);
    void updateDownloadProgress_UI(qint64 bytesRead, qint64 totalBytes, QString speed);
    void downloadFinished(bool success);
    //..слоты unzipa
    void unzipStart(QString fileName);
    void unzipFinished(QString fileName);
    //..слоты работы с функционалом апдейтера
    //..проверка файлов
    void updaterCheckAddonsUI();
    void checkAddonsFinishedUI(int type, const QList< QMap<QString, QString> > otherFiles, const QList< QMap<QString, QString> > newFiles,
                               const QList< QMap<QString, QString> > correctFiles, const QList< QMap<QString, QString> > notCorrectFiles);
    void checkAddonsProgressUI(int index, const QList< QMap<QString, QString> > existsFiles);
    //..начала загрузки аддонов
    void downloadUpdateStartUI();
    void downloadAddonStartedUI(const QList< QMap<QString, QString> > downloadFiles, int index);
    void stopUpdaterUI();
    void downloadAddonsFinishUI(bool success);
    //..слоты изменения состояния элементов апдейтера
    void addonsTreeCheck(QTreeWidgetItem *item);
    void on_checksum_stateChanged(int stateCheck);
    //..слот завершения удаления лишних файлов
    void deleteOtherFilesFinish();
    //..слоты настроек аддонов
    void on_AddonsSettings_clicked();
    void addonsSettingsFinish(QStringList listD, QStringList listPriorityAddonsD);
    //..слоты настроек лаунчера
    void on_launcherSettings_clicked();
    void launcherSettingsFinish(Settings launcherS);
    //..слоты обновления версии
    void downloadVersionFinished(QNetworkReply *reply);
    void launcherUpdateResult(int result);
    // Слот активирования аддона по клику
    void on_addonTree_itemClicked(QTreeWidgetItem *item);

private:
    Ui::launcher *ui;
    // Указатели на классы доп. форм
    serverEdit *edit;
    addonsSettings *AddonsSettings;
    launcherSettings *LauncherSettings;
    deleteOtherFiles *delOtherFiles;
    repoEdit *repositoryEdit;
    launcherUpdate *LauncherUpdate;

    // UPDATER
    updateAddons *updater;
    QThread* thread;
    // Данные апдейтера
    QStringList modsL;
    QList< QMap<QString, QString> > otherFiles;
    QList< QMap<QString, QString> > newFiles;
    QList< QMap<QString, QString> > correctFiles;
    QList< QMap<QString, QString> > notCorrectFiles;
    bool stopUpdaterInProcess;

    // Работа с dll
    //..получение длл
    QLibrary library;
    //..объявление функции dll
    typedef unsigned char* (*ExchangeDataWithServer)(const char* host, int port, const int timeout, const unsigned char* str, const int len, int &ping);
    ExchangeDataWithServer exchangeDataWithServer;

    // Временные переменные
    QProcess *dx;
    QNetworkAccessManager *manager;
    bool updateAfterClose;
    bool updateInfoInWidget;        // Тригер обновления данных (дабы не задеть сигналы смены row)
    bool dxDiagIsRunning;           // Тригер запущенного dxdiag'а
    int selectServer;               // Выбранный сервер в списке
    int addonTreeRow;
    qlonglong totalSize;

    // Пременные для контроля апдейтера
    bool updaterIsRunning;
    bool checkAddonsIsRunning;
    bool downloadAddonsIsRunning;

    // Функции обновления данных
    void updateInformationInWidget();       // Обновление информации в виджете
    void updateInformationInMem();          // Обновление информации в памяти
    void updateInformationInCfg();          // Обновление информации в конфиге
    void updateInformationInAddonList();    // Обновление информации в списке аддонов
    void updateInfoParametersInMem();       // Обновление информации параметров в памяти
    void updateInfoParametersInWidget();    // Обновление информации параметров в виджете
    void updateInfoInRunParametersWidget(); // Обновениие информации в виджете запускаемых параметров

    // Получение списка параметров запуска
    QStringList getLaunchParam();
    // Проверка, содержит ли Лист Виджет строку str
    bool ListWidgetContains(QListWidget * widget, QString str);
    // Получше хэндла по titleName
    HANDLE getHandle (QString titleName, bool wait);
    // Получение версии файла
    QString getFileVersion(QString path);
    // Выделение подстрок в байтовом массиве
    QByteArray GetNextPart(int &pos, BYTE* response);
    // Применение другого стиля
    void changeStyleSheet(int style);
    // Проверка версий лаунчера
    void checkForUpdates();
    // Запуск обновления лаунчера
    void UpdateLauncher();

    // Переменные конфига
    QString     pathFolder;                     // Путь к исполняемому файлу игры
    QStringList listDirs;                       // Список путей к аддонам
    QStringList listPriorityAddonsDirs;         // Список папок приоритетных аддонов
    QString DocumentsLocation;                  // Путь к документам системы
    QList<addon> addonsList;                    // Списон структуры с параметрами аддона
    QList<favServer> favServers;                // Список избранных серверов
    param parameters;                           // Настройки параметров
    QStringList checkAddons;                    // Список активированных аддонов
    QSize widgetSize;                           // Size главного окна
    QList<Repository> repositories;
    Settings settings;

protected:
    // Действия при закрытии окна
    virtual void closeEvent( QCloseEvent * event );
};

#endif // LAUNCHER_H
