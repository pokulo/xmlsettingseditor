#include "attributwidget.h"

AttributeWidget::AttributeWidget(QModelIndex index, const QString &label,const QString &value, QWidget *parent, int attributeIndex) : QWidget(parent){

    this->setAttribute(Qt::WA_DeleteOnClose);
    modelIndex = index;
    labelWidget = new QPushButton(label,this);
    static_cast<QPushButton*>(labelWidget)->setCheckable(true);
    if (attributeIndex != 0){
        labelWidget->setChecked(true);
    }else{
        labelWidget->setDisabled(true);
        labelWidget->setFlat(true);
    }

    valueWidget = new QLineEdit(value,this);
    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(labelWidget);
    layout->addWidget(valueWidget);
    this->setLayout(layout);
    QObject::connect(valueWidget,SIGNAL(textEdited(QString)),this,SLOT(valueChanged(QString)));
    QObject::connect(labelWidget,SIGNAL(clicked(bool)),this,SLOT(attributeToggled(bool)));
    this->attributeIndex = attributeIndex;
}

void AttributeWidget::valueChanged(QString value)//SLOT
{
    emit attributeChanged(modelIndex, labelWidget->text(), value);
}

void AttributeWidget::setDisabled(bool disabled)
{
    valueWidget->setDisabled(disabled);
    labelWidget->setChecked(false);
}

void AttributeWidget::attributeToggled(bool checked)
{
    valueWidget->setEnabled(checked);
    if (checked){
        emit activateAttribute(modelIndex,attributeIndex);
    }else{
        emit deactivateAttribute(modelIndex,attributeIndex, this);
    }
}

void AttributeWidget::setValue(QString value)
{
    valueWidget->setText(value);
}

bool AttributeWidget::isEnabled()
{
    return valueWidget->isEnabled();
}

