#include "xmlsettingseditor.h"

AttributeWidget::AttributeWidget(QModelIndex index, const QString &label,const QString &value, QWidget *parent, int attributeIndex, int attributeTagIndex,bool hasValue) : QWidget(parent){

    this->setAttribute(Qt::WA_DeleteOnClose);
    modelIndex = index;
    labelWidget = new QPushButton(label,this);
    labelWidget->setMinimumWidth(30);
    labelWidget->setCheckable(true);
    if (attributeIndex != 0){
        labelWidget->setChecked(true);
    }else{
        labelWidget->setDisabled(true);
        labelWidget->setFlat(true);
    }

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(labelWidget);
    QObject::connect(labelWidget,SIGNAL(clicked(bool)),this,SLOT(attributeToggled(bool)));


    if (hasValue){
        valueWidget = new QLineEdit(value,this);
        valueWidget->setMinimumWidth(50);
        layout->addWidget(valueWidget);
        QObject::connect(valueWidget,SIGNAL(textEdited(QString)),this,SLOT(valueChanged(QString)));
    }else{
        valueWidget = 0;
    }

    this->setLayout(layout);

    this->attributeIndex = attributeIndex;
    this->attributeTagIndex = attributeTagIndex;
}

void AttributeWidget::valueChanged(QString value)//SLOT
{
    emit attributeChanged(modelIndex, labelWidget->text(), value);
}

void AttributeWidget::setDisabled(bool disabled)
{
    if (valueWidget)
        valueWidget->setDisabled(disabled);
    labelWidget->setChecked(false);
}

void AttributeWidget::attributeToggled(bool checked)
{
    if (valueWidget)
        valueWidget->setEnabled(checked);
    if (checked){
        emit activateAttribute(modelIndex,attributeIndex);
        emit activateTagAttribute(modelIndex,attributeTagIndex);
    }else{
        emit deactivateAttribute(modelIndex,attributeIndex, this);
        emit deactivateTagAttribute(modelIndex,attributeTagIndex, this);
    }
}

void AttributeWidget::setValue(QString value)//SLOT
{
    if (valueWidget)
        valueWidget->setText(value);
}

bool AttributeWidget::isEnabled()
{
    return labelWidget->isChecked();
}

QString AttributeWidget::label()
{
    return labelWidget->text();
}



XMLSettingsEditor::XMLSettingsEditor(QWidget *parent) : QSplitter(parent)
{
    init(); //initialising members (defined in header)

    //left side
        QWidget * leftVB = new QWidget(this); //right-side-description/edit-widget
        QVBoxLayout * leftVL = new QVBoxLayout(leftVB); //->vertical layouted

        QHBoxLayout * searchHB = new QHBoxLayout(leftVB);
        QLineEdit * searchInput = new QLineEdit(leftVB);//fast search bar
        QPushButton * searchButton= new QPushButton(QIcon(":/xmleditor/icons/search"),"",leftVB);

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

        backwardButton = new QPushButton(QIcon(":/xmleditor/icons/back"),"",rightVB);
        backwardButton->setToolTip(tr("backwards selection"));
        backwardButton->setDisabled(true);
        QObject::connect(backwardButton,SIGNAL(clicked()),this,SLOT(goBackwards()));

        forwardButton = new QPushButton(QIcon(":/xmleditor/icons/forward"),"",rightVB);
        forwardButton->setToolTip(tr("forward selection"));
        forwardButton->setDisabled(true);
        QObject::connect(forwardButton,SIGNAL(clicked()),this,SLOT(goForwards()));

        saveButton = new QPushButton(QIcon(":/xmleditor/icons/save"),"",rightVB);
        saveButton->setDisabled(true);
        saveButton->setToolTip(tr("save"));
        QObject::connect(saveButton,SIGNAL(clicked()),this,SLOT(saveChanges()));

        QPushButton * openButton= new QPushButton(QIcon(":/xmleditor/icons/open"),"",rightVB);
        openButton->setToolTip(tr("open XML-file"));
        QObject::connect(openButton,SIGNAL(clicked()),this,SLOT(openFileDialog()));

        resetButton= new QPushButton(QIcon(":/xmleditor/icons/reset"),"",rightVB);
        resetButton->setDisabled(true);
        resetButton->setToolTip(tr("reload data from file"));
        QObject::connect(resetButton,SIGNAL(clicked()),this,SLOT(openXMLFile()));

        //puting everything together

        titleHL->addWidget(optionName,5);//top (title)
        titleHL->addWidget(backwardButton,1);
        titleHL->addWidget(forwardButton,1);
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

void XMLSettingsEditor::optionSelected(QModelIndex index){ //SLOT
    if (history.indexOf(index) < 0 || abs(history.indexOf(index)-lastSelected)>1){
        while (history.count()-1 > lastSelected)//cut history list from lastSelected
            history.takeLast();
        lastSelected = history.count();//save new position
        history.append(index);//save new index
        backwardButton->setEnabled(true);
    }

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

void XMLSettingsEditor::datumChanged(QModelIndex index, QString key, QString value) //SLOT
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

void XMLSettingsEditor::saveChanges() //SLOT
{
    QFile file(sourceFile);
    if (model->save(file)){
        saveButton->setDisabled(true);
        resetButton->setDisabled(true);
    }
}

void XMLSettingsEditor::fastSearch(QString searchString)//SLOT
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

void XMLSettingsEditor::nextFound()//SLOT
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

void XMLSettingsEditor::openXMLFile()
{
    if (!sourceFile.isEmpty()){
        QFile file(sourceFile);
        try{
            model = new XmlTreeModel(file,this);
        }catch(QFile::FileError){
            model = 0;
        }

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


            history.clear();
            lastSelected = -1;
            backwardButton->setDisabled(true);
            forwardButton->setDisabled(true);
            saveButton->setDisabled(true);
            resetButton->setDisabled(true);
        }

    }
}

void XMLSettingsEditor::openFileDialog()//SLOT
{
    //creating new data model
    sourceFile = QFileDialog::getOpenFileName(0,"Open XML file", "../testdata/");
    openXMLFile();
}

void XMLSettingsEditor::selectionChanged(QModelIndex current, QModelIndex previous)
{
    optionSelected(current);
}

void XMLSettingsEditor::activateAttribute(QModelIndex index, int attributeIndex)
{
    if (model->attribute(index,0).key == QString("all"))
        model->insertAttribute(index,attributeIndex,QString::number(attributeIndex).prepend("a"),model->attribute(index,0).value);
    else{

        QModelIndex all = model->parent(index).child(0,0);
        if (attributeIndex > 0){
            int i = 1;
            int j = 1;
            while (i < model->attributes(index).count() && j < attributeIndex){
                while (!(model->attribute(index,i).key == model->attribute(all,j).key) && j < attributeIndex){
                    j++;
                }
                if (j != attributeIndex)
                    i++;
            }
            model->insertAttribute(index,i,allAttrList.value(attributeIndex).key,allAttrList.value(attributeIndex).value);
        }else{
            model->insertAttribute(index,attributeIndex,allAttrList.value(attributeIndex).key,allAttrList.value(attributeIndex).value);
        }
    }

    saveButton->setEnabled(true);
    resetButton->setEnabled(true);
}

void XMLSettingsEditor::deactivateAttribute(QModelIndex index, int attributeIndex, AttributeWidget * widget)
{
    if (model->attribute(index,0).key == QString("all")){
        widget->setValue(model->attribute(index,0).value); //reset to all-value
        int i = 0;
        while(!model->attribute(index,i).key.contains(QString::number(attributeIndex)))
            i++;
        if (i < model->attributes(index).count())
            model->removeAttribute(index, i);

    }else{
        widget->setValue(model->attribute(model->index(0,0,model->parent(index)),attributeIndex).value); //reset to all-value
        int i = 0;
        while(model->attribute(index,i).key != allAttrList.value(attributeIndex).key)
            i++;
        if (i < model->attributes(index).count())
            model->removeAttribute(index, i);

    }
    saveButton->setEnabled(true);
    resetButton->setEnabled(true);
}

void XMLSettingsEditor::activateAttributeTag(QModelIndex index, int attributeIndex)
{
    static_cast<TreeItem*>(index.internalPointer())->insertChild(attributeIndex,QString::number(attributeIndex).prepend("a"));
    optionSelected(index);
    saveButton->setEnabled(true);
    resetButton->setEnabled(true);
}

void XMLSettingsEditor::deactivateAttributeTag(QModelIndex index, int attributeIndex, AttributeWidget *widget)
{
    static_cast<TreeItem*>(index.internalPointer())->removeChild(attributeIndex);
    optionSelected(index);
    saveButton->setEnabled(true);
    resetButton->setEnabled(true);
}

void XMLSettingsEditor::goBackwards()
{
    if (history.count() > 0 && lastSelected > 0){
        lastSelected--;
        tree->setCurrentIndex(history.value(lastSelected));
        forwardButton->setEnabled(true);
    }
    if (lastSelected==0)
        backwardButton->setDisabled(true);
}

void XMLSettingsEditor::goForwards()
{
    if (history.count() > lastSelected+1){
        lastSelected++;
        tree->setCurrentIndex(history.value(lastSelected));
        backwardButton->setEnabled(true);
    }
    if (history.count() == lastSelected+1)
        forwardButton->setDisabled(true);
}
