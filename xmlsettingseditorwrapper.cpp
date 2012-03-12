#include "xmlsettingseditorwrapper.h"

XMLSettingsEditorWrapper::XMLSettingsEditorWrapper(QFile * file, QWidget *parent) : QSplitter(parent)
{
    init(); //initialising members (defined in header)
    sourceFile = file;

    //creating new data model
    model = new XmlTreeModel(*sourceFile,this);

    QTreeView * tree = new QTreeView(this);//TreeView of XML donfig data
    tree->setModel(model); //connecting custom XMLTreeModel

    QWidget * vbwidg = new QWidget(this); //right-side-description/edit-widget
    QVBoxLayout * vbox = new QVBoxLayout(vbwidg); //->vertical layouted

    QLabel * optionName = new QLabel(tr("Config"),vbwidg); //view for selected item name
    QPlainTextEdit * optionDescription = new QPlainTextEdit(tr("Please select an item in the <-TreeView!"),vbwidg); //view for selected item description
    optionDescription->setReadOnly(true); //for now description does need to be changed (no associated functionality implemented)
    attrBox = new QGridLayout(vbwidg); //layout to dynamically view attributes

    QPushButton * saveButton = new QPushButton(tr("save"),vbwidg);
    QObject::connect(saveButton,SIGNAL(clicked()),this,SLOT(saveChanges()));


    //puting everything together
    vbox->addWidget(optionName,1);
    vbox->addWidget(optionDescription,7);
    vbox->addLayout(attrBox,1);
    vbox->addWidget(saveButton,1);

    vbwidg->setLayout(vbox);

    this->addWidget(tree);
    this->addWidget(vbwidg);
    //arange initial dimensions
    this->setMinimumSize(800,600);
    this->setStretchFactor(0,1);
    this->setStretchFactor(1,3);

    //selection event in QTreeView
    QObject::connect(tree, SIGNAL(activated(QModelIndex)), this, SLOT(optionSelected(QModelIndex))); //trigger this->show attributes on doubleClick and Enter (platform-dependant)
    QObject::connect(tree, SIGNAL(clicked(QModelIndex)), this, SLOT(optionSelected(QModelIndex))); //trigger this->show attributes on singleClick
    QObject::connect(this, SIGNAL(labelChanged(QString)), optionName, SLOT(setText(QString))); //write name to label view
    QObject::connect(this, SIGNAL(decriptionChanged(QString)), optionDescription, SLOT(setPlainText(QString))); //write description to view
}

void XMLSettingsEditorWrapper::optionSelected(QModelIndex index){ //SLOT
    emit labelChanged(index.data(Qt::DisplayRole).toString());
    emit decriptionChanged(model->description(index));
    QList<TreeItem::Attribute> attr = model->attributes(index);
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
        QList<TreeItem::Attribute>::const_iterator i;
        for( i = attr.constBegin(); i != attr.constEnd(); i++){
            c++;
            a = new AttributWidget(index, (*i).key, (*i).value, attrBox->parentWidget());
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

void XMLSettingsEditorWrapper::datumChanged(QModelIndex index, QString key, QString value)
{
    model->changeAttribute(index,key,value);
}

void XMLSettingsEditorWrapper::saveChanges() //SLOT
{
    sourceFile->setFileName(sourceFile->fileName().append(".txt"));
    model->save(*sourceFile);
}
