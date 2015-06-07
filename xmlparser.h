#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QDebug>
#include <QFIle>
#include <QXmlStreamReader>
#include <QStack>

#include "cfg.h"

class XMLParser {
    public:
        XMLParser(QString XMLPath);
        ~XMLParser();

        // Получение Распарсенных данных..
        //..DirectX диагностики
        dxdiag                              getDxdiag ();
        //..списка модов
        QStringList                         getMods   ();
        //..о файлах модов
        QList< QMap<QString, QString> >     getAddons ();

        void setPath(QString path);

    private:
        QXmlStreamReader xmlReader;
};

#endif // XMLPARSER_H
