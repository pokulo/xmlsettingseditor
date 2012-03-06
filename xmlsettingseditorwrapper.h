#ifndef XMLSETTINGSEDITORWRAPPER_H
#define XMLSETTINGSEDITORWRAPPER_H

#include <QMainWindow>
#include <QModelIndex>
#include <QSettings>
#include <QTreeView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "qsettingsmodel.h"

class XMLSettingsEditorWrapper : public QWidget
{
    Q_OBJECT

public:
    explicit XMLSettingsEditorWrapper(QWidget *parent = 0);
    ~XMLSettingsEditorWrapper();

};




class ModelPipe : public QObject
{
    Q_OBJECT
public:
    explicit ModelPipe(QObject *parent) : QObject(parent){}
    virtual ~ModelPipe() {}
public slots:
    void optionSelected(QModelIndex);
signals:
    void labelChanged(QString);
    void decriptionChanged(QString);
};


#endif // XMLSETTINGSEDITORWRAPPER_H
