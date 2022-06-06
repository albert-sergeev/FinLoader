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

#ifndef MODELSESSIONS_H
#define MODELSESSIONS_H

#include <QAbstractTableModel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QDate>
#include "market.h"

//inline const int DateRole {Qt::UserRole+1};
//inline const int TimeRole {Qt::UserRole+2};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief model class used in user interface to represent session list table of market
///
class modelSessions : public QStandardItemModel
{
    Q_OBJECT

protected:
    Market::SessionTable_type &sessionTable;
    //typedef  std::vector<std::pair<std::time_t,std::pair<std::time_t,std::vector<std::pair<std::time_t,std::time_t>>>>> SessionTable_type;
public:
    //---------------------------------------------------------------------
    explicit modelSessions(Market::SessionTable_type &Table, QObject *parent = nullptr):
        QStandardItemModel(parent), sessionTable{Table} {

        setSessionTable(Table);
    };
    //---------------------------------------------------------------------
    void setSessionTable(const Market::SessionTable_type &Table);
    void addNewPeriod   (const QModelIndex& indx = QModelIndex());
    void DeletePeriod   (const QModelIndex& indx = QModelIndex());
    void addNewTimeRange(const QModelIndex& indx,const bool bInsert = false);
    void DeleteTimeRange(const QModelIndex& indx);
    //---------------------------------------------------------------------
    Market::SessionTable_type getSessionTable() const;
    //---------------------------------------------------------------------
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    //---------------------------------------------------------------------
    // base interface
    bool setData(const QModelIndex &index,const QVariant &value,int nRole = Qt::DisplayRole) override;
    // ---------------------------------------------------------------------

protected:
    bool getTimeRangeAppend(const QModelIndex& indx,const QStandardItem *parent,QTime &t);
    bool getTimeRangeInsert(const QModelIndex& indx,const QStandardItem *parent,QTime &t);
    static QTime accomodateTimeRange(const QTime &,const bool bFirst = true);


};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief The modelSessionsProxy class
///
class modelSessionsProxy: public QSortFilterProxyModel
{
    Q_OBJECT

private:

public:
    //--------------------------------------------------------------------------------------------
    modelSessionsProxy (QObject *parent = nullptr): QSortFilterProxyModel(parent)
    {};
    //--------------------------------------------------------------------------------------------
    bool lessThan(const QModelIndex & L, const QModelIndex & R)  const override {

            QVariant qvL = sourceModel()->data(L);
            QVariant qvR = sourceModel()->data(R);

            if ( qvL.isValid()  && qvR.isValid()){
               if (qvL.canConvert<QDate>() && qvR.canConvert<QDate>()){
                   QDate dateL = qvariant_cast<QDate>(qvL);
                   QDate dateR = qvariant_cast<QDate>(qvR);
                   return dateL < dateR;
               }
               else if (qvL.canConvert<QTime>() && qvR.canConvert<QTime>()){
                   QTime dateL = qvariant_cast<QTime>(qvL);
                   QTime dateR = qvariant_cast<QTime>(qvR);
                   return dateL < dateR;
               }
            }
            return false;
    }
    //--------------------------------------------------------------------------------------------
    void addNewPeriod(const QModelIndex& indx = QModelIndex()){
        QModelIndex src_indx =  mapToSource(indx);
        return  qobject_cast<modelSessions *>(this->sourceModel())->addNewPeriod(src_indx);
    }
    //--------------------------------------------------------------------------------------------
    void DeletePeriod   (const QModelIndex& indx){
        QModelIndex src_indx =  mapToSource(indx);
        return  qobject_cast<modelSessions *>(this->sourceModel())->DeletePeriod(src_indx);
    }
    //--------------------------------------------------------------------------------------------
    void addNewTimeRange(const QModelIndex& indx, const bool bInsert = false){
        QModelIndex src_indx =  mapToSource(indx);
        return  qobject_cast<modelSessions *>(this->sourceModel())->addNewTimeRange(src_indx,bInsert);
    }
    //--------------------------------------------------------------------------------------------
    void DeleteTimeRange(const QModelIndex& indx ){
        QModelIndex src_indx =  mapToSource(indx);
        return  qobject_cast<modelSessions *>(this->sourceModel())->DeleteTimeRange(src_indx);
    }
    //--------------------------------------------------------------------------------------------
    bool setData(const QModelIndex &indx,const QVariant &value,int nRole = Qt::DisplayRole) override{
        QModelIndex src_indx =  mapToSource(indx);
        return  qobject_cast<modelSessions *>(this->sourceModel())->setData(src_indx,value,nRole);
    }
    //--------------------------------------------------------------------------------------------

};

#endif // MODELSESSIONS_H
