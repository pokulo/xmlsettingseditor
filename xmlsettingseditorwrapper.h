#ifndef XMLSETTINGSEDITORWRAPPER_H
#define XMLSETTINGSEDITORWRAPPER_H

#include <QMainWindow>
#include <QModelIndex>
#include <QTreeView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <xmltreemodel.h>


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
    ModelPipe(XmlTreeModel * model,QObject *parent);
    virtual ~ModelPipe() {}
private:
    XmlTreeModel * model;
public slots:
    void optionSelected(QModelIndex);
signals:
    void labelChanged(QString);
    void decriptionChanged(QString);
};


#endif // XMLSETTINGSEDITORWRAPPER_H
