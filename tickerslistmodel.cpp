#include "tickerslistmodel.h"
#include<iostream>

//TickerListModel::TickerListModel()
//{

//}


QVariant TickersListModel::headerData(int nSection, Qt::Orientation orientation, int nRole) const
{
    if(nRole!=Qt::DisplayRole){
        return QVariant();
    }
    if(orientation==Qt::Horizontal){
        return QString(tr("Ticker"));
    }
    else{
        return QString::number(nSection);
    }
}
//--------------------------------------------------------------------------------------------------------
int TickersListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    return (int)vTickersLst->size();
}
//--------------------------------------------------------------------------------------------------------
QVariant TickersListModel::data(const QModelIndex &index, int nRole) const
{
    if (!index.isValid())
        return QVariant();

    if(index.row() < 0    || index.row() >= (int)vTickersLst->size()) return QVariant();
    if(index.column() < 0 || index.column() >= 7) return QVariant();

    if(nRole == Qt::DisplayRole || nRole==Qt::EditRole){

        if(index.column()       ==  1){
            return QVariant::fromValue(QString::number(vTickersLst->at(index.row()).TickerID()));
        }
        else if(index.column()  ==  0){
            return QVariant::fromValue(QString::fromStdString(vTickersLst->at(index.row()).TickerName()));
        }
        else if(index.column()  ==  2){
            return QVariant::fromValue(QString::fromStdString(vTickersLst->at(index.row()).TickerSign()));
        }
    }
    return  QVariant();
}
////--------------------------------------------------------------------------------------------------------
const Ticker & TickersListModel::getTicker(const QModelIndex &index)
{
    if(index.row() < 0    || index.row() >= (int)vTickersLst->size()) {
        throw std::invalid_argument("Index out of range {MarketsListModel::getMarket}");
    }
    return vTickersLst->at(index.row());
}
//--------------------------------------------------------------------------------------------------------
bool TickersListModel::setData(const QModelIndex &index,const QVariant &value,int nRole)
{
        return QAbstractTableModel::setData(index,value,nRole);
}
//--------------------------------------------------------------------------------------------------------

bool TickersListModel::setData(const QModelIndex& index,const Ticker &t,int role)
{
    if(index.isValid() && role == Qt::EditRole){
        (*vTickersLst)[index.row()] = t;
        emit dataChanged(index,index);
    }
    return  0;
}
//--------------------------------------------------------------------------------------------------------
int TickersListModel::AddRow(Ticker &t)
{
    beginInsertRows(QModelIndex(),(int)vTickersLst->size(),(int)vTickersLst->size());
    vTickersLst->push_back(t);

    auto indx(this->index((int)vTickersLst->size()-1,0));
    if(indx.isValid()){
        emit dataChanged(indx,indx);
        return  indx.row();
    }
    else{
        throw std::runtime_error("Error adding market!");
    }
    endInsertRows();
    return  0;
}

//--------------------------------------------------------------------------------------------------------
bool TickersListModel::removeRow(int indx,const QModelIndex &parent )
{

    if(parent.isValid()){
        return false;
    }

    beginRemoveRows(parent,indx,indx);

    if(indx>=0 && indx < (int)vTickersLst->size()){

        Ticker t {(*vTickersLst)[indx]};
        //std::cout <<"["<<indx<<"] " << t.MarketID() << "," << t.TickerID() << "," << t.TickerName() << "," << t.TickerSign() << "\n";
        vTickersLst->erase(next(std::begin(*vTickersLst),indx));

        vTickersLst->shrink_to_fit();

        endRemoveRows();

        emit dataRemoved(t);
        return true;
    }
    else{
        endRemoveRows();
        return false;
    }
}

Qt::ItemFlags TickersListModel::flags(const QModelIndex &indx)const
{
    Qt::ItemFlags flgs=QAbstractTableModel::flags(indx);
    if(indx.isValid()) return flgs/*|Qt::ItemIsEditable*/;
    else return flgs;
}
//--------------------------------------------------------------------------------------------------------
