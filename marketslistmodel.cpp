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

    //this->rowsAboutToBeInserted
}
//--------------------------------------------------------------------------------------------------------
int MarketsListModel::AddRow(Market &m)
{
    beginInsertRows(QModelIndex(),(int)vMarketsLst->size(),(int)vMarketsLst->size());
    vMarketsLst->push_back(m);

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
bool MarketsListModel::removeItem(const int indx)
{
    if(indx>=0 && indx < (int)vMarketsLst->size()){

        std::copy(next(std::begin(*vMarketsLst),indx+1),end(*vMarketsLst),next(std::begin(*vMarketsLst)));
        //vMarketsLst->resize(vMarketsLst->size()-1);
        vMarketsLst->erase(next(std::begin(*vMarketsLst),vMarketsLst->size()-1));

        vMarketsLst->shrink_to_fit();


        emit dataChanged(this->index(indx,0),this->index(indx+1,0));
        return  true;
    }
    return  false;
}
//--------------------------------------------------------------------------------------------------------
//bool MarketsListModel::setData(const QModelIndex &indx,const QVariant &value,
//                        int nRole/*=Qt::DisplayRole*/)
//{
//    //if(false) {
//    if(indx.isValid()&&nRole==Qt::DisplayRole) {
//            //(*vMarketsLst)[indx.row()]
//            value.value<QString>();
//            emit dataChanged(indx,indx);
//            return  true;
//    }
//    else   return false;

//}
//--------------------------------------------------------------------------------------------------------

Qt::ItemFlags MarketsListModel::flags(const QModelIndex &indx)const
{
    Qt::ItemFlags flgs=QAbstractTableModel::flags(indx);
    if(indx.isValid()) return flgs/*|Qt::ItemIsEditable*/;
    else return flgs;
}
//--------------------------------------------------------------------------------------------------------
