#include "xmlsettingseditorwrapper.h"

XMLSettingsEditorWrapper::XMLSettingsEditorWrapper(QFile * file, QWidget *parent) : QSplitter(parent)
{
    init(); //initialising members (defined in header)

    //creating new data model
        sourceFile = file;
        model = new XmlTreeModel(*sourceFile,this);

    //left side
        QWidget * leftVB = new QWidget(this); //right-side-description/edit-widget
        QVBoxLayout * leftVL = new QVBoxLayout(leftVB); //->vertical layouted

        QTreeView * tree = new QTreeView(this);//TreeView of XML donfig data
        tree->setModel(model); //connecting custom XMLTreeModel

        QLineEdit * searchInput = new QLineEdit(leftVB);//fast search bar

        leftVL->addWidget(tree); //top
        leftVL->addWidget(searchInput); //bottom

        leftVB->setLayout(leftVL);

        //search bar connections
        QObject::connect(searchInput,SIGNAL(textChanged(QString)),this,SLOT(fastSearch(QString))); //initiate new search -> store in resultList
        QObject::connect(searchInput,SIGNAL(returnPressed()),this,SLOT(nextFound()));//initiate selection of next result item in TreeView on enter
        QObject::connect(this,SIGNAL(selectFound(QModelIndex)),tree,SLOT(setCurrentIndex(QModelIndex)));//connect selection to TreeView

        //adding context menu to access collapseAll() of TreeView
        QAction * collapseAction = new QAction(tr("collapse all"), tree);//context menue entry
        QObject::connect(collapseAction,SIGNAL(triggered()),tree,SLOT(collapseAll())); //trigger collapseAll() of TreeView
        tree->addAction(collapseAction);//add action to View
        tree->setContextMenuPolicy(Qt::ActionsContextMenu); //activate context menu
    //left side

    //right side
        QWidget * rightVB = new QWidget(this); //right-side-description/edit-widget
        QVBoxLayout * rightVL = new QVBoxLayout(rightVB); //->vertical layouted

        QLabel * optionName = new QLabel(tr("Config"),rightVB); //view for selected item name
        QFont font(optionName->font().family(),12);//set font size 12
        font.setBold(true);//set font bold
        optionName->setFont(font);//apply font

        QPlainTextEdit * optionDescription = new QPlainTextEdit(tr("Please select an item in the <-TreeView!"),rightVB); //view for selected item description
        optionDescription->setReadOnly(true); //for now description does need to be changed (no associated functionality implemented)
        attrBox = new QGridLayout(rightVB); //layout to dynamically view attributes

        saveButton = new QPushButton(tr("save"),rightVB);
        saveButton->setDisabled(true);
        QObject::connect(saveButton,SIGNAL(clicked()),this,SLOT(saveChanges()));

        //puting everything together
        rightVL->addWidget(optionName,1);//top (title)
        rightVL->addWidget(optionDescription,7);//middle description
        rightVL->addLayout(attrBox,1);//middle attributes
        rightVL->addWidget(saveButton,1);//bottom

        rightVB->setLayout(rightVL);
    //right side

    this->addWidget(leftVB);
    this->addWidget(rightVB);
    //arange initial dimensions
    this->setMinimumSize(800,600);
    this->setStretchFactor(0,1);
    this->setStretchFactor(1,3);

    //selection event in QTreeView (left -> right)
    QObject::connect(tree, SIGNAL(activated(QModelIndex)), this, SLOT(optionSelected(QModelIndex))); //trigger this->show attributes on doubleClick and Enter (platform-dependant)
    QObject::connect(tree, SIGNAL(clicked(QModelIndex)), this, SLOT(optionSelected(QModelIndex))); //trigger this->show attributes on singleClick
    QObject::connect(this, SIGNAL(labelChanged(QString)), optionName, SLOT(setText(QString))); //write name to label view
    QObject::connect(this, SIGNAL(decriptionChanged(QString)), optionDescription, SLOT(setPlainText(QString))); //write description to view
}

void XMLSettingsEditorWrapper::optionSelected(QModelIndex index){ //SLOT
    emit labelChanged(index.data(Qt::DisplayRole).toString()); //set new right-side-title
    emit decriptionChanged(model->description(index));//set new right side description
    //handling dynamic attribute view
        AttributWidget * a;
        while (!attrList->isEmpty()){//remove all attribute widgets from
            a = attrList->takeFirst();
            attrBox->removeWidget(a);
            a->close(); //close widget created with Qt::WA_DeleteOnClose
        }
        r = -1;//reinitiate row and collumn count
        c = -1;

        QList<TreeItem::Attribute> attr = model->attributes(index);//get a QList of attributes
        if (!attr.isEmpty()){
            r++;
            QList<TreeItem::Attribute>::const_iterator i;
            for( i = attr.constBegin(); i != attr.constEnd(); i++){//adding all attributes as widgets to right-side attrBox (GridLayout) max 3 in a row
                c++;
                a = new AttributWidget(index, (*i).key, (*i).value, attrBox->parentWidget());//create custom AttributeWidget
                attrList->append(a);//save pointer in list for easy remove later
                attrBox->addWidget(a,r,c);//add to gridLayout
                QObject::connect(a,SIGNAL(attributeChanged(QModelIndex,QString,QString)),this,SLOT(datumChanged(QModelIndex,QString,QString)));//write back change of attribute to model
                if(c>1){//added widget with c=2 now return to nex row
                    c = -1;
                    r++;
                }
            }
        }
    //handling dynamic attribute view
}

void XMLSettingsEditorWrapper::datumChanged(QModelIndex index, QString key, QString value) //SLOT
{
    model->changeAttribute(index,key,value);
    saveButton->setEnabled(true);
}

void XMLSettingsEditorWrapper::saveChanges() //SLOT
{
    sourceFile->setFileName(sourceFile->fileName().append("~new.xml"));//todo: save changes in files with suffix
    if (model->save(*sourceFile)){
        saveButton->setDisabled(true);
    }
}

void XMLSettingsEditorWrapper::fastSearch(QString searchString)//SLOT
{
    resultList = model->findItems(searchString);
    lastIndex = 0;

    if (resultList.count() > 0){
        emit selectFound(resultList[lastIndex]);//select item in TreeView
        optionSelected(resultList[lastIndex]);//show selected item on right side
    }

}

void XMLSettingsEditorWrapper::nextFound()//SLOT
{
    if (resultList.count() > 0){
        lastIndex++;
        if (resultList.count() <= lastIndex){
            lastIndex = 0;
        }
        emit selectFound(resultList[lastIndex]);//select item in TreeView
        optionSelected(resultList[lastIndex]);//show selected item on right side
    }
}
