#include "tickerslistmodel.h"

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
//--------------------------------------------------------------------------------------------------------
Ticker & TickersListModel::getTicker(const QModelIndex &index)
{
    if(index.row() < 0    || index.row() >= (int)vTickersLst->size()) {
        throw std::invalid_argument("Index out of range {MarketsListModel::getMarket}");
    }
    return vTickersLst->at(index.row());

    //this->rowsAboutToBeInserted
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
bool TickersListModel::removeItem(const int indx)
{
    if(indx>=0 && indx < (int)vTickersLst->size()){

        std::copy(next(std::begin(*vTickersLst),indx+1),end(*vTickersLst),next(std::begin(*vTickersLst)));
        //vMarketsLst->resize(vMarketsLst->size()-1);
        vTickersLst->erase(next(std::begin(*vTickersLst),vTickersLst->size()-1));

        vTickersLst->shrink_to_fit();


        emit dataChanged(this->index(indx,0),this->index(indx,0));
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

Qt::ItemFlags TickersListModel::flags(const QModelIndex &indx)const
{
    Qt::ItemFlags flgs=QAbstractTableModel::flags(indx);
    if(indx.isValid()) return flgs/*|Qt::ItemIsEditable*/;
    else return flgs;
}
//--------------------------------------------------------------------------------------------------------
