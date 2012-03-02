#include "xmlsettingseditorwrapper.h"
#include "ui_xmlsettingseditorwrapper.h"
#include <QDebug>
#include "qsettingsmodel.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStringList>
#include <QSettings>


bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map)
{
    QXmlStreamReader xmlReader(&device);
    QStringList elements;


    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        xmlReader.readNext();

        if (xmlReader.isStartElement()) {
            elements.append(xmlReader.name().toString());

            // if there are attributes in the token
            if (!xmlReader.attributes().isEmpty()){
                QString key,key2;

                for(int i = 0; i < elements.size(); i++) {
                    if(i != 0) key += "/";
                    key += elements.at(i);
                }

                foreach(QXmlStreamAttribute elem, xmlReader.attributes()){
                    if (!QString::compare(elem.name().toString(),"name")){
                        key2 = key;
                        key2 += "/";
                        key2 +=elem.name();
                        map[key2] = elem.value().toString();
                    }
                }
            }

        } else if (xmlReader.isEndElement()) {

            // delete last element
            if(!elements.isEmpty()) elements.removeLast();

         // if token contains text content
        } else if (xmlReader.isCharacters() && !xmlReader.isWhitespace()) {
            QString key = "";

            for(int i = 0; i < elements.size(); i++) {
                if(i != 0) key += "/";
                key += elements.at(i);
            }
            map[key] = xmlReader.text().toString();

        }
    }

    // warning on error
    if (xmlReader.hasError()) {
        qWarning() << xmlReader.errorString();
        return false;
    }

    return true;
}

bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map){
    QXmlStreamWriter xmlWriter(&device);

     xmlWriter.setAutoFormatting(true);
     xmlWriter.writeStartDocument();
     xmlWriter.writeStartElement("Settings");

     QStringList prev_elements;
     QSettings::SettingsMap::ConstIterator map_i;

     // Alle Elemente der Map durchlaufen
     for (map_i = map.begin(); map_i != map.end(); map_i++) {

         QStringList elements = map_i.key().split("/");

         int x = 0;
         // Zu schließende Elemente ermitteln
         while(x < prev_elements.size() && elements.at(x) == prev_elements.at(x)) {
             x++;
         }

         // Elemente schließen
         for(int i = prev_elements.size() - 1; i >= x; i--) {
             xmlWriter.writeEndElement();
         }

         // Elemente öffnen
         for (int i = x; i < elements.size(); i++) {
             xmlWriter.writeStartElement(elements.at(i));
         }

         // Wert eintragen
         xmlWriter.writeCharacters(map_i.value().toString());

         prev_elements = elements;
     }

     // Noch offene Elemente schließen
     for(int i = 0; i < prev_elements.size(); i++) {
         xmlWriter.writeEndElement();
     }

     xmlWriter.writeEndElement();
     xmlWriter.writeEndDocument();

     return true;

}


XMLSettingsEditorWrapper::XMLSettingsEditorWrapper(QWidget *parent) : QMainWindow(parent), ui(new Ui::XMLSettingsEditorWrapper)
{
    ui->setupUi(this);
    ui->centralWidget->setLayout(ui->gridLayout);

    bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map);
    bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map);

    QSettings::Format XMLformat = QSettings::registerFormat("xml", readXmlFile, writeXmlFile);

    QSettingsModel * qsm = new QSettingsModel("./../testdata/Config-Demo.xml", XMLformat);
    ui->treeView->setModel(qsm);
}

XMLSettingsEditorWrapper::~XMLSettingsEditorWrapper()
{
    delete ui;
}
