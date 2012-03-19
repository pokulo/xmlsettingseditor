#include "xmlsettingseditorwrapper.h"

#include <QDebug>

XMLSettingsEditorWrapper::XMLSettingsEditorWrapper(QWidget *parent) : QSplitter(parent)
{
    init(); //initialising members (defined in header)

    //left side
        QWidget * leftVB = new QWidget(this); //right-side-description/edit-widget
        QVBoxLayout * leftVL = new QVBoxLayout(leftVB); //->vertical layouted

        QHBoxLayout * searchHB = new QHBoxLayout(leftVB);
        QLineEdit * searchInput = new QLineEdit(leftVB);//fast search bar
        QPushButton * searchButton= new QPushButton(QIcon("../icons/search.png"),"",leftVB);

        searchHB->addWidget(searchInput,4);
        searchHB->addWidget(searchButton,1);

        leftVL->addWidget(tree); //top
        leftVL->addLayout(searchHB); //bottom

        leftVB->setLayout(leftVL);

        //search bar connections
        QObject::connect(searchInput,SIGNAL(textChanged(QString)),this,SLOT(fastSearch(QString))); //initiate new search -> store in resultList
        QObject::connect(searchInput,SIGNAL(returnPressed()),this,SLOT(nextFound()));//initiate selection of next result item in TreeView on enter
        QObject::connect(searchButton,SIGNAL(clicked()),this,SLOT(nextFound()));//initiate selection of next result item in TreeView on enter
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
        QHBoxLayout * titleHL = new QHBoxLayout(rightVB);

        QLabel * optionName = new QLabel(rightVB); //view for selected item name
        QFont font(optionName->font().family(),12);//set font size 12
        font.setBold(true);//set font bold
        optionName->setFont(font);//apply font

        QPlainTextEdit * optionDescription = new QPlainTextEdit(rightVB); //view for selected item description
        optionDescription->setReadOnly(true); //for now description does need to be changed (no associated functionality implemented)
        attrBox = new QGridLayout(rightVB); //layout to dynamically view attributes

        saveButton = new QPushButton(tr("save"),rightVB);
        saveButton->setDisabled(true);
        QObject::connect(saveButton,SIGNAL(clicked()),this,SLOT(saveChanges()));

        QPushButton * openButton= new QPushButton(tr("open"),rightVB);
        QObject::connect(openButton,SIGNAL(clicked()),this,SLOT(openFileDialog()));

        resetButton= new QPushButton(tr("reset"),rightVB);
        resetButton->setDisabled(true);
        QObject::connect(resetButton,SIGNAL(clicked()),this,SLOT(openXMLFile()));

        //puting everything together

        titleHL->addWidget(optionName,5);//top (title)
        titleHL->addWidget(openButton,1);
        titleHL->addWidget(saveButton,1);
        titleHL->addWidget(resetButton,1);

        rightVL->addLayout(titleHL,1);
        rightVL->addWidget(optionDescription,3);//middle description
//        rightVL->setStretchFactor(optionDescription,3);
        rightVL->addLayout(attrBox,3);//middle attributes
        rightVL->setAlignment(attrBox,Qt::AlignTop);

        rightVB->setLayout(rightVL);
    //right side

    this->addWidget(leftVB);
    this->addWidget(rightVB);
    //arange initial dimensions
    this->setMinimumSize(800,600);
    this->setStretchFactor(0,1);
    this->setStretchFactor(1,3);

    //selection event in QTreeView (selectionModel -> right)
    QObject::connect(this, SIGNAL(labelChanged(QString)), optionName, SLOT(setText(QString))); //write name to label view
    QObject::connect(this, SIGNAL(decriptionChanged(QString)), optionDescription, SLOT(setPlainText(QString))); //write description to view
}

void XMLSettingsEditorWrapper::optionSelected(QModelIndex index){ //SLOT
    emit labelChanged(index.data(Qt::DisplayRole).toString()); //set new right-side-title
    emit decriptionChanged(model->description(index));//set new right side description
    //handling dynamic attribute view
        AttributeWidget * a;
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
            if (attr.first().key == QString("all")){//all attribute to preset a1 .. a6
                c++;
                //all
                a = new AttributeWidget(index, attr.first().key, attr.first().value, attrBox->parentWidget());//create custom AttributeWidget
                attrList->append(a);//save pointer in list for easy remove later
                attrBox->addWidget(a,r,c);//add to gridLayout
                QObject::connect(a,SIGNAL(attributeChanged(QModelIndex,QString,QString)),this,SLOT(datumChanged(QModelIndex,QString,QString)));//write back change of attribute to model

                //ai
                int ai = 1;//attr index
                for(int i = 1; i <= 6; i++){//generating all a1 .. a6
                    c++;
                    if (attr.value(ai).key == QString::number(i).prepend("a")){//ai is existing special value
                        a = new AttributeWidget(index, attr.value(ai).key, attr.value(ai).value, attrBox->parentWidget(), i);//create custom AttributeWidget
                        ai++;
                    }else{//ai is not specified (set to all-value)
                        a = new AttributeWidget(index, QString::number(i).prepend("a"), attr.first().value, attrBox->parentWidget(), i);//create custom AttributeWidget
                        a->setDisabled(true);
                    }
                    QObject::connect(a,SIGNAL(activateAttribute(QModelIndex,int)),this,SLOT(activateAttribute(QModelIndex,int)));//write back change of attribute to model
                    QObject::connect(a,SIGNAL(deactivateAttribute(QModelIndex,int,AttributeWidget*)),this,SLOT(deactivateAttribute(QModelIndex,int,AttributeWidget*)));//write back change of attribute to model
                    QObject::connect(a,SIGNAL(attributeChanged(QModelIndex,QString,QString)),this,SLOT(datumChanged(QModelIndex,QString,QString)));//write back change of attribute to model
                    attrList->append(a);//save pointer in list for easy remove later
                    attrBox->addWidget(a,r,c);//add to gridLayout
                    if(c>1){//added widget with c=2 now return to nex row
                        c = -1;
                        r++;
                    }
                }

            }else{ //no all-attribute

                QList<TreeItem::Attribute>::const_iterator i;
                for( i = attr.constBegin(); i != attr.constEnd(); i++){//adding all attributes as widgets to right-side attrBox (GridLayout) max 3 in a row
                    c++;
                    a = new AttributeWidget(index, (*i).key, (*i).value, attrBox->parentWidget());//create custom AttributeWidget
                    attrList->append(a);//save pointer in list for easy remove later
                    attrBox->addWidget(a,r,c);//add to gridLayout
                    QObject::connect(a,SIGNAL(attributeChanged(QModelIndex,QString,QString)),this,SLOT(datumChanged(QModelIndex,QString,QString)));//write back change of attribute to model
                    if(c>1){//added widget with c=2 now return to nex row
                        c = -1;
                        r++;
                    }
                }

            }
        }

        if(model->index(0,0,index).data() == QVariant("all")){//first child is an all-tag
            r++;
            QModelIndex childIndex = model->index(0,0,index);

            QString childName = childIndex.data().toString();

            //all
            c++;
            a = new AttributeWidget(childIndex, childName,QString(""), attrBox->parentWidget(),0,0,false); //all-label
            a->setDisabled(true);
            attrList->append(a);//save pointer in list for easy remove later
            attrBox->addWidget(a,r,c);//add to gridLayout

            //all attr
            allAttrList = model->attributes(childIndex);
            QList<TreeItem::Attribute>::const_iterator i;
            for( i = allAttrList.constBegin(); i != allAttrList.constEnd(); i++){//setup all-attributes
                c++;
                a = new AttributeWidget(childIndex, (*i).key, (*i).value, attrBox->parentWidget());//create custom AttributeWidget
                attrList->append(a);//save pointer in list for easy remove later
                attrBox->addWidget(a,r,c);//add to gridLayout
                QObject::connect(a,SIGNAL(attributeChanged(QModelIndex,QString,QString)),this,SLOT(datumChanged(QModelIndex,QString,QString)));//write back change of attribute to model
            }
            c = -1;
            r++;


            //ai
            int aai = 1;//allAttrList index
            for(int j = 1; j <= 6; j++){
                childIndex = model->index(aai,0,index);
                childName = QString::number(j).prepend("a");

                //ai toggle button
                c++;
                AttributeWidget *aib = new AttributeWidget(index, childName,QString(""), attrBox->parentWidget(), j,j,false); //all-label
                attrList->append(aib);//save pointer in list for easy remove later
                attrBox->addWidget(aib,r,c);//add to gridLayout
                QObject::connect(aib,SIGNAL(activateAttribute(QModelIndex,int)),this,SLOT(activateAttributeTag(QModelIndex,int)));//write back change of attribute to model
                QObject::connect(aib,SIGNAL(deactivateAttribute(QModelIndex,int,AttributeWidget*)),this,SLOT(deactivateAttributeTag(QModelIndex,int,AttributeWidget*)));//write back change of attribute to model

                //ai attr
                if (childName == childIndex.data()){//ai is specified
                    aai++;
                    attr = model->attributes(childIndex);//get a QList of attributes
                    int ai = 0;//attr index
                    for(int i = 0; i < allAttrList.count(); i++){//setup all-attributes
                        c++;
                        if (allAttrList.value(i).key == attr.value(ai).key){//ai has attr[aai]
                            a = new AttributeWidget(childIndex, attr.value(ai).key, attr.value(ai).value, attrBox->parentWidget(), j,i);//create custom AttributeWidget
                            ai++;
                        }else{//in ai no attr[aai] is specified
                            a = new AttributeWidget(childIndex, allAttrList.value(i).key, allAttrList.value(i).value, attrBox->parentWidget(), j,i);//create custom AttributeWidget
                            a->setDisabled(true);
                        }
                        QObject::connect(a,SIGNAL(activateTagAttribute(QModelIndex,int)),this,SLOT(activateAttribute(QModelIndex,int)));//write back change of attribute to model
                        QObject::connect(a,SIGNAL(deactivateTagAttribute(QModelIndex,int,AttributeWidget*)),this,SLOT(deactivateAttribute(QModelIndex,int,AttributeWidget*)));//write back change of attribute to model
                        QObject::connect(a,SIGNAL(attributeChanged(QModelIndex,QString,QString)),this,SLOT(datumChanged(QModelIndex,QString,QString)));//write back change of attribute to model
                        attrList->append(a);//save pointer in list for easy remove later
                        attrBox->addWidget(a,r,c);//add to gridLayout

                    }
                }else{
                    aib->setDisabled(true);
                }

                c = -1;
                r++;
            }
        }
    //handling dynamic attribute view
}

void XMLSettingsEditorWrapper::datumChanged(QModelIndex index, QString key, QString value) //SLOT
{
    model->changeAttribute(index,key,value);
    if (key == QString("all")){
        for (QList<AttributeWidget*>::iterator i = attrList->begin(); i < attrList->end(); i++){
            if(!(**i).isEnabled())
                (**i).setValue(value);
        }
    }
    if(model->data(index) == QString("all")){
        for (QList<AttributeWidget*>::iterator i = attrList->begin(); i < attrList->end(); i++){
            if(!(**i).isEnabled() && (**i).label() == key)
                (**i).setValue(value);
        }
    }
    saveButton->setEnabled(true);
    resetButton->setEnabled(true);
}

void XMLSettingsEditorWrapper::saveChanges() //SLOT
{
    sourceFile = sourceFile.append("~new.xml");//todo: save changes in files with suffix
    QFile file(sourceFile);
    if (model->save(file)){
        saveButton->setDisabled(true);
        resetButton->setDisabled(true);
    }
}

void XMLSettingsEditorWrapper::fastSearch(QString searchString)//SLOT
{
    if (model){
        resultList = model->findItems(searchString);
        lastIndex = 0;

        if (resultList.count() > 0){
            emit selectFound(resultList[lastIndex]);//select item in TreeView
            optionSelected(resultList[lastIndex]);//show selected item on right side
        }
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

void XMLSettingsEditorWrapper::openXMLFile()
{
    if (!sourceFile.isEmpty()){
        QFile file(sourceFile);
        try{
            model = new XmlTreeModel(file,this);
        }catch(QFile::FileError){
            model = 0;
        }

        QModelIndex ind = model->index(0,0).parent();

        Q_ASSERT_X(model->index(0,0).parent() == QModelIndex(),
                       "QAbstractItemView::setModel",
                       "The parent of a top level index should be invalid");

        if (model){
            tree->setModel(model); //connecting custom XMLTreeModel

            //initial selection
            tree->selectionModel()->setCurrentIndex(model->index(0,0),QItemSelectionModel::SelectCurrent);
            optionSelected(model->index(0,0));

            //handling selections of TreeView (left -> selectionModel)
            QItemSelectionModel * selection = new QItemSelectionModel(model);
            tree->setSelectionModel(selection);
            QObject::connect(selection,SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(selectionChanged(QModelIndex,QModelIndex)));


            saveButton->setDisabled(true);
            resetButton->setDisabled(true);
        }

    }
}

void XMLSettingsEditorWrapper::openFileDialog()//SLOT
{
    //creating new data model
    sourceFile = QFileDialog::getOpenFileName(0,"Open XML file", "../testdata/");
    openXMLFile();
}

void XMLSettingsEditorWrapper::selectionChanged(QModelIndex current, QModelIndex previous)
{
    optionSelected(current);
}

void XMLSettingsEditorWrapper::activateAttribute(QModelIndex index, int attributeIndex)
{
    if (model->attribute(index,0).key == QString("all"))
        model->insertAttribute(index,attributeIndex,QString::number(attributeIndex).prepend("a"),model->attribute(index,0).value);
    else
        model->insertAttribute(index,attributeIndex,model->attribute(model->parent(index),attributeIndex).value,model->attribute(model->parent(index),attributeIndex).value);

    saveButton->setEnabled(true);
    resetButton->setEnabled(true);
}

void XMLSettingsEditorWrapper::deactivateAttribute(QModelIndex index, int attributeIndex, AttributeWidget * widget)
{
    if (model->attribute(index,0).key == QString("all"))
        widget->setValue(model->attribute(index,0).value); //reset to all-value
    else
        widget->setValue(model->attribute(model->index(0,0,model->parent(index)),attributeIndex).value); //reset to all-value

    model->removeAttribute(index, attributeIndex);
    saveButton->setEnabled(true);
    resetButton->setEnabled(true);
}

void XMLSettingsEditorWrapper::activateAttributeTag(QModelIndex index, int attributeIndex)
{
    static_cast<TreeItem*>(index.internalPointer())->insertChild(attributeIndex,QString::number(attributeIndex).prepend("a"));
    optionSelected(index);
    saveButton->setEnabled(true);
    resetButton->setEnabled(true);
}

void XMLSettingsEditorWrapper::deactivateAttributeTag(QModelIndex index, int attributeIndex, AttributeWidget *widget)
{
    static_cast<TreeItem*>(index.internalPointer())->removeChild(attributeIndex);
    optionSelected(index);
    saveButton->setEnabled(true);
    resetButton->setEnabled(true);
}
