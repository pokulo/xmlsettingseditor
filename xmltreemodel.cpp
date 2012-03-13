#include "xmltreemodel.h"
#include <QDebug>
#include <QTextCodec>


XmlTreeModel::XmlTreeModel(QFile &device, QObject *par) : QAbstractItemModel(par){
    //for now readonly
    device.open(QIODevice::ReadOnly);
    QXmlStreamReader xmlReader(&device);

    while (!xmlReader.atEnd() && (xmlReader.name().toString() == "" || xmlReader.isWhitespace())){
        xmlReader.readNext();
    }

    if (!xmlReader.atEnd()){
        TreeItem * parent;

        //cathing <config>-Tag (or any other first tag) as root item
        root = new TreeItem(xmlReader.name().toString(),0);
        parent = root;

        while (!xmlReader.atEnd()) {
            xmlReader.readNext();

            //create a new childItem for every start tag
            if (xmlReader.isStartElement()) {
                //summing attributes
                QList<TreeItem::Attribute> *attr = new QList<TreeItem::Attribute>();
                foreach(QXmlStreamAttribute elem, xmlReader.attributes()){
                    TreeItem::Attribute a;
                    a.key = elem.name().toString();
                    a.value = elem.value().toString();
                    (*attr).append(a);
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
    xmlReader.clear();
    device.close();
}

XmlTreeModel::~XmlTreeModel(){

}

int XmlTreeModel::columnCount(const QModelIndex &parent) const
{
        return 1;
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
        return QVariant(root->name());
    else
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

    if (childItem == root)
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

QString XmlTreeModel::description(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return "";
    TreeItem * item = static_cast<TreeItem*>(parent.internalPointer());
    if(item != 0x0){
        QString ret;
        do {
            ret = item->description();
            item = item->parent();
        }while (ret.isEmpty() && item != 0x0);
        return ret;
    }else{
        return QString();
    }
}

QList<TreeItem::Attribute> XmlTreeModel::attributes(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return QList<TreeItem::Attribute>();

    return static_cast<TreeItem*>(parent.internalPointer())->attributes();
}

bool XmlTreeModel::changeAttribute(const QModelIndex &parent, const QString &key, const QString &value)
{
    if (!parent.isValid())
        return false;

    return (!static_cast<TreeItem*>(parent.internalPointer())->insertAttribute(key,value).isEmpty());
}

bool XmlTreeModel::save(QFile &device)
{
    device.open(QIODevice::ReadWrite);
    QXmlStreamWriter xmlWriter(&device);

    xmlWriter.setAutoFormatting(true);//adds line breaks
    xmlWriter.setAutoFormattingIndent(true);//<tag /> have bad indentation anyway
//    xmlWriter.setCodec(); //should be automatically UTF-8 from header

    xmlWriter.writeStartDocument();

        TreeItem * item = root;
        recursiveWriting(xmlWriter,item);//recurse through model tree

    xmlWriter.writeEndDocument();
    xmlWriter.~QXmlStreamWriter();//to fully write buffer
    device.close();

    return true;
}

void XmlTreeModel::recursiveWriting(QXmlStreamWriter &xmlWriter, TreeItem *item)
{
   QList<TreeItem::Attribute>::iterator i;

   //NAME (parent)
   xmlWriter.writeStartElement(item->name());

   //ATTRIBUTES
   QList<TreeItem::Attribute> att = item->attributes();
   for (i = att.begin(); i != att.end(); i++){
       xmlWriter.writeAttribute((*i).key,(*i).value);
   }
   //DESCRIPTION as first child characters
   if(!item->description().isEmpty()){
       xmlWriter.writeCharacters(item->description());
   }
   //recurse to children
   for (int j = 0; j < item->childCount(); j++){
       recursiveWriting(xmlWriter,item->child(j));
   }
   //close parent
   xmlWriter.writeEndElement();
}

QList<QModelIndex> XmlTreeModel::findItems(const QString &searchString) const
{
    QList<QModelIndex> list;
    if (!searchString.isEmpty())
        recursiveSearch(&list, 0, root, searchString);//recurse through tree
    return list;
}

void XmlTreeModel::recursiveSearch(QList<QModelIndex> *list, int row, TreeItem *item, const QString &searchString) const
{
    if (item->name().contains(searchString,Qt::CaseInsensitive) || item->description().contains(searchString,Qt::CaseInsensitive)){
        //append item to results if name or description contain the searchString
        list->append(createIndex(row,0,item));
    }
    for(int i = 0; i < item->childCount(); i++){//recurse to children
        recursiveSearch(list,i,item->child(i),searchString);
    }
}

