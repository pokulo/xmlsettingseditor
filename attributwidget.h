#ifndef ATTRIBUTWIDGET_H
#define ATTRIBUTWIDGET_H

#include <xmltreemodel.h>

#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

class AttributWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AttributWidget(QModelIndex index,const QString &label,const QString &value, QWidget *parent = 0);

private:
    QModelIndex modelIndex;
    QLabel * labelWidget;
    QLineEdit * valueWidget;

signals:
    void attributeChanged(QModelIndex,QString,QString);

public slots:
    void valueChanged(QString value);

};

#endif // ATTRIBUTWIDGET_H