#ifndef XMLSETTINGSEDITORWRAPPER_H
#define XMLSETTINGSEDITORWRAPPER_H

#include <QMainWindow>
#include <QModelIndex>
#include <QTreeView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QList>

#include <xmltreemodel.h>
#include "attributwidget.h"


class XMLSettingsEditorWrapper : public QSplitter
{
    Q_OBJECT

public:
    explicit XMLSettingsEditorWrapper(QSplitter *parent = 0);
    ~XMLSettingsEditorWrapper();

private:
    XmlTreeModel * model;
    QGridLayout * attrBox;
    QList<AttributWidget*> * attrList;
    int r,c;

    inline void init(){
        r = -1;
        c = -1;
        attrList = new QList<AttributWidget*>();
    }

public slots:
    void optionSelected(QModelIndex index);
    void datumChanged(QModelIndex index,QString key,QString value);

signals:
    void labelChanged(QString);
    void decriptionChanged(QString);

};

#endif // XMLSETTINGSEDITORWRAPPER_H
