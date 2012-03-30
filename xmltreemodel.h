#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QMap>

class TreeItem :QObject
{
public:
    struct Attribute{ //custom struct to store Attribute key-value-pairs in original order
        QString key;
        QString value;
    };

    TreeItem(const QString &name, TreeItem *parent = 0);
    TreeItem(const QString &name, const QList<Attribute> &attributes,const QString &description, TreeItem *parent = 0);
    ~TreeItem();

    //standard members for AbstractItemModel
    void appendChild(TreeItem *child);
    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    TreeItem *parent();

    QString name() const;

    void insertChild(int index, const QString &key);//index is the "i" of a1..ai..a6, key should be ai
    void removeChild(int index);//index is the "i" of a1..ai..a6

    QString description() const;
    void setDescription(QString description);//only one description string can be set per item

    QList<Attribute> attributes() const; //returns itemAtributes
    Attribute attribute(int index) const;
    QString setAttribute(const QString &key,const QString &value);//overwrites and returns old value if attribute existed before, otherwise returns QString("")

    void insertAttribute(int index, const QString &key, const QString &value);//inserts attribute index is the "i" of a1..ai..a6 not the actual position in QList of attributes
    void removeAttribute(int index); //index is the "i" of a1..ai..a6

private:
    QList<TreeItem*> childItems;
    QString itemName;
    TreeItem *parentItem;
    QList<Attribute> itemAttributes; //QList of custom struct TreeItem::Attribute
    QString itemDescription;
};

#endif


#ifndef XMLTREEMODEL_H
#define XMLTREEMODEL_H

#include <QStandardItemModel>
#include <QXmlStreamReader>
#include <QFile>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class XmlTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    XmlTreeModel(QFile &device, QObject *parent);
    ~XmlTreeModel();

    //methodes implementing QAbstractItemModel
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    //methodes handling specially XML structured data
    QString description(const QModelIndex &parent) const;
    QList<TreeItem::Attribute> attributes(const QModelIndex &parent) const;
    TreeItem::Attribute attribute(const QModelIndex &parent, int index) const;
    bool changeAttribute(const QModelIndex &parent, const QString &key, const QString &value);
    void insertAttribute(const QModelIndex &parent, int index, const QString &key, const QString &value);
    void removeAttribute(const QModelIndex &parent, int index);
    //to be continued (i.e. changing structure, description etc.)

    void insertChild(QModelIndex index, int aIndex, QString name);
    void removeChild(QModelIndex index, int aIndex);

    bool save(QFile &device); //save modifications to specified file

    QList<QModelIndex> findItems(const QString &searchString) const;//providing fast selection in tree view

private:
    TreeItem * root;
    void recursiveWriting(QXmlStreamWriter &xmlWriter, TreeItem * item); //recursively write item tree to xml stream (used in save())
    void recursiveSearch(QList<QModelIndex> * list, int row, TreeItem * item, const QString &searchString) const ;//recursively search through tree for fast select (findItem())

    inline void init(){
        root = 0;
    }
};

#endif // XMLTREEMODEL_H
