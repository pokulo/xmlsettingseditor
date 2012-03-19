/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include <QStringList>

#include "treeitem.h"

TreeItem::TreeItem(const QString &name, TreeItem *parent)
{
    parentItem = parent;
    itemName = name;
    itemDescription = "";
}

TreeItem::TreeItem(const QString &name,const QList<Attribute> &attributes,const QString &description, TreeItem *parent){
    parentItem = parent;
    itemName = name;
    itemDescription = description;
    itemAttributes = attributes;
}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem *item)
{
    Q_ASSERT(item);//debug only
    childItems.append(item);
}

TreeItem *TreeItem::child(int row)
{
    return childItems.value(row);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

TreeItem *TreeItem::parent()
{
    return parentItem;
}

int TreeItem::row() const
{
    if (parentItem)//because parent of root is 0x0
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

QString TreeItem::name() const
{
    return itemName;
}

QString TreeItem::description() const
{
    return itemDescription;
}

void TreeItem::setDescription(QString description)
{
    itemDescription = description;
}

QList<TreeItem::Attribute> TreeItem::attributes() const
{
    return itemAttributes;
}

QString TreeItem::appendAttribute(const QString &key,const QString &value)
{
    QString ret("");
    if (!itemAttributes.isEmpty()){

        //test if attribute exists -> replace value
        QList<Attribute>::iterator i;
        for(i = itemAttributes.begin(); (i != itemAttributes.end()) && ret.isEmpty(); i++){
            if (key == (*i).key){//found attribute
                ret = (*i).value; //store old value
                (*i).value = value; //overwrite with new
            }
        }

    }

    //if attribute hasn't exist append a new one
    if (ret.isEmpty()){
        TreeItem::Attribute a;
        a.key = key;
        a.value = value;
        itemAttributes.append(a);
    }

    return ret;//return the old value (empty if attribute hasn't exist)
}

void TreeItem::insertAttribute(int index, const QString &key, const QString &value)
{
    TreeItem::Attribute a;
    a.key = key;
    a.value = value;
    if (index > 0){
        int i = 1;
        int j = 1;
        while (i < itemAttributes.count() && j < index){
            while (!itemAttributes.value(i).key.contains(QString::number(j)) && j < index){
                j++;
            }
            if (j != index)
                i++;
        }
        itemAttributes.insert(i,a);
    }else{
        itemAttributes.insert(index,a);
    }
}

TreeItem::Attribute TreeItem::attribute(int index) const
{
    return itemAttributes.value(index);
}

void TreeItem::removeAttribute(int index)
{
    if (index > 0){//only
        int i = 0;
        while (!itemAttributes.value(i).key.contains(QString::number(index)) && i < itemAttributes.count()){
            i++;
        }
        if (i <= itemAttributes.count())
            itemAttributes.removeAt(i);
    }else{
        itemAttributes.removeAt(index);
    }
}
