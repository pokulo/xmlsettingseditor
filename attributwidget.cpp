#include "attributwidget.h"

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

