#ifndef TICKERLISTMODEL_H
#define TICKERLISTMODEL_H

#include <QAbstractTableModel>
#include "ticker.h"

class TickersListModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit TickersListModel(std::vector<Ticker> &v, QObject *parent = nullptr):QAbstractTableModel{parent},vTickersLst{&v}{};

private:

    std::vector<Ticker> * vTickersLst; //init in const by ref

public:

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &/*indx=QModelIndex()*/) const  override{return 7;};

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    //bool setData(const QModelIndex &indx,const QVariant &value,int nRole=Qt::DisplayRole) override;

    int AddRow(Ticker &m);
    bool removeItem(const int indx);


    Qt::ItemFlags flags(const QModelIndex &indx) const override;

    Ticker & getTicker(const QModelIndex &index);
};

#endif // TICKERLISTMODEL_H
