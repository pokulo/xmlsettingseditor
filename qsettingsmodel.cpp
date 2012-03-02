#include "qsettingsmodel.h"
#include <QMessageBox>
//#include <QDebug>

QSettingsModel::QSettingsModel
(
  QString             filename,  // file where the settings are stored
  QSettings::Format   format,    // format of the file
  QObject           * parent     // parent object (defaults to none)
)
: QAbstractItemModel(parent)
{
  qsettings = new QSettings(filename,format,this);
//  qDebug( "qsettings object has been created to %s", filename.toAscii().data() );
  init();
}

/*
** Define Constructor for external QSettings.
**
*/
QSettingsModel::QSettingsModel
(
  QSettings & settings,  // QSettings Object
  QObject   * parent     // parent object (defaults to none)
)
: QAbstractItemModel(parent)
{
  qsettings = &settings;
  init();
}

/*
** Define Constructor for internal system QSettings
**
*/
QSettingsModel::QSettingsModel(QObject *parent)
: QAbstractItemModel(parent)
{
  qsettings = new QSettings(this);
  init();
}

/*
** Destructor
**
*/
QSettingsModel::~QSettingsModel()
{
}


/********************************************************************
**
** AbstractItemModel Interface
**
*/
QModelIndex QSettingsModel::index(int row, int col, const QModelIndex & parent) const
{
//  qDebug( "index row %d col %d",row,col );

  /*
  ** Make sure the requested colum and row is within range before doing anything.
  **
  */
  if( col < 0 || col >= 1 || row < 0 || row >= rowCount(parent) )
    return QModelIndex();

  /*
  ** Get a pointer to our parent and make a new node item for this row
  **  with the proper parent.
  **
  */
  QSettingsNode *p = static_cast<QSettingsNode*>(parent.internalPointer());
  QSettingsNode *n = node(row, p);
  Q_ASSERT(n);

  /*
  ** Create a new index item and return that to the caller.  Passing .n.
  **  insures that the index has the internalPointer set to the new
  **  node we created.
  **
  */
  return createIndex(row,col,n);

} // endQModelIndex QSettingsModel::index(int row, int col, const QModelIndex & parent) const


QModelIndex QSettingsModel::parent(const QModelIndex & child) const
{
  /*
  ** Must have a valid index before we can find its parent.
  **
  */
  if (!child.isValid())
    return QModelIndex();

  /*
  ** Get a pointer to this indexs' internalPointer which points to the
  **  actual node.  The node will contain a pointer to its parent node.
  **
  */
  QSettingsNode *node =
      static_cast<QSettingsNode*>(child.internalPointer());
  Q_ASSERT(node);

//  qDebug( "QModelIndex parent child=%s",node->name.toAscii().data() );

  /*
  ** Referring to this function parent() with our node will return
  **  our parent node.
  **
  */
  QSettingsNode *par = parent(node);
  if( !par || (par == &root) )
  {
//    qDebug("we are at the root node");
    return QModelIndex(); // parent is the root node
  }

  /*
  ** idx returns the index position of a particular node within
  **   a list
  **
  */
  int r = idx(par);
  Q_ASSERT(r >= 0);
  return createIndex(r, 0, par);

} // endQModelIndex QSettingsModel::parent(const QModelIndex & child) const


int QSettingsModel::rowCount(const QModelIndex & parent) const
{
  /*
  ** The p pointer points to the parent.internalPointer which basically points
  **  to our private data structure containing the necessary details about this
  **  (the parent) node.
  **
  */
  QSettingsNode *p =
      static_cast<QSettingsNode*>(parent.internalPointer());

  /*
  ** The parent is a folder when we have no parent internalPointer, meaning
  **  we must refer to the root item, or we do have an internalPointer and it
  **  is identifying itself as a folder.
  **
  */
  bool isGroup = !p || p->isGroup; // no node pointer means that it is the root

  /*
  ** If this parent has no internalPointer then we must refer to the root
  **  item.
  **
  */
  if( !p )
    p = &root;

  /*
  ** If this is a folder, and it is not populated, then populate it.
  **
  */
  if( isGroup && !p->populated ) // lazy population (only populates this parent)
    populate(p);


//  qDebug( "rowCount %d",p->children.count() );

  /*
  ** Return the children count.
  **
  */
  return p->children.count();

} // endint QSettingsModel::rowCount(const QModelIndex & parent) const

int QSettingsModel::columnCount(const QModelIndex & parent) const
{
//  qDebug( "columnCount" );
  return 1;
}


QVariant QSettingsModel::data(const QModelIndex & index, int role) const
{
  /*
  ** Make sure we have a vaild index.
  **
  */
  if( !index.isValid() )
    return QVariant();

  /*
  ** Get the node data for this index.  If there isn't any node
  **  data then we have a serious error.
  **
  */
  QSettingsNode *node =
      static_cast<QSettingsNode*>(index.internalPointer());
  Q_ASSERT(node);

  /*
  ** We only parse DisplayRole and EditRole.
  **
  */
  if( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    switch( index.column() )
    {
      case 0: return node->name;
      case 1: return value(node);

      default:
      {
        qWarning("data: invalid display value column %d", index.column());
        return QVariant();
      }
    }
  }

#ifdef NEVER
  if (index.column() == 0)
  {
    if (role == SettingsIconRole) return fileIcon(index);
    if (role == FilePathRole)     return filePath(index);
    if (role == FileNameRole)     return fileName(index);
  }
#endif

  return QVariant();

} // endQVariant QSettingsModel::data(const QModelIndex & index, int role) const


Qt::ItemFlags QSettingsModel::flags(const QModelIndex &index) const
{
  /*
  ** Get our ancestor flags.
  **
  */
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  /*
  ** If we have a bad index then stop here.
  **
  */
  if( !index.isValid() )
    return flags;

  /*
  ** If we're readonly then we're done.
  **
  */
  if( readOnly )
    return flags;

  /*
  ** Get a handle on the node.
  **
  */
  QSettingsNode *node =
      static_cast<QSettingsNode*>(index.internalPointer());
  Q_ASSERT( node );

//  if( (index.column() == 0) && node->info.isWritable() )
//  {
    flags |= Qt::ItemIsEditable;
//    if( fileInfo(index).isDir() ) // is directory and is editable
//      flags |= Qt::ItemIsDropEnabled;
//  }

  return flags;

} // endQt::ItemFlags QSettingsModel::flags() const

bool QSettingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  /*
  ** These are the conditions upon which this item can be updated.
  **
  */
  if(
      /*
      ** The index we have received must be valid.
      **
      */
      !index.isValid() ||

      /*
      ** Flags associated with this item must indicate that it is
      **  editable.
      **
      */
      (flags(index) & Qt::ItemIsEditable) == 0 ||

      /*
      ** The role requested must be the EditRole.
      **
      */
       role != Qt::EditRole
    )
      return false;

  /*
  ** Get a handle on the node.
  **
  */
  QSettingsNode *node =
      static_cast<QSettingsNode*>(index.internalPointer());
  Q_ASSERT(node);

  /*
  ** Set the value
  **
  */
  if( index.column() == 0 ) rename(   node,value );
  if( index.column() == 1 ) setValue( node,value );

  /*
  ** Emit a refresh signal.
  **
  */
  QModelIndex par = parent(index);
  QModelIndex topLeft = this->index(0, 0, par);
  int rc = rowCount(par);
  int cc = columnCount(par);
  QModelIndex bottomRight = this->index(rc, cc, par);
  emit dataChanged(topLeft, bottomRight);

  return true;

} // endbool QSettingsModel::setData(const QModelIndex &index, const QVariant &value, int role)


QSettingsModel::QSettingsNode *QSettingsModel::node(int row, QSettingsNode *parent) const
{
  /*
  ** .row. cannot be less than zero - it shouldn't be
  **
  */
  if( row < 0 )
    return 0;

  /*
  ** If there is no parent to this node, or this parent indicates itself
  **  as a group then remember that.
  **
  */
  bool isGroup =  !parent || parent->isGroup;

//  qDebug("node row %d parent is %s",row,isGroup?"a group":"not a group");

  /*
  ** If this node has no parent then return the root item.
  **
  */
  QSettingsNode *p = (parent ? parent : &root);

  /*
  ** If this is a group node and it's not populated then
  **  populate it.  This is what is called a lazy population
  **  because it only populates when a group is expanded.
  **
  */
  if( isGroup && !p->populated )
    populate(p); // will also resolve symlinks

  /*
  ** Check for bad row numbers.
  **
  */
  if( row >= p->children.count() )
  {
    qWarning("node: the row does not exist");
    return 0;
  }

  /*
  ** Point to the node based upon the row requested.
  **
  */
  QSettingsNode *retVal = const_cast<QSettingsNode*>(&p->children.at(row));

//  qDebug( "node name %s is %s",retVal->name.toAscii().data(),retVal->isGroup?"a group":"not a group" );

  return retVal;

} // endQSettingsModel::QSettingsNode *QSettingsModel::node(int row, QSettingsNode *parent) const


/****************************************************************************
**
** children
**
** This method returns all the children for a particular node.  This is
**  where the beef begins.  This is where the Model meets the Settings
**  object.  Up till now all the other Model interface methods, index
**  parent, rowCount, columnCount, data, flags, setData and node have all
**  dealt with existing nodes or links to nodes and what-not.  None of them
**  have so far interfaced to the QSettings object.  So, you've got to
**  be asking 'how do they get the data to and from the QSettings object???'
**  The answer is; 'this is the 'from' part of that question.'
**
** This method populates a vector of QSettingsNodes.  It does this by
**  reading out all of the groups and keys from the QSettings object and
**  placing the key names and key values into the QSettingsNodes vector
**  items.  It reads out all the items 'from' the QSettings object at a
**  particular branch level... the children of a particular parent.
**
*/
QVector<QSettingsModel::QSettingsNode> QSettingsModel::children(QSettingsNode *parent) const
{
  /*
  ** Make sure we have a valid pointer.
  **
  */
  Q_ASSERT(parent);

//  qDebug("children for '%s'",parent->name.toAscii().data());

  /*
  ** The field .name. should contain only the immediate name
  **  not the fully qualified path of the parent group, so we
  **  have to specially request that.  By setting beginGroup()
  **  we are forcing the QSettings object to return only those
  **  groups and keys within that group (or parent).  When we
  **  do that we have to make sure we remember to 'undo' that
  **  group specification by calling endGroup().
  **
  */
  beginGroup(fullPath(parent));

  /*
  ** Create the nodes required for the return result.
  **
  */
  QVector<QSettingsNode> nodes;

  /*
  ** Here we have an option to show the root items of the QSettings
  **  object.  From a file/directory perspective, showing these
  **  items is the equivalent of showing 'files' in the root of
  **  your harddrive.  From a QSettings perspective, showing these
  **  items is the same as showing the keys/values in the [general]
  **  section of the .ini file.
  **
  */
  if( parent == &root && showRootItems == false )
  {
    nodes.resize( childGroups().count() );
  }
  else
  {
    nodes.resize( childGroups().count() + childKeys().count() );
  }

  /*
  ** Load the groups
  **
  */
  for( int i=0; i < childGroups().count(); i++ )
  {
    QSettingsNode &node = nodes[i];
    node.parent         = parent;
    node.name           = childGroups().at(i);
    node.isGroup        = true;
    node.populated      = false;
  }

  /*
  ** Load the keys (items)
  **
  */
  if( !(parent == &root && showRootItems == false) )
  {
    for( int i=0; i < childKeys().count(); i++ )
    {
      QSettingsNode &node = nodes[childGroups().count() + i];
      node.parent         = parent;
      node.name           = childKeys().at(i);
      node.isGroup        = false;
      node.populated      = false;
    }
  }

  /*
  ** Don't never forget to do this after setting beginGroup().
  **
  */
  endGroup();

  return nodes;

} // endQVector<QSettingsModel::QSettingsNode> QSettingsModel::children(QSettingsNode *parent) const

/*
** This returns the index position of a node within the list of children.
**
*/
int QSettingsModel::idx(QSettingsNode *node) const
{
  Q_ASSERT(node);

  /*
  ** If this is the root node then the index is always zero.
  **
  */
  if( node == &root )
    return 0;

  /*
  ** Get a local as a vector for all the children of the parent
  **  of this node.  Note that this code is a bit redundant when
  **  it tests for node->parent? because we already tested for
  **  the root node in the conditional above, but this was copied
  **  from QDir and that's the way they did it.
  **
  */
  const QVector<QSettingsNode> children =
      node->parent ? node->parent->children : root.children;

  /*
  ** If there are no children then we have a problem, because we (node) are a child
  **  of our parent, and we just fetched all the children of our parent, and if we're
  **  now not listed then something is wrong.
  **
  */
  Q_ASSERT(children.count() > 0);

  /*
  ** Setup to determine our index position.
  **
  */
  const QSettingsNode *first = &(children.at(0));

  /*
  ** Return our index position.
  **
  */
  return (node - first);

} // endint QSettingsModel::idx(QSettingsNode *node) const

QVariant QSettingsModel::value(const QSettingsNode *node) const
{
  Q_ASSERT(node);

  return value(fullPath(node));
}

QString QSettingsModel::parentPath( const QSettingsNode * node ) const
{
  Q_ASSERT(node);

  /*
  ** If this node has a parent, then we need to fetch the parent
  **  name.  This is a recursive function call and should top out
  **  at the absolute parent of the tree.
  **
  */
  if( node->parent )
    return parentPath(node->parent) + node->parent->name + "/";

  return QString();

} // endQString QSettingsModel::parentPath(QSettingsNode * node) const

QString QSettingsModel::fullPath( const QSettingsNode * node ) const
{
  Q_ASSERT(node);

  return parentPath(node) + node->name;

} // endQString QSettingsModel::fullPath(QSettingsNode *node) const


void QSettingsModel::setValue( const QSettingsNode * node,const QVariant & newKey )
{
  setValue(fullPath(node),newKey);

} // endvoid QSettingsModel::setValue( const QSettingsNode * node,const QVariant & newKey )

/****************************************************************************
**
** rename
**
** This method takes care of changing the name of a node on a particular
**  branch.  It does NOT support moving a node to a different branch.
**  All we have when we enter this function is the existing node and the
**  newKey that we'd like that node to be called.  Therefore, the parent
**  of the newKey must be the same as the parent of the node.
**
*/
bool QSettingsModel::rename(QSettingsNode * node,const QVariant & newKey )
{
  Q_ASSERT(node);

//  qDebug
//  (
//    "renaming node '%s=%s' to '%s' (existing node %s a group)",
//    fullPath(node).toAscii().data(),
//    value(fullPath(node)).toString().toAscii().data(),
//    (parentPath(node)+newKey.toString()).toAscii().data(),
//    node->isGroup? "is" : "is not"
//  );

  /*
  ** Make sure they're not trying to rename this node to the same name.  This
  **  happens when someone double-clicks on the node name and then just hits
  **  <enter> to get off the editor.
  **
  */
  if( node->name == newKey.toString() )
    return false;

  /*
  ** Check right here if the new key already exists.  If so then we need
  **  some way to either stop the operation or merge the data to the
  **  other existing key and drop this node.
  **
  ** NOTE: Right now the code will just abort.  In the case of QDirModel,
  **        depending on the conditions, it will either overwrite the
  **        destination node or crash with a segmentation fault.
  */
  const QVector<QSettingsNode> children = node->parent->children;

  for( int i=0; i < children.count(); i++ )
  {
//    qDebug( "child %s",children[i].name.toAscii().data() );
    if( children[i].name == newKey.toString() )
    {
//      qDebug( "key exists!" );
      return false;
    }
//    qDebug( "the parent of this node has children" );
  }


  /*
  ** Look for all children of this node and rename them first.
  **
  */
//  if( node->isGroup )
//  {
//    rename(node->children[i],
//  }

  /*
  ** Set the value for the new key - note that this only allows the key
  **  to be changed at the same tree depth... this will not handle
  **  moving a key to a different branch.
  **
  */
//  setValue( parentPath(node)+newKey.toString(),value(fullPath(node)) );

  /*
  ** Remove the old key
  **
  */
//  remove( fullPath(node) );

  /*
  ** This isn't finished yet, but for simplicity just rename the existing
  **  node.
  **
  */
//  node->name = newKey.toString();

  return true;

} // endbool QSettingsModel::rename(QSettingsNode * node,const QVariant & newKey )

/********************************************************************
**
** AbstractItemModel Interface
*/
QVariant QSettingsModel::headerData( int section, Qt::Orientation orientation, int role) const
{
//  qDebug( "headerData section %d role(%s)",section,role2String(role) );

  if( orientation == Qt::Horizontal )
  {
    if( role == Qt::DisplayRole )
    {
      switch( section )
      {
        case 0: return tr( "Name"  );
        case 1: return tr( "Value" );
      }
    }
    return QVariant();
  }

  return QAbstractItemModel::headerData(section,orientation,role);

} // endQVariant QSettingsModel::headerData( int section, Qt::Orientation orientation, int role) const


bool QSettingsModel::isGroup(const QModelIndex &index) const
{
}


QModelIndex QSettingsModel::mkGroup(const QModelIndex &parent, const QString &name)
{
}


bool QSettingsModel::rmGroup(const QModelIndex &index)
{
}


bool QSettingsModel::remove(const QModelIndex &index)
{
}
