#ifndef MODELSESSIONS_H
#define MODELSESSIONS_H

#include <QAbstractTableModel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QDate>
#include "market.h"

//inline const int DateRole {Qt::UserRole+1};
//inline const int TimeRole {Qt::UserRole+2};

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
    void addNewTimeRange(const QModelIndex& indx);
    void DeleteTimeRange(const QModelIndex& indx);
    //---------------------------------------------------------------------
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    //---------------------------------------------------------------------
    // base interface
    bool setData(const QModelIndex &index,const QVariant &value,int nRole = Qt::DisplayRole) override;

};

class modelSessionsProxy: public QSortFilterProxyModel
{
    Q_OBJECT

private:

public:
    modelSessionsProxy (QObject *parent = nullptr): QSortFilterProxyModel(parent)
    {};
    ////////////////////////////
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
};

#endif // MODELSESSIONS_H
