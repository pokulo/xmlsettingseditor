#include "xmlsettingseditorwrapper.h"

#include "xmltreemodel.h"

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStringList>
#include <QSettings>
#include <QFile>


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
                QString key,key2 = "";

                for(int i = 0; i < elements.size(); i++) {
                    if(i != 0) key += "/";
                    key += elements.at(i);
                }

                foreach(QXmlStreamAttribute elem, xmlReader.attributes()){
                    if (elem.name().toString().compare("name",Qt::CaseInsensitive) != 0){
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
//            map[key] = xmlReader.text().toString();

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

void ModelPipe::optionSelected(QModelIndex index){
    emit ModelPipe::labelChanged(index.data(Qt::EditRole).toString());
    emit ModelPipe::decriptionChanged("fake");
}

XMLSettingsEditorWrapper::XMLSettingsEditorWrapper(QWidget *parent) : QWidget(parent)
{

//    bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map);
//    bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map);

//    QSettings::Format XMLformat = QSettings::registerFormat("xml", readXmlFile, writeXmlFile);

//    QSettingsModel * qsm = new QSettingsModel("./../testdata/Config-Demo.xml", XMLformat);


    QFile file("./../testdata/Config-Demo.xml");
    XmlTreeModel * qsm = new XmlTreeModel(file,this);

    QTreeView * tree = new QTreeView(this);
    tree->setModel(qsm);

    QLabel * optionName = new QLabel("label",this);
    QPlainTextEdit * optionDescription = new QPlainTextEdit(this);

    QHBoxLayout * hbox = new QHBoxLayout(this);
    QVBoxLayout * vbox = new QVBoxLayout(this);

    hbox->addWidget(tree);
    hbox->addLayout(vbox);

    vbox->addWidget(optionName);
    vbox->addWidget(optionDescription);

    this->setLayout(hbox);
    this->setMinimumSize(800,600);

//    ModelPipe * pipe = new ModelPipe(this);

//    QObject::connect(tree, SIGNAL(clicked(QModelIndex)), pipe, SLOT(optionSelected(QModelIndex)));
//    QObject::connect(pipe, SIGNAL(labelChanged(QString)), optionName, SLOT(setText(QString)));
//    QObject::connect(pipe, SIGNAL(decriptionChanged(QString)), optionDescription, SLOT(setPlainText(QString)));
}

XMLSettingsEditorWrapper::~XMLSettingsEditorWrapper()
{
}

