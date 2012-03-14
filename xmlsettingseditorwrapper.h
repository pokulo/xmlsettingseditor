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
#include <QAction>
#include <QMenu>
#include <QFileDialog>
#include <QSpacerItem>

#include <xmltreemodel.h>
#include "attributwidget.h"


class XMLSettingsEditorWrapper : public QSplitter
{
    Q_OBJECT

public:
    explicit XMLSettingsEditorWrapper(QWidget *parent = 0);
    inline ~XMLSettingsEditorWrapper(){}

private:
    QString sourceFile;
    QTreeView * tree;
    XmlTreeModel * model; //Datamodel containing whole xml data tree
    QGridLayout * attrBox; //Layout management for AttributeWigdets
    QList<AttributWidget*> * attrList; //separate storing for widget pointer to delete/close them later
    int r,c; //r : rowCount of attrBox / c : colCount of last row of attrBox (managing layout if AttributeWidgets

    QPushButton * saveButton;//trigger saveToFile action
    QPushButton * resetButton;//trigger saveToFile action

    int lastIndex; //last selected search result
    QList<QModelIndex> resultList; //results of fastSearch

    inline void init(){
        r = -1; //no AttributeWidget in last row of attrBox
        c = -1; //no AttributeWidget in attrBox
        attrList = new QList<AttributWidget*>();
        tree = new QTreeView(this);//TreeView of XML donfig data
        model = 0;
    }

public slots:
    void openXMLFile();
    void openFileDialog();
    void selectionChanged(QModelIndex current,QModelIndex previous);
    void optionSelected(QModelIndex index); //slot for connection from TreeView->(itemSelected) to detailed view
    void datumChanged(QModelIndex index,QString key,QString value); //slot for conntection from attribute edit->(textChanged) to Model (save changes)
    void saveChanges(); //save button pressed -> save to file
    void fastSearch(QString searchString); //fast search string changed -> new search
    void nextFound();//search string didn't change -> select next

signals:
    void labelChanged(QString); //indicating newly selected TreeItem->name() chould be displayed
    void decriptionChanged(QString); //indicating newly selected TreeItem->description() chould be displayed
    void selectFound(QModelIndex); //indicating fast-search found index to tree view

};

#endif // XMLSETTINGSEDITORWRAPPER_H
