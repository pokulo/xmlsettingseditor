#include "xmlsettingseditorwrapper.h"

#include <QFile>

void XMLSettingsEditorWrapper::optionSelected(QModelIndex index){
    emit labelChanged(index.data(Qt::DisplayRole).toString());
    emit decriptionChanged(model->description(index));
    QMap<QString,QString> attr = model->attributes(index);
    AttributWidget * a;
    while (!attrList->isEmpty()){
        a = attrList->takeFirst();
        attrBox->removeWidget(a);
        a->close();
    }
    r = -1;
    c = -1;
    if (!attr.isEmpty()){
        r++;
        foreach(const QString &key, attr.keys()){
            c++;
            a = new AttributWidget(index, key, attr.value(key), attrBox->parentWidget());
            attrList->append(a);
            attrBox->addWidget(a,r,c);
            QObject::connect(a,SIGNAL(attributeChanged(QModelIndex,QString,QString)),this,SLOT(datumChanged(QModelIndex,QString,QString)));
            if(c>1){
                c = -1;
                r++;
            }
        }
    }
}

XMLSettingsEditorWrapper::XMLSettingsEditorWrapper(QSplitter *parent) : QSplitter(parent)
{
    init();
    //ToDo maybe file selection dialog or parameter for filename
    QFile file("./../testdata/Config-Demo.xml");
    //creating new data model
    model = new XmlTreeModel(file,this);

    QTreeView * tree = new QTreeView(this);
    tree->setModel(model);

    QWidget * vbwidg = new QWidget(this);
    QVBoxLayout * vbox = new QVBoxLayout(vbwidg);

    QLabel * optionName = new QLabel("label",vbwidg);
    QPlainTextEdit * optionDescription = new QPlainTextEdit(vbwidg);
    optionDescription->setReadOnly(true);
    attrBox = new QGridLayout(vbwidg);

    vbox->addWidget(optionName,1);
    vbox->addWidget(optionDescription,7);
    vbox->addLayout(attrBox,5);

    vbwidg->setLayout(vbox);

    this->setMinimumSize(800,600);
    this->addWidget(tree);
    this->addWidget(vbwidg);
    this->setStretchFactor(0,1);
    this->setStretchFactor(1,3);

    //selection event in QTreeView
    QObject::connect(tree, SIGNAL(clicked(QModelIndex)), this, SLOT(optionSelected(QModelIndex)));
    QObject::connect(this, SIGNAL(labelChanged(QString)), optionName, SLOT(setText(QString)));
    QObject::connect(this, SIGNAL(decriptionChanged(QString)), optionDescription, SLOT(setPlainText(QString)));
}

XMLSettingsEditorWrapper::~XMLSettingsEditorWrapper()
{
}

void XMLSettingsEditorWrapper::datumChanged(QModelIndex index, QString key, QString value)
{
   model->changeAttribute(index,key,value);
}
