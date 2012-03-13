#include "attributwidget.h"

AttributWidget::AttributWidget(QModelIndex index, const QString &label,const QString &value, QWidget *parent) : QWidget(parent){

    this->setAttribute(Qt::WA_DeleteOnClose);
    modelIndex = index;
    labelWidget = new QLabel(label,this);
    valueWidget = new QLineEdit(value,this);
    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(labelWidget);
    layout->addWidget(valueWidget);
    this->setLayout(layout);
    QObject::connect(valueWidget,SIGNAL(textChanged(QString)),this,SLOT(valueChanged(QString)));

}

void AttributWidget::valueChanged(QString value)//SLOT
{
    emit attributeChanged(modelIndex, labelWidget->text(), value);
}

