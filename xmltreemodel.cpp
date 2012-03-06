#include "xmltreemodel.h"
#include <QDebug>


XmlTreeModel::XmlTreeModel(QFile &device, QObject *par) : QAbstractItemModel(par){
    //for now only readonly
    device.open(QIODevice::ReadOnly);
    QXmlStreamReader xmlReader(&device);

    if (!xmlReader.atEnd()){

        if (!xmlReader.hasError()) {
            TreeItem * parent;

            //cathing <config>-Tag (or any other first tag) as root item
            xmlReader.readNext();
            root = new TreeItem(xmlReader.name().toString(),0);
            parent = root;

            while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                xmlReader.readNext();

                //create a new childItem for every start tag
                if (xmlReader.isStartElement()) {
                    //summing attributes
                    QMap<QString,QString> *attr = new QMap<QString,QString>();
                    foreach(QXmlStreamAttribute elem, xmlReader.attributes()){
                        (*attr).insert(elem.value().toString(), elem.value().toString());
                    }
                    //create the new item an append to parent
                    TreeItem * item = new TreeItem(xmlReader.name().toString(),*attr,xmlReader.text().toString(),parent);
                    parent->appendChild(item);

                    //step up in the tree
                    parent = item;

                } else if (xmlReader.isEndElement()) {
                    //step down from the tree
                    parent = parent->parent();

                 // if token contains text content set it as parents description
                } else if (xmlReader.isCharacters() && !xmlReader.isWhitespace()) {
                    parent->setDescription(xmlReader.text().toString());
                }
            }
        }
        // warning on error
        if (xmlReader.hasError()) {
            qWarning() << xmlReader.errorString();
        }
    }
}

XmlTreeModel::~XmlTreeModel(){

}

int XmlTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 1;
    else
        return 0;
}

QVariant XmlTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return QVariant(item->name());
}

Qt::ItemFlags XmlTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant XmlTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return root->name();

    return QVariant();
}

QModelIndex XmlTreeModel::index(int row, int column, const QModelIndex &parent) const
{
//    if (!hasIndex(row, column, parent))
//        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = root;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex XmlTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == root)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int XmlTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = root;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}
