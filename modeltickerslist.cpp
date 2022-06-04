/****************************************************************************
*  This is part of FinLoader
*  Copyright (C) 2021  Albert Sergeyev
*  Contact: albert.s.sergeev@mail.ru
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

#include "modeltickerslist.h"
#include "storage.h"
#include<iostream>

#include<QColor>

//TickerListModel::TickerListModel()
//{

//}


QVariant modelTickersList::headerData(int nSection, Qt::Orientation orientation, int nRole) const
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
int modelTickersList::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    return (int)vTickersLst->size();
}
//--------------------------------------------------------------------------------------------------------
QVariant modelTickersList::data(const QModelIndex &index, int nRole) const
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
        else if(index.column()  ==  3){
            const Ticker & t ( vTickersLst->at(index.row()));

            QString qstrN = QString::fromStdString(trim(t.TickerName()));
            qstrN = qstrN.leftJustified(20, ' ', true);

            return QVariant::fromValue(qstrN+"\t{"+getMarketNameByID(t.MarketID())+"}");
        }
        else if(index.column()  ==  4){
            const Ticker & t ( vTickersLst->at(index.row()));
            return QVariant::fromValue(QString::fromStdString(t.TickerSign())+"\t{"
                                       +getMarketNameByID(t.MarketID())+"}");
        }
    }

    if(nRole == Qt::BackgroundColorRole
            ){
        const Ticker & t ( vTickersLst->at(index.row()));
        auto It (mBlinkedState->find(t.TickerID()));
        if (It != mBlinkedState->end() && It->second.first){
            //QColor colorDarkGreen(0, 100, 52,200);
            QColor colorDarkGreen(0, 100, 52,150);
            return QVariant(colorDarkGreen);
            //return QVariant(QColor(Qt::green));
        }
        else{
            auto ItM = mTickerState.find(t.TickerID());
            if (ItM != mTickerState.end()){
                switch (ItM->second) {
                case eTickerState::Informant:
                    if (bGrayColorForInormants) {return QVariant(QColor(Qt::lightGray));}
                    else                        {return QVariant();}
                    break;
                case eTickerState::Connected:
                    return  QVariant();
                    break;
                case eTickerState::NeededPipe:
                    return QVariant(QColor(Qt::magenta));
                    break;
                case eTickerState::Halted:
                    return QVariant(QColor(Qt::red));
                    break;
                }
            }
            else{
                if (bGrayColorForInormants) {return QVariant(QColor(Qt::lightGray));}
                else                        {return QVariant();}
            }
        }

    }
    return  QVariant();
}
////--------------------------------------------------------------------------------------------------------
void modelTickersList::blinkTicker(int TickerID){

    auto It = std::find_if(vTickersLst->begin(),vTickersLst->end(),[&](const Ticker &t){
                        return t.TickerID() == TickerID;});
    if (It != vTickersLst->end()){
        QVector<int> vV {Qt::BackgroundColorRole};
        QModelIndex indexBeg = this->index(std::distance(vTickersLst->begin(),It),0);
        QModelIndex indexEnd = this->index(std::distance(vTickersLst->begin(),It),4);
        emit dataChanged(indexBeg,indexEnd,vV);
    }

}
////--------------------------------------------------------------------------------------------------------
void modelTickersList::setTickerState(int TickerID, eTickerState st)
{
    auto It = std::find_if(vTickersLst->begin(),vTickersLst->end(),[&](const Ticker &t){
                        return t.TickerID() == TickerID;});
    if (It != vTickersLst->end()){
        auto ItState = mTickerState.find(TickerID);
        if (!(ItState != mTickerState.end() && ItState->second == st)){
            mTickerState[TickerID] = st;

            QVector<int> vV {Qt::BackgroundColorRole};
            QModelIndex indexBeg = this->index(std::distance(vTickersLst->begin(),It),0);
            QModelIndex indexEnd = this->index(std::distance(vTickersLst->begin(),It),4);

            emit dataChanged(indexBeg,indexEnd,vV);
        }
    }
}
////--------------------------------------------------------------------------------------------------------
QString modelTickersList::getMarketNameByID(const int ID) const
{
    for (const Market &m: *vMarketsLst){
        if (m.MarketID() == ID){
            return QString::fromStdString( m.MarketSign());
        }
    }
    return "";
}
////--------------------------------------------------------------------------------------------------------
const Ticker & modelTickersList::getTicker(const QModelIndex &index)
{
    if(index.row() < 0    || index.row() >= (int)vTickersLst->size()) {
        {
            ThreadFreeCout pcout;
            pcout << "Index out of range {modelTickersList::getTicker}\n";
            pcout << "index.row(): "<<index.row()<<"\n";
            pcout << "vTickersLst->size(): "<<vTickersLst->size()<<"\n";

        }
        throw std::invalid_argument("Index out of range {modelTickersList::getTicker}");
    }
    return vTickersLst->at(index.row());
}
//--------------------------------------------------------------------------------------------------------
bool modelTickersList::setData(const QModelIndex &index,const QVariant &value,int nRole)
{
        return QAbstractTableModel::setData(index,value,nRole);
}
//--------------------------------------------------------------------------------------------------------

bool modelTickersList::setData(const QModelIndex& index,const Ticker &t,int role)
{
    if(index.isValid() && role == Qt::EditRole){
        if(!(*vTickersLst)[index.row()].equal(t)){
            (*vTickersLst)[index.row()] = t;
            emit dataChanged(index,index);
        }
    }
    return  0;
}
//--------------------------------------------------------------------------------------------------------
int modelTickersList::AddRow(Ticker &t)
{
    beginInsertRows(QModelIndex(),(int)vTickersLst->size(),(int)vTickersLst->size());
    vTickersLst->push_back(t);
    endInsertRows();

    auto indx(this->index((int)vTickersLst->size()-1,0));
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
bool modelTickersList::removeRow(int indx,const QModelIndex &parent )
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
//--------------------------------------------------------------------------------------------------------
Qt::ItemFlags modelTickersList::flags(const QModelIndex &indx)const
{
    Qt::ItemFlags flgs=QAbstractTableModel::flags(indx);
    if(indx.isValid()) return flgs/*|Qt::ItemIsEditable*/;
    else return flgs;
}
//--------------------------------------------------------------------------------------------------------
bool modelTickersList::searchTickerBySign(const std::string &sSign, QModelIndex & indx)
{
    auto It = std::find_if(vTickersLst->begin(),vTickersLst->end(),[&](const auto &c){
        return c.TickerSign() == sSign && c.TickerSignFinam() == "";
        });
    if(It == vTickersLst->end()){
            return false;
    }
    /////////////////

    indx = this->index(std::distance(vTickersLst->begin(),It),0);

    return  true;
}
//--------------------------------------------------------------------------------------------------------
bool modelTickersList::searchTickerByPureSign(const std::string &sSign, QModelIndex & indx)
{
    auto It = std::find_if(vTickersLst->begin(),vTickersLst->end(),[&](const auto &c){
        return c.TickerSign() == sSign;
        });
    if(It == vTickersLst->end()){
            return false;
    }
    /////////////////

    indx = this->index(std::distance(vTickersLst->begin(),It),0);

    return  true;
}
//--------------------------------------------------------------------------------------------------------

bool modelTickersList::searchTickerByFinamSign(const std::string &sSign, QModelIndex & indx)
{
    auto It = std::find_if(vTickersLst->begin(),vTickersLst->end(),[&](const auto &c){
                        return c.TickerSignFinam() == sSign;
    });
    if(It == vTickersLst->end()){
            return false;
    }
    /////////////////

    indx = this->index(std::distance(vTickersLst->begin(),It),0);

    return  true;
}
//--------------------------------------------------------------------------------------------------------
bool modelTickersList::searchTickerByQuikSign(const std::string &sSign, QModelIndex & indx)
{
    auto It = std::find_if(vTickersLst->begin(),vTickersLst->end(),[&](const auto &c){
                        return c.TickerSignQuik() == sSign;
    });
    if(It == vTickersLst->end()){
            return false;
    }
    /////////////////

    indx = this->index(std::distance(vTickersLst->begin(),It),0);

    return  true;
}

//--------------------------------------------------------------------------------------------------------
bool modelTickersList::searchTickerByTickerID(const int TickerID, QModelIndex & indx)
{
    auto It = std::find_if(vTickersLst->begin(),vTickersLst->end(),[&](const auto &c){
                        return c.TickerID() == TickerID;
    });
    if(It == vTickersLst->end()){
        return false;
    }
    /////////////////

    indx = this->index(std::distance(vTickersLst->begin(),It),0);

    return  true;
}
//--------------------------------------------------------------------------------------------------------
void modelTickersList::setGrayColorForInformants(bool b)
{
    if (bGrayColorForInormants != b){
        bGrayColorForInormants = b;

        QVector<int> vV {Qt::BackgroundColorRole};
        QModelIndex indexBeg = this->index(0,0);
        QModelIndex indexEnd = this->index((int)vTickersLst->size()-1,4);

        emit dataChanged(indexBeg,indexEnd,vV);
    }
}
//--------------------------------------------------------------------------------------------------------
