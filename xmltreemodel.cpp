#include "xmltreemodel.h"
#include <QDebug>
#include <QTextCodec>

TreeItem::TreeItem(const QString &name, TreeItem *parent)
{
    parentItem = parent;
    itemName = name;
    itemDescription = "";
}

TreeItem::TreeItem(const QString &name,const QList<Attribute> &attributes,const QString &description, TreeItem *parent){
    parentItem = parent;
    itemName = name;
    itemDescription = description;
    itemAttributes = attributes;
}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem *item)
{
    Q_ASSERT(item);//debug only
    childItems.append(item);
}

void TreeItem::insertChild(int index, const QString &key){
    TreeItem * a = new TreeItem(key,this);
    if (index > 0){
        int i = 1;
        int j = 1;
        while (i < childItems.count() && j < index){
            while (!childItems.value(i)->name().contains(QString::number(j)) && j < index){
                j++;
            }
            if (j != index)
                i++;
        }
        childItems.insert(i,a);
    }else{
        childItems.insert(index,a);
    }
}

TreeItem *TreeItem::child(int row)
{
    return childItems.value(row);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

TreeItem *TreeItem::parent()
{
    return parentItem;
}

int TreeItem::row() const
{
    if (parentItem)//because parent of root is 0x0
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

QString TreeItem::name() const
{
    return itemName;
}

QString TreeItem::description() const
{
    return itemDescription;
}

void TreeItem::setDescription(QString description)
{
    itemDescription = description;
}

QList<TreeItem::Attribute> TreeItem::attributes() const
{
    return itemAttributes;
}

QString TreeItem::appendAttribute(const QString &key,const QString &value)
{
    QString ret("");
    if (!itemAttributes.isEmpty()){

        //test if attribute exists -> replace value
        QList<Attribute>::iterator i;
        for(i = itemAttributes.begin(); (i != itemAttributes.end()) && ret.isEmpty(); i++){
            if (key == (*i).key){//found attribute
                ret = (*i).value; //store old value
                (*i).value = value; //overwrite with new
            }
        }

    }

    //if attribute hasn't exist append a new one
    if (ret.isEmpty()){
        TreeItem::Attribute a;
        a.key = key;
        a.value = value;
        itemAttributes.append(a);
    }

    return ret;//return the old value (empty if attribute hasn't exist)
}

void TreeItem::insertAttribute(int index, const QString &key, const QString &value)
{
    TreeItem::Attribute a;
    a.key = key;
    a.value = value;
    itemAttributes.insert(index,a);
}

TreeItem::Attribute TreeItem::attribute(int index) const
{
    return itemAttributes.value(index);
}

void TreeItem::removeAttribute(int index)
{
   itemAttributes.removeAt(index);
}

void TreeItem::removeChild(int index)
{
    if (index > 0){//only
        int i = 0;
        while (!childItems.value(i)->name().contains(QString::number(index)) && i < childItems.count()){
            i++;
        }
        if (i <= childItems.count())
            childItems.removeAt(i);
    }else{
        childItems.removeAt(index);
    }
}


XmlTreeModel::XmlTreeModel(QFile &device, QObject *par) : QAbstractItemModel(par){
    //for now readonly
    init();

    if (device.open(QIODevice::ReadOnly)){
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
    }else{
        throw device.error();
    }
}

XmlTreeModel::~XmlTreeModel(){

}

int XmlTreeModel::columnCount(const QModelIndex &parent) const
{
//    if (!parent.isValid())
//        return 0;
//    else
        return 1;
}

QVariant XmlTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    return QVariant(static_cast<TreeItem*>(index.internalPointer())->name());
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
    if (!parent.isValid())
        return createIndex(0, 0, root);
    else {
        TreeItem *childItem = static_cast<TreeItem*>(parent.internalPointer())->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }
}

QModelIndex XmlTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *parentItem = static_cast<TreeItem*>(index.internalPointer())->parent();

    if (parentItem == 0)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int XmlTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return 1;
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

    return (!static_cast<TreeItem*>(parent.internalPointer())->appendAttribute(key,value).isEmpty());
}

bool XmlTreeModel::save(QFile &device)
{
    device.open(QIODevice::WriteOnly);
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

void XmlTreeModel::insertAttribute(const QModelIndex &parent, int index, const QString &key, const QString &value)
{
    if (parent.isValid()){
        static_cast<TreeItem*>(parent.internalPointer())->insertAttribute(index,key,value);
    }
}

TreeItem::Attribute XmlTreeModel::attribute(const QModelIndex &parent, int index) const
{
    if (parent.isValid()){
        return static_cast<TreeItem*>(parent.internalPointer())->attribute(index);
    }
}

void XmlTreeModel::removeAttribute(const QModelIndex &parent, int index)
{
    if (parent.isValid()){
        return static_cast<TreeItem*>(parent.internalPointer())->removeAttribute(index);
    }
}

