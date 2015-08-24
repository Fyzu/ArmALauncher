#include "launcher.h"
#include <QApplication>

#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QTextStream>
#include <QTranslator>
#include "version.h"

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QFile log(qApp->applicationDirPath() + "/log.txt");
    if(log.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream TextStream(&log);
        QString currentDateTime = "[" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") + context.file + "]";
        switch (type) {
        case QtInfoMsg:     TextStream << QString("%1: %2").arg(currentDateTime).arg(msg)<< endl; break;
        case QtDebugMsg:    TextStream << QString("Debug%1: %2").arg(currentDateTime).arg(msg)<< endl; break;
        case QtWarningMsg:  TextStream << QString("Warning%1: %2").arg(currentDateTime).arg(msg)<< endl; break;
        case QtCriticalMsg: TextStream << QString("Critical%1: %2").arg(currentDateTime).arg(msg)<< endl; break;
        case QtFatalMsg:    TextStream << QString("Fatal%1: %2").arg(currentDateTime).arg(msg)<< endl; abort();
        }log.flush();
    } log.close();
}

int main(int argc, char *argv[]) {

    // Записываем данные в лог, есл прописан парметр "log"
    for (int i = 1; i < argc; ++i)
            if (!qstrcmp(argv[i], "-log"))
                qInstallMessageHandler(messageOutput);

    QApplication a(argc, argv);

    qInfo() << "Launcher [" << VER_FILEVERSION_STR << "] - start";

    /*
    QTranslator translator;
    // Применение перевода UI
    if(!QLocale::system().name().contains("ru")) { // Если система не русская
        qInfo() << "Launcher lang - eu";
        translator.load(QString(":/eu/launcher_eu.qm"));
        a.installTranslator(&translator);
    }*/

    qInfo() << "System locale - " << QLocale::system().name();
    launcher w;

    //Отобразить виджет
    w.show();

    int execCode = a.exec();
    qInfo() << "Launcher - finish - execCode - " << execCode;
    return execCode;
}
