#ifndef ATTRIBUTWIDGET_H
#define ATTRIBUTWIDGET_H

#include <xmltreemodel.h>

#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>

class AttributeWidget : public QWidget
{
    Q_OBJECT
public:
    AttributeWidget(QModelIndex index,const QString &label,const QString &value, QWidget *parent = 0, int attributeIndex = 0, int attributeTagIndex = 0, bool hasValue = true);
//    AttributWidget(QModelIndex index,const QString &label,const QString &value,int attributeIndex, QWidget *parent = 0);
    void setDisabled(bool disabled);
    bool isEnabled();
    QString label();

private:
    QModelIndex modelIndex;
    QPushButton * labelWidget;
    QLineEdit * valueWidget;
    int attributeIndex;
    int attributeTagIndex;

signals:
    void attributeChanged(QModelIndex,QString,QString);//indicating changed text of LineEdit -> model
    void activateAttribute(QModelIndex modelIndex, int attributeIndex);//if clicked to activate attributeWidget
    void deactivateAttribute(QModelIndex modelIndex, int attributeIndex, AttributeWidget * widget);//if clicked to activate attributeWidget
    void activateTagAttribute(QModelIndex modelIndex, int attributeIndex);//if clicked to activate attributeWidget
    void deactivateTagAttribute(QModelIndex modelIndex, int attributeIndex, AttributeWidget * widget);//if clicked to activate attributeWidget

public slots:
    void setValue(QString value);

private slots:
    void valueChanged(QString value);//slot for LineEdit indicating changed text
    void attributeToggled(bool checked);

};

#endif // ATTRIBUTWIDGET_H


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
#include <QResource>

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
    QList<AttributeWidget*> * attrList; //separate storing for widget pointer to delete/close them later
    int r,c; //r : rowCount of attrBox / c : colCount of last row of attrBox (managing layout if AttributeWidgets
    QList<TreeItem::Attribute> allAttrList;

    QPushButton * saveButton;//trigger saveToFile action
    QPushButton * resetButton;//trigger saveToFile action

    int lastIndex; //last selected search result
    QList<QModelIndex> resultList; //results of fastSearch

    int lastSelected;
    QList<QModelIndex> history;
    QPushButton * backwardButton;
    QPushButton * forwardButton;


    inline void init(){
        r = -1; //no AttributeWidget in last row of attrBox
        c = -1; //no AttributeWidget in attrBox
        attrList = new QList<AttributeWidget*>();
        tree = new QTreeView(this);//TreeView of XML donfig data
        tree->setHeaderHidden(true);
        model = 0;
//        QResource::registerResource("icons.qrc");
    }

private slots:
    void openXMLFile();
    void openFileDialog();

    void selectionChanged(QModelIndex current,QModelIndex previous);//selection in TreeView changed calling optionSelected(old)

    void optionSelected(QModelIndex index); //slot for connection from TreeView->(itemSelected) to detailed view

    void activateAttribute(QModelIndex index, int attributeIndex);// inserting one of a1..a6 attribute onToggle into attrBox
    void deactivateAttribute(QModelIndex index, int attributeIndex, AttributeWidget * widget);// removing one of a1..a6 attribute onToggle from attrBox

    void activateAttributeTag(QModelIndex index, int attributeIndex);// inserting one of a1..a6 attribute onToggle into attrBox
    void deactivateAttributeTag(QModelIndex index, int attributeIndex, AttributeWidget * widget);// removing one of a1..a6 attribute onToggle from attrBox

    void datumChanged(QModelIndex index,QString key,QString value); //slot for conntection from attribute edit->(textChanged) to Model (save changes)
    void saveChanges(); //save button pressed -> save to file
    void fastSearch(QString searchString); //fast search string changed -> new search
    void nextFound();//search string didn't change -> select next

    void goBackwards();//select previously selected QModelIndex
    void goForwards();//select next QModel in history

signals:
    void labelChanged(QString); //indicating newly selected TreeItem->name() chould be displayed
    void decriptionChanged(QString); //indicating newly selected TreeItem->description() chould be displayed
    void selectFound(QModelIndex); //indicating fast-search found index to tree view

};

#endif // XMLSETTINGSEDITORWRAPPER_H
