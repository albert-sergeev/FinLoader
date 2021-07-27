#include "marketslistmodel.h"
#include<iostream>
#include<algorithm>
//--------------------------------------------------------------------------------------------------------
/*
MarketsListModel::MarketsListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}*/
//--------------------------------------------------------------------------------------------------------
QVariant MarketsListModel::headerData(int nSection, Qt::Orientation orientation, int nRole) const
{
    if(nRole!=Qt::DisplayRole){
        return QVariant();
    }
    if(orientation==Qt::Horizontal){
        return QString(tr("Market"));
    }
    else{
        return QString::number(nSection);
    }
}
//--------------------------------------------------------------------------------------------------------
int MarketsListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    return (int)vMarketsLst->size();
}
//--------------------------------------------------------------------------------------------------------
QVariant MarketsListModel::data(const QModelIndex &index, int nRole) const
{
    if (!index.isValid())
        return QVariant();

    if(index.row() < 0    || index.row() >= (int)vMarketsLst->size()) return QVariant();
    if(index.column() < 0 || index.column() >= 7) return QVariant();

    if(nRole == Qt::DisplayRole || nRole==Qt::EditRole){

        if(index.column()       ==  1){
            return QVariant::fromValue(QString::number(vMarketsLst->at(index.row()).MarketID()));
        }
        else if(index.column()  ==  0){
            return QVariant::fromValue(QString::fromStdString(vMarketsLst->at(index.row()).MarketName()));
        }
        else if(index.column()  ==  2){
            return QVariant::fromValue(QString::fromStdString(vMarketsLst->at(index.row()).MarketSign()));
        }
    }
    return  QVariant();
}
//--------------------------------------------------------------------------------------------------------
Market & MarketsListModel::getMarket(const QModelIndex &index)
{
    if(index.row() < 0    || index.row() >= (int)vMarketsLst->size()) {
        throw std::invalid_argument("Index out of range {MarketsListModel::getMarket}");
    }
    return vMarketsLst->at(index.row());
}
//--------------------------------------------------------------------------------------------------------
int MarketsListModel::AddRow(Market &m)
{
    beginInsertRows(QModelIndex(),(int)vMarketsLst->size(),(int)vMarketsLst->size());
    vMarketsLst->push_back(m);
    endInsertRows();

    auto indx(this->index((int)vMarketsLst->size()-1,0));
    if(indx.isValid()){
        emit dataChanged(indx,indx);
        return  indx.row();
    }
    else{
        throw std::runtime_error("Error adding market!");
    }

    return  0;
}

//--------------------------------------------------------------------------------------------------------
bool MarketsListModel::removeRow(int indx,const QModelIndex &parent )
{
    if(parent.isValid()){
        return false;
    }

    beginRemoveRows(parent,indx,indx);

    if(indx>=0 && indx < (int)vMarketsLst->size()){

        Market t {(*vMarketsLst)[indx]};

        vMarketsLst->erase(next(std::begin(*vMarketsLst),indx));

        vMarketsLst->shrink_to_fit();

        endRemoveRows();

        return true;
    }
    else{
        endRemoveRows();
        return false;
    }
}
//bool MarketsListModel::removeItem(const int indx)
//{
//    if(indx>=0 && indx < (int)vMarketsLst->size()){


//        vMarketsLst->erase(next(std::begin(*vMarketsLst),indx));
//        vMarketsLst->shrink_to_fit();
//        emit dataChanged(this->index(indx,0),this->index(indx+1,0));
//        return  true;
//    }
//    return  false;
//}

//--------------------------------------------------------------------------------------------------------

Qt::ItemFlags MarketsListModel::flags(const QModelIndex &indx)const
{
    Qt::ItemFlags flgs=QAbstractTableModel::flags(indx);
    if(indx.isValid()) return flgs/*|Qt::ItemIsEditable*/;
    else return flgs;
}
//--------------------------------------------------------------------------------------------------------
bool MarketsListModel::searchMarketByMarketID(const int MarketID, QModelIndex & indx)
{
    auto It = std::find_if(vMarketsLst->begin(),vMarketsLst->end(),[&](const auto &c){
                        return c.MarketID() == MarketID;
    });
    if(It == vMarketsLst->end()){
        return false;
    }
    /////////////////

    indx = this->index(std::distance(vMarketsLst->begin(),It),0);

    return  true;
}
//--------------------------------------------------------------------------------------------------------
