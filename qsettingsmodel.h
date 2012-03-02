#ifndef QSETTINGSMODEL_H
#define QSETTINGSMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QStringList>
#include <QVector>
#include <QSettings>

/*
** Class for viewing QSettings like a model.
**
*/
class Q_GUI_EXPORT QSettingsModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  struct QSettingsNode
  {
            QSettingsNode          * parent;      // pointer to the parent of this node
            QString                  name;        // node name only, no path information
            QVariant                 value(void); // value of the node
            QVariant                 note(void);  // note of the node
    mutable bool                     isGroup;     // TRUE == is a group (folder, dir, whatever-your-pleasure)
    mutable QVector<QSettingsNode>   children;    // the list of children
    mutable bool                     populated;   // TRUE == it's 11 o-clock and we know who our children are

  }; // endstruct QSettingsModel::QSettingsNode

  /*
  ** In the QSettings ini files, there is a [general] section which can
  **  contain values.  This would be akin to storing files in your
  **  root directory along with folders.  Depending on how you're using
  **  this model it may be advantageous to NOT show the items in the
  **  [general] (root level) group.  Setting this value to fals causes
  **  those items to NOT be displayed and it also causes them to not
  **  not be created or deleted as well (since they're not visible).
  **  However, since you have access to the QSettings object through
  **  this interface, then you can still manipulate things in the
  **  [general] section.
  **
  */
  bool showRootItems;

  /*
  ** Define Constructor for internal QSettings.
  **
  */
  QSettingsModel
  (
    QString             filename,       // file where the settings are stored
    QSettings::Format   format,         // format of the file
    QObject           * parent = 0      // parent object (defaults to none)
  );

  /*
  ** Define Constructor for external QSettings.
  **
  */
  QSettingsModel
  (
    QSettings & settings,       // QSettings Object
    QObject   * parent = 0      // parent object (defaults to none)
  );

  /*
  ** Define Constructor for internal system QSettings
  **
  */
  QSettingsModel(QObject *parent = 0);

  /*
  ** Destructor
  **
  */
  ~QSettingsModel();


  /********************************************************************
  **
  ** AbstractItemModel Interface (pure virtual implementations)
  */
  virtual QModelIndex index       ( int row, int col, const QModelIndex & parent = QModelIndex()            ) const;
  virtual QModelIndex parent      (                   const QModelIndex & child                             ) const;
  virtual int         rowCount    (                   const QModelIndex & parent = QModelIndex()            ) const;
  virtual int         columnCount (                   const QModelIndex & parent = QModelIndex()            ) const;
  virtual QVariant    data        (                   const QModelIndex & index, int role = Qt::DisplayRole ) const;


  /********************************************************************
  **
  ** AbstractItemModel interface (virtual implementations)
  */
  virtual QVariant      headerData (       int           section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
  virtual Qt::ItemFlags flags      ( const QModelIndex & index                                                            ) const;
  virtual bool          setData    ( const QModelIndex & index, const QVariant &value, int role                           );


  /********************************************************************
  **
  ** QSettings convenience interface
  **
  **  note, i could probably use multiple-inheritance here to make this
  **  both a model and a qsettings object, but I'm unclear about how to
  **  do that properly.
  **
  */
  QStringList allKeys             (                                                                 ) const  { return qsettings->allKeys();                    }
  void        beginGroup          ( const QString & prefix                                          ) const  {        qsettings->beginGroup(prefix);           }
  int         beginReadArray      ( const QString & prefix                                          ) const  { return qsettings->beginReadArray(prefix);       }
  void        beginWriteArray     ( const QString & prefix, int size = -1                           )        {        qsettings->beginWriteArray(prefix,size); }
  QStringList childGroups         (                                                                 ) const  { return qsettings->childGroups();                }
  QStringList childKeys           (                                                                 ) const  { return qsettings->childKeys();                  }
  void        clear               (                                                                 )        {        qsettings->clear();                      }
  bool        contains            ( const QString & key                                             ) const  { return qsettings->contains(key);                }
  void        endArray            (                                                                 )        {        qsettings->endArray();                   }
  void        endGroup            (                                                                 ) const  {        qsettings->endGroup();                   }
  bool        fallbacksEnabled    (                                                                 ) const  { return qsettings->fallbacksEnabled();           }
  QString     fileName            (                                                                 ) const  { return qsettings->fileName();                   }
  QString     group               (                                                                 ) const  { return qsettings->group();                      }
  bool        isWritable          (                                                                 ) const  { return qsettings->isWritable();                 }
  void        remove              ( const QString & key                                             )        {        qsettings->remove(key);                  }
  void        setArrayIndex       (       int       i                                               )        {        qsettings->setArrayIndex(i);             }
  void        setFallbacksEnabled (       bool      b                                               )        {        qsettings->setFallbacksEnabled(b);       }
  void        setValue            ( const QString & key, const QVariant & value                     )        {        qsettings->setValue(key,value);          }
  void        sync                (                                                                 )        {        qsettings->sync();                       }
  QVariant    value               ( const QString & key, const QVariant & defaultValue = QVariant() ) const  { return qsettings->value(key,defaultValue);      }


  /********************************************************************
  **
  ** QSettings enhancements
  **
  */
  void rename ( const QString & key, const QVariant & newKey ) const;

    /********************************************************************
  **
  ** QSettingsModel item management
  **
  */
  bool        isGroup  (  const QModelIndex   & index                           ) const;
  QModelIndex mkGroup  (  const QModelIndex   & parent, const QString & name    );
  bool        rmGroup  (  const QModelIndex   & index                           );
  bool        rename   (        QSettingsNode * node,   const QVariant & newKey );
  bool        remove   (  const QModelIndex   & index                           );
  void        setValue (  const QSettingsNode * node,   const QVariant & newKey );

  /********************************************************************
  **
  ** QSettingsModel item/model interface
  **
  */
  QSettingsNode          * node     ( int row, QSettingsNode *parent ) const;
  QSettingsNode          * parent   (          QSettingsNode *child  ) const  { return child ? child->parent : 0;}
  QVector<QSettingsNode>   children (          QSettingsNode *parent ) const;
  int                      idx      (          QSettingsNode *node   ) const;
  void                     refresh  (          QSettingsNode *parent );

private:

  /*
  ** Keep a root node for attaching everything to.
  **
  */
  mutable QSettingsNode root;

  /*
  ** Keep a pointer to the QSettings object
  **
  */
  QSettings * qsettings;

  /*
  ** Setting this to TRUE causes this entire model to be ReadOnly.
  **
  */
  bool readOnly;

  /*
  ** Object initializer.
  **
  */
  inline void init(void)
  {
    root.isGroup   = true;
    root.populated = false;
    root.parent    = 0;
    root.name      = "";
    readOnly       = false;
    showRootItems  = true;
  }

  /*
  ** Node populator.
  **
  */
  inline void populate(QSettingsNode *parent) const
  {
    Q_ASSERT(parent);
    parent->children = children(parent);
    parent->populated = true;
  }

  QVariant value      ( const QSettingsNode * node ) const;
  QString  parentPath ( const QSettingsNode * node ) const;
  QString  fullPath   ( const QSettingsNode * node ) const;

}; // endclass QSettingsModel


#endif // #ifdef QSETTINGSMODEL_H
