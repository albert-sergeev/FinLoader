#ifndef MODELSESSIONS_H
#define MODELSESSIONS_H

#include <QAbstractTableModel>
#include <QStandardItemModel>
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
    //---------------------------------------------------------------------
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

};

#endif // MODELSESSIONS_H
