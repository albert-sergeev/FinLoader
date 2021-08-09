#ifndef TICKERLISTMODEL_H
#define TICKERLISTMODEL_H

#include<QString>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include "ticker.h"

class TickersListModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit TickersListModel(std::vector<Ticker> &v,std::vector<Market> &m, QObject *parent = nullptr):
        QAbstractTableModel{parent},vTickersLst{&v},vMarketsLst{&m}{};

private:

    std::vector<Ticker> * vTickersLst; //init in const by ref
    std::vector<Market> * vMarketsLst; //init in const by ref

private:
    QString getMarketNameByID(const int i) const ;

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

    bool searchTickerByFinamSign(const std::string &sSign, QModelIndex & indx);
    bool searchTickerBySign(const std::string &sSign, QModelIndex & indx);
    bool searchTickerByTickerID(const int TickerID, QModelIndex & indx);


signals:
    void dataRemoved(const Ticker &);
};

class TickerProxyListModel: public QSortFilterProxyModel
{
    Q_OBJECT

private:
    int iDefaultMarket;
    bool bFilterByActive;

public:
    TickerProxyListModel (QObject *parent = nullptr): QSortFilterProxyModel(parent),iDefaultMarket(-1),bFilterByActive{false}{};

    bool lessThan(const QModelIndex & L, const QModelIndex & R)  const override {

            QVariant qvL = sourceModel()->data(L);
            QVariant qvR = sourceModel()->data(R);

            if ( qvL.isValid()  && qvR.isValid()){
               if (qvL.userType()  == QVariant::String && qvR.userType()  == QVariant::String){
                  return qvL.toString() < qvR.toString();
               }
               else if (qvL.userType()  == QVariant::Int && qvR.userType()  == QVariant::Int){
                   return qvL.toInt() < qvR.toInt();
                }
            }
            return false;
    }

    const Ticker & getTicker(const QModelIndex &index){
        QModelIndex src_indx =  mapToSource(index);
        return  ((TickersListModel*)this->sourceModel())->getTicker(src_indx);
    };

    bool setData(const QModelIndex &index,const QVariant &value, int role = Qt::DisplayRole) override
    {
        QModelIndex src_indx =  mapToSource(index);
        return QSortFilterProxyModel::setData(src_indx,value,role);
    };
    bool setData(const QModelIndex& index,const Ticker &t,int role) {
        QModelIndex src_indx =  mapToSource(index);
        return  ((TickersListModel*)this->sourceModel())->setData(src_indx,t,role);
    };

    int AddRow(Ticker &t){
        int i_src= ((TickersListModel*)this->sourceModel())->AddRow(t);
        return mapFromSource(sourceModel()->index(i_src,0)).row();
    }
    bool removeRow(int i,const QModelIndex &parent = QModelIndex()){
        QModelIndex src_indx =  mapToSource(index(i,0));
        return ((TickersListModel*)this->sourceModel())->removeRow(src_indx.row(),parent);
    }

    bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const override
    {
        QModelIndex indx= sourceModel()->index(source_row, 0, source_parent);
        if(indx.isValid()){
            const Ticker & t  (((TickersListModel*)this->sourceModel())->getTicker(indx));
            if (iDefaultMarket >=0 && t.MarketID() != iDefaultMarket){
                return false;
            }
            if (bFilterByActive && !t.AutoLoad()){
                return false;
            }
            return true;
        }
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    void setDefaultMarket(int iMarket){
        if (iMarket != iDefaultMarket){
            iDefaultMarket = iMarket;
            this->invalidate();
        }
    }

    void setFilterByActive(bool bFilter){
        if (bFilter != bFilterByActive){
            bFilterByActive = bFilter;
            this->invalidate();
        }
    }


};


#endif // TICKERLISTMODEL_H
