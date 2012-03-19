#ifndef ATTRIBUTWIDGET_H
#define ATTRIBUTWIDGET_H

#include <xmltreemodel.h>

#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>

class AttributeWidget : public QWidget
{
    Q_OBJECT
public:
    AttributeWidget(QModelIndex index,const QString &label,const QString &value, QWidget *parent = 0, int attributeIndex = 0, int attributeTagIndex = 0, bool hasValue = true);
//    AttributWidget(QModelIndex index,const QString &label,const QString &value,int attributeIndex, QWidget *parent = 0);
    void setDisabled(bool disabled);
    bool isEnabled();
    QString label();

private:
    QModelIndex modelIndex;
    QPushButton * labelWidget;
    QLineEdit * valueWidget;
    int attributeIndex;
    int attributeTagIndex;

signals:
    void attributeChanged(QModelIndex,QString,QString);//indicating changed text of LineEdit -> model
    void activateAttribute(QModelIndex modelIndex, int attributeIndex);//if clicked to activate attributeWidget
    void deactivateAttribute(QModelIndex modelIndex, int attributeIndex, AttributeWidget * widget);//if clicked to activate attributeWidget
    void activateTagAttribute(QModelIndex modelIndex, int attributeIndex);//if clicked to activate attributeWidget
    void deactivateTagAttribute(QModelIndex modelIndex, int attributeIndex, AttributeWidget * widget);//if clicked to activate attributeWidget

public slots:
    void setValue(QString value);

private slots:
    void valueChanged(QString value);//slot for LineEdit indicating changed text
    void attributeToggled(bool checked);

};

#endif // ATTRIBUTWIDGET_H
