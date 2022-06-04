/****************************************************************************
*  This is part of FinLoader
*  Copyright (C) 2021  Albert Sergeyev
*  Contact: albert.s.sergeev@mail.ru
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

#ifndef MARKETSLISTMODEL_H
#define MARKETSLISTMODEL_H

#include <QAbstractListModel>
#include <vector>
#include "ticker.h"

class modelMarketsList : public QAbstractTableModel
{
    Q_OBJECT

private:

    std::vector<Market> * vMarketsLst; //init in const by ref

public:

    explicit modelMarketsList(std::vector<Market> &v, QObject *parent = nullptr):QAbstractTableModel{parent},vMarketsLst{&v}{};

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &/*indx=QModelIndex()*/) const  override{return 7;};

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int AddRow(Market &m);
    bool removeRow(int indx,const QModelIndex &parent = QModelIndex());
    //bool removeItem(const int indx);

    Qt::ItemFlags flags(const QModelIndex &indx) const override;

    Market & getMarket(const QModelIndex &index);

    bool searchMarketByMarketID(const int MarketID, QModelIndex & indx);

};

#endif // MARKETSLISTMODEL_H
