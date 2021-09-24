#ifndef TICKERLISTMODEL_H
#define TICKERLISTMODEL_H

#include<QString>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include "ticker.h"
#include "utilites.h"

class modelTickersList : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum eTickerState:int {Informant,Connected,NeededPipe,Halted};

    explicit modelTickersList(std::vector<Ticker> &v,std::vector<Market> &m,
                              std::map<int,std::pair<bool,std::chrono::time_point<std::chrono::steady_clock>>> &mS,
                              QObject *parent = nullptr):
        QAbstractTableModel{parent},vTickersLst{&v},vMarketsLst{&m},mBlinkedState{&mS},bGrayColorForInormants{false}{};

private:

    std::vector<Ticker> * vTickersLst; //init in const by ref
    std::vector<Market> * vMarketsLst; //init in const by ref
    std::map<int,std::pair<bool,std::chrono::time_point<std::chrono::steady_clock>>> *mBlinkedState; //init in const by ref
    std::map<int,int> mTickerState;

    bool bGrayColorForInormants;

private:
    QString getMarketNameByID(const int i) const ;

public:

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &/*indx*/=QModelIndex()) const  override{return 5;};

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    const Ticker & getTicker(const QModelIndex &index);

    bool setData(const QModelIndex &indx,const QVariant &value,int nRole=Qt::DisplayRole) override;
    bool setData(const QModelIndex& index,const Ticker &t,int role=Qt::DisplayRole);

    int AddRow(Ticker &m);
    bool removeRow(int indx,const QModelIndex &parent = QModelIndex());

    Qt::ItemFlags flags(const QModelIndex &indx) const override;

    bool searchTickerByQuikSign(const std::string &sSign, QModelIndex & indx);
    bool searchTickerByFinamSign(const std::string &sSign, QModelIndex & indx);
    bool searchTickerBySign(const std::string &sSign, QModelIndex & indx);
    bool searchTickerByPureSign(const std::string &sSign, QModelIndex & indx);
    bool searchTickerByTickerID(const int TickerID, QModelIndex & indx);

    void blinkTicker(int iTickerID);
    void setTickerState(int TickerID, eTickerState st);

    void setGrayColorForInformants(bool b);


signals:
    void dataRemoved(const Ticker &);
};

class TickerProxyListModel: public QSortFilterProxyModel
{
    Q_OBJECT

private:
    int iDefaultMarket;
    bool bFilterByActive;
    bool bFilterByOff;
    bool bFilterByUnallocate;
    bool bFilterByAllocate;

public:
    TickerProxyListModel (QObject *parent = nullptr): QSortFilterProxyModel(parent),iDefaultMarket(-1),
        bFilterByActive{false},
        bFilterByOff{false},
        bFilterByUnallocate{false},
        bFilterByAllocate{false}
    {};

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
        return  ((modelTickersList*)this->sourceModel())->getTicker(src_indx);
    };

    bool setData(const QModelIndex &index,const QVariant &value, int role = Qt::DisplayRole) override
    {
        QModelIndex src_indx =  mapToSource(index);
        return QSortFilterProxyModel::setData(src_indx,value,role);
    };
    bool setData(const QModelIndex& index,const Ticker &t,int role) {
        QModelIndex src_indx =  mapToSource(index);
        return  ((modelTickersList*)this->sourceModel())->setData(src_indx,t,role);
    };

    int AddRow(Ticker &t){
        int i_src= ((modelTickersList*)this->sourceModel())->AddRow(t);
        return mapFromSource(sourceModel()->index(i_src,0)).row();
    }
    bool removeRow(int i,const QModelIndex &parent = QModelIndex()){
        QModelIndex src_indx =  mapToSource(index(i,0));
        return ((modelTickersList*)this->sourceModel())->removeRow(src_indx.row(),parent);
    }

    bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const override
    {
        QModelIndex indx= sourceModel()->index(source_row, 0, source_parent);
        if(indx.isValid()){
            const Ticker & t  (((modelTickersList*)this->sourceModel())->getTicker(indx));
            if (iDefaultMarket >=0 && t.MarketID() != iDefaultMarket){
                return false;
            }
            if (bFilterByActive && !t.AutoLoad()){
                return false;
            }
            if (bFilterByOff && t.AutoLoad()){
                return false;
            }
            if (bFilterByUnallocate && trim(t.TickerSignQuik()).size() == 0){
                return false;
            }
            if (bFilterByAllocate && trim(t.TickerSignQuik()).size() != 0){
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

    void setFilterByOff(bool bFilter){
        if (bFilter != bFilterByOff){
            bFilterByOff = bFilter;
            this->invalidate();
        }
    }
    void setFilterByUnallocate(bool bFilter){
        if (bFilter != bFilterByUnallocate){
            bFilterByUnallocate = bFilter;
            this->invalidate();
        }
    }
    void setFilterByAllocate(bool bFilter){
        if (bFilter != bFilterByAllocate){
            bFilterByAllocate = bFilter;
            this->invalidate();
        }
    }

    bool searchTickerByQuikSign(const std::string &sSign, QModelIndex & indx){
        QModelIndex src_indx ;//=  mapToSource(index)
        bool bRet =((modelTickersList*)this->sourceModel())->searchTickerByQuikSign(sSign,src_indx);
        indx = mapFromSource(src_indx);
        if (indx.row() < 0 ) return false;
        return bRet;
    }
    bool searchTickerByPureSign(const std::string &sSign, QModelIndex & indx){
        QModelIndex src_indx ;//=  mapToSource(index)
        bool bRet =((modelTickersList*)this->sourceModel())->searchTickerByPureSign(sSign,src_indx);
        indx = mapFromSource(src_indx);
        if (indx.row() < 0 ) return false;
        return bRet;
    }
};


#endif // TICKERLISTMODEL_H
