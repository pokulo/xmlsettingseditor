#include "xmlsettingseditorwrapper.h"

#include <QFile>

ModelPipe::ModelPipe(XmlTreeModel * model,QObject *parent) : QObject(parent){
    this->model = model;
}

void ModelPipe::optionSelected(QModelIndex index){
    emit ModelPipe::labelChanged(index.data(Qt::DisplayRole).toString());
    emit ModelPipe::decriptionChanged(model->description(index));
}

XMLSettingsEditorWrapper::XMLSettingsEditorWrapper(QWidget *parent) : QWidget(parent)
{
    //ToDo maybe file selection dialog or parameter for filename
    QFile file("./../testdata/Config-Demo.xml");
    //creating new
    XmlTreeModel * qsm = new XmlTreeModel(file,this);

    QTreeView * tree = new QTreeView(this);
    tree->setModel(qsm);

    QLabel * optionName = new QLabel("label",this);
    QPlainTextEdit * optionDescription = new QPlainTextEdit(this);
    optionDescription->setEnabled(false);

    QHBoxLayout * hbox = new QHBoxLayout(this);
    QVBoxLayout * vbox = new QVBoxLayout(this);

    hbox->addWidget(tree);
    hbox->addLayout(vbox);

    vbox->addWidget(optionName);
    vbox->addWidget(optionDescription);

    setLayout(hbox);
    setMinimumSize(800,600);

    ModelPipe * pipe = new ModelPipe(qsm, this);

    QObject::connect(tree, SIGNAL(clicked(QModelIndex)), pipe, SLOT(optionSelected(QModelIndex)));
    QObject::connect(pipe, SIGNAL(labelChanged(QString)), optionName, SLOT(setText(QString)));
    QObject::connect(pipe, SIGNAL(decriptionChanged(QString)), optionDescription, SLOT(setPlainText(QString)));
}

XMLSettingsEditorWrapper::~XMLSettingsEditorWrapper()
{
}

