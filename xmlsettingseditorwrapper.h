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
#include <QFile>
#include <QPushButton>

#include <xmltreemodel.h>
#include "attributwidget.h"


class XMLSettingsEditorWrapper : public QSplitter
{
    Q_OBJECT

public:
    explicit XMLSettingsEditorWrapper(QFile * file, QWidget *parent = 0);
    inline ~XMLSettingsEditorWrapper(){}

private:
    QFile  * sourceFile;
    XmlTreeModel * model; //Datamodel containing whole xml data tree
    QGridLayout * attrBox; //Layout management for AttributeWigdets
    QList<AttributWidget*> * attrList; //separate storing for widget pointer to delete/close them later
    int r,c; //r : rowCount of attrBox / c : colCount of last row of attrBox (managing layout if AttributeWidgets

    inline void init(){
        r = -1; //no AttributeWidget in last row of attrBox
        c = -1; //no AttributeWidget in attrBox
        attrList = new QList<AttributWidget*>();
    }

public slots:
    void optionSelected(QModelIndex index); //slot for connection from TreeView->(itemSelected) to detailed view
    void datumChanged(QModelIndex index,QString key,QString value); //slot for conntection from attribute edit->(textChanged) to Model (save changes)
    void saveChanges();

signals:
    void labelChanged(QString); //indicating newly selected TreeItem->name() chould be displayed
    void decriptionChanged(QString); //indicating newly selected TreeItem->description() chould be displayed

};

#endif // XMLSETTINGSEDITORWRAPPER_H
