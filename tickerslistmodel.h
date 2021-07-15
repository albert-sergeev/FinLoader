#ifndef TICKERLISTMODEL_H
#define TICKERLISTMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
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
    const Ticker & getTicker(const QModelIndex &index);

    bool setData(const QModelIndex &indx,const QVariant &value,int nRole=Qt::DisplayRole) override;
    bool setData(const QModelIndex& index,const Ticker &t,int role=Qt::DisplayRole);

    int AddRow(Ticker &m);
    bool removeRow(int indx,const QModelIndex &parent = QModelIndex());

    Qt::ItemFlags flags(const QModelIndex &indx) const override;



signals:
    void dataRemoved(const Ticker &);
};

class TickerProxyListModel: public QSortFilterProxyModel
{
    Q_OBJECT

private:
    int iDefaultMarket;

public:
    TickerProxyListModel (QObject *parent = nullptr): QSortFilterProxyModel(parent){};

//    bool lessThan(const QModelIndex &, const QModelIndex &) const {
//          return false;
//       }

    const Ticker & getTicker(const QModelIndex &index){
        return  ((TickersListModel*)this->sourceModel())->getTicker(index);
    };

    bool setData(const QModelIndex &index,const QVariant &value, int role = Qt::DisplayRole) override
    {
        return QSortFilterProxyModel::setData(index,value,role);
    };
    bool setData(const QModelIndex& index,const Ticker &t,int role) {
        return  ((TickersListModel*)this->sourceModel())->setData(index,t,role);
    };

    int AddRow(Ticker &t){
        return ((TickersListModel*)this->sourceModel())->AddRow(t);
    }
    bool removeRow(int indx,const QModelIndex &parent = QModelIndex()){
        return ((TickersListModel*)this->sourceModel())->removeRow(indx,parent);
    }

    bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const override
    {
        QModelIndex indx= sourceModel()->index(source_row, 0, source_parent);
        if(indx.isValid()){
            return ((TickersListModel*)this->sourceModel())->getTicker(indx).MarketID() == iDefaultMarket;
        }
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    void setDefaultMarket(int iMarket){
        if (iMarket != iDefaultMarket){
            iDefaultMarket = iMarket;
            this->invalidate();
        }
    }

};


#endif // TICKERLISTMODEL_H
