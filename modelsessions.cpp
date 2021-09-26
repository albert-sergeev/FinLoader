#include "modelsessions.h"

#include<QDate>
#include<QTime>


//------------------------------------------------------------------------------------------------------------------
//modelSessions::modelSessions(Market::SessionTable_type &Table):
//    sessionTable{Table}
//{

//}
//------------------------------------------------------------------------------------------------------------------
QVariant modelSessions::headerData(int nSection, Qt::Orientation orientation, int nRole) const
{
    if(nRole!=Qt::DisplayRole){
        return QVariant();
    }
    if(orientation==Qt::Horizontal){
        if (nSection == 0){
            return QString(tr("Periods"));
        }
        else {
            return QString(tr("Time"));
        }
    }
    else{
        return QString::number(nSection);
    }

}

//------------------------------------------------------------------------------------------------------------------
void modelSessions::setSessionTable(const Market::SessionTable_type &Table)
{
    sessionTable = Table;

    std::string sT;
    std::string sSubB;
    std::string sSubE;

    this->clear();
    insertRows(0,(int)sessionTable.size());
    insertColumns(0,2);

    auto It = (sessionTable.begin());
    int iI{0};
    while (It != sessionTable.end()){

        QModelIndex indx = index(iI,0);

        setData(indx,stdTimeToQDate(It->first));
        //
        insertRows(0,(int)It->second.second.size(),indx);
        insertColumns(0,2,indx);

        for(int j = 0; j < (int)It->second.second.size(); ++j){
            sSubB = threadfree_gmtime_time_to_str(&It->second.second[j].first);
            sSubE = threadfree_gmtime_time_to_str(&It->second.second[j].second);

            setData(index(j,0,indx),stdTimeToQTime(It->second.second[j].first));
            setData(index(j,1,indx),stdTimeToQTime(It->second.second[j].second));
        }
        ++It; ++iI;
    }
}
//------------------------------------------------------------------------------------------------------------------
void modelSessions::addNewPeriod(const QModelIndex& indx)
{
    QTime tmB(0,0,0);
    QTime tmE(17,59,59);

    if (indx == QModelIndex()){

        QStandardItem *root = invisibleRootItem();
        QDate dtMax;
        QDate dtCur = QDate::currentDate();

        for (int iRow = 0 ; iRow < root->rowCount(); ++iRow){
            QModelIndex dx = index(iRow,0);
            QDate dateD = qvariant_cast<QDate>(dx.data());

            if (dtMax < dateD)  dtMax = dateD;
        }
        dtMax = dtMax.addDays(1);

        if(dtMax < dtCur)  dtMax = dtCur;
        ///

        insertRows(rowCount(),1);
        QModelIndex indxNew = index(rowCount() - 1,0);

        setData(indxNew,dtMax);
        //
        insertRows(0,1,indxNew);
        insertColumns(0,2,indxNew);

        setData(index(0,0,indxNew),tmB);
        setData(index(0,1,indxNew),tmE);
    }
    else{

    }
}
//------------------------------------------------------------------------------------------------------------------
void modelSessions::DeletePeriod   (const QModelIndex& indx)
{}
//------------------------------------------------------------------------------------------------------------------
void modelSessions::addNewTimeRange(const QModelIndex& indx )
{}
//------------------------------------------------------------------------------------------------------------------
void modelSessions::DeleteTimeRange(const QModelIndex& indx)
{}
//------------------------------------------------------------------------------------------------------------------
bool modelSessions::setData(const QModelIndex &indx,const QVariant &value,int nRole)
{
    QStandardItem *parent = static_cast<QStandardItem*>(indx.internalPointer());
    if (parent == 0) return false;

    for (int iRow = 0 ; iRow < parent->rowCount(); ++iRow){
        QModelIndex dx = index(iRow,0);
        if (dx == indx) continue; // not self
        //////////////////////////////////
        if (dx.data().canConvert<QDate>() && value.canConvert<QDate>()){
            QDate dateX = qvariant_cast<QDate>(dx.data());
            QDate dateI = qvariant_cast<QDate>(value);
            if (dateX == dateI){ // doubles not alowed!
                return false;
            }
        }
        //////////////////////////////////
        if (dx.data().canConvert<QTime>() && value.canConvert<QTime>()){
            QTime dateX = qvariant_cast<QTime>(dx.data());
            QTime dateI = qvariant_cast<QTime>(value);
            if (dateX == dateI){ // doubles not alowed!
                return false;
            }
        }
    }
    return QStandardItemModel::setData(indx,value,nRole);
}
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
