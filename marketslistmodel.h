#ifndef MARKETSLISTMODEL_H
#define MARKETSLISTMODEL_H

#include <QAbstractListModel>
#include <vector>
#include "ticker.h"

class MarketsListModel : public QAbstractTableModel
{
    Q_OBJECT

private:

    std::vector<Market> * vMarketsLst; //init in const by ref

public:

    explicit MarketsListModel(std::vector<Market> &v, QObject *parent = nullptr):QAbstractTableModel{parent},vMarketsLst{&v}{};

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &/*indx=QModelIndex()*/) const  override{return 7;};

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    //bool setData(const QModelIndex &indx,const QVariant &value,int nRole=Qt::DisplayRole) override;

    int AddRow(Market &m);
    bool removeItem(const int indx);


    Qt::ItemFlags flags(const QModelIndex &indx) const override;

    Market & getMarket(const QModelIndex &index);


private:
};

#endif // MARKETSLISTMODEL_H
