////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2011 Mario Negri Institute & Orobix Srl
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include <QtGui>
#include "treeitem.h"
#include "treemodel.h"

TreeModel::TreeModel( QObject *parent)
     : QAbstractItemModel(parent)
{
   QVector<QVariant> rootData;
   QStringList headers;
   headers <<tr("Property")<< tr("Value")<<"att"<<"typ"<<"mbrs"<<"lftmbrs"<<"rmv"<<"RO"<<"+"<<"-"<<"enum"<<"line";
   foreach (QString header, headers)
      rootData << header;
   rootItem = new TreeItem(rootData);
}

TreeModel::~TreeModel()
{
   delete rootItem;
}

void TreeModel::setErrorLine(int line)
{
   errorLine = line;
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (role != Qt::EditRole)
      return false;

   TreeItem *item = getItem(index);
   bool result = item->setData(index.column(), value);
   if (result)
      emit dataChanged(index, index);

   return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
   if (role != Qt::EditRole || orientation != Qt::Horizontal)
      return false;

   bool result = rootItem->setData(section, value);
   if (result)
      emit headerDataChanged(orientation, section, section);

   return result;
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
   bool success;

   beginRemoveColumns(parent, position, position + columns - 1);
   success = rootItem->removeColumns(position, columns);
   endRemoveColumns();

   if (rootItem->columnCount() == 0)
      removeRows(0, rowCount());

   return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
   TreeItem *parentItem = getItem(parent);
   bool success = true;
   beginRemoveRows(parent, position, position + rows - 1);
   success = parentItem->removeChildren(position, rows);
   endRemoveRows();
   return success;
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
   bool success;
   beginInsertColumns(parent, position, position + columns - 1);
   success = rootItem->insertColumns(position, columns);
   endInsertColumns();
   return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
   TreeItem *parentItem = getItem(parent);
   bool success;
   beginInsertRows(parent, position, position + rows - 1);
   success = parentItem->insertChildren(position, rows, rootItem->columnCount());
   endInsertRows();
   return success;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
   return rootItem->columnCount();
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
   TreeItem *parentItem = getItem(parent);
   return parentItem->childCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
   if (!index.isValid())
      return QVariant();

   if (role == Qt::FontRole){
      if (index.column() == 0 ) {
         if (getItem(index)->data(2).toBool()){
            QFont italicFont;
            italicFont.setItalic(true);
	         return italicFont;
         }
      }
   }

    /* if (role == Qt::ForegroundRole){
         if (index.column() == 0 and !getItem(index)->data(2).toBool())
            return QBrush("#2244ee");
         if (index.column() == 0 and getItem(index)->data(2).toBool())
            return QBrush("#22aa44");
         if (index.column() == 1 and getItem(index)->data(2).toBool())
            return QBrush("#ff2244");
     }*/

   if (role == Qt::BackgroundRole){
      if (index.column() <= 1 and errorLine>0 and getItem(index)->data(11).toInt()==errorLine)
         return QBrush("#ee8080");
   }

   if (role == Qt::DecorationRole){
      if(index.column()==8 and !getItem(index)->data(2).toBool() and !getItem(index)->data(5).toStringList().isEmpty())
         return QIcon(":icons/add_child.png");
      if(index.column()==9 and !getItem(index)->data(6).toBool())
         return QIcon(":icons/remove_element.png");
   }

   if (role != Qt::DisplayRole && role != Qt::EditRole)
      return QVariant();

   TreeItem *item = getItem(index);
   return item->data(index.column());
}


Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
      return 0;
   if (index.column()==9) // (-)
      return Qt::ItemIsEnabled;

   if (index.column()==8 and !getItem(index)->data(5).toStringList().isEmpty()) //(+)
      return Qt::ItemIsEnabled | Qt::ItemIsEditable;

   if (index.column()!=1) // (value)
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

   if (getItem(index)->data(7).toBool())
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable ;

   if (!getItem(index)->data(2).toBool() and !getItem(index)->data(4).toStringList().isEmpty())
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable ;

   return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
   if (index.isValid()) {
      TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
      if (item) return item;
   }
   return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return rootItem->data(section);

   return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
   if (parent.isValid() && parent.column() != 0)
      return QModelIndex();

   TreeItem *parentItem = getItem(parent);

   TreeItem *childItem = parentItem->child(row);
   if (childItem)
      return createIndex(row, column, childItem);
   else
      return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
   if (!index.isValid())
      return QModelIndex();

   TreeItem *childItem = getItem(index);
   TreeItem *parentItem = childItem->parent();

   if (parentItem == rootItem)
      return QModelIndex();

   return createIndex(parentItem->childNumber(), 0, parentItem);
}

