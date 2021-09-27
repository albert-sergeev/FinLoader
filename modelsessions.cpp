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

    QDate dtEpoch(1971,1,1);
    QTime tmB(9,0,0);
    QTime tmE(17,59,59);
    QStandardItem *root = invisibleRootItem();

    if (indx == QModelIndex()){

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
        if (indx.isValid()){

            QStandardItem *parent = static_cast<QStandardItem*>(indx.internalPointer());

            if (parent == root && indx.row() < root->rowCount()){
                QDate dateX = qvariant_cast<QDate>(indx.data());
                dateX = dateX.addDays(-1);
                QModelIndex dx = indx;

                bool bWas = true;
                while(bWas){
                    bWas = false;
                    for (int iRow = 0 ; iRow < root->rowCount(); ++iRow){
                        dx = index(iRow,0);
                        QDate dateD = qvariant_cast<QDate>(dx.data());

                        if (dateX == dateD) {
                            dateX = dateX.addDays(-1);
                            bWas = true;
                            break;
                        }
                    }
                    if (dtEpoch > dateX) return;
                }
                ///

                insertRows(dx.row(),1);
                QModelIndex indxNew = index(dx.row(),0);

                setData(indxNew,dateX);
                //
                insertRows(0,1,indxNew);
                insertColumns(0,2,indxNew);

                setData(index(0,0,indxNew),tmB);
                setData(index(0,1,indxNew),tmE);

            }
        }
    }
}
//------------------------------------------------------------------------------------------------------------------
void modelSessions::DeletePeriod   (const QModelIndex& indx)
{
    if ( indx !=QModelIndex() && indx.isValid()){

        QStandardItem *parent = static_cast<QStandardItem*>(indx.internalPointer());
        QStandardItem *root = invisibleRootItem();

        if (parent == root && indx.row() < root->rowCount()){
            if (root->rowCount() > 1) //not last
                removeRow(indx.row());
        }
    }
}
//------------------------------------------------------------------------------------------------------------------
void modelSessions::addNewTimeRange(const QModelIndex& indx,const bool bInsert)
{
    if ( indx !=QModelIndex() && indx.isValid()){

        QStandardItem *root = invisibleRootItem();
        QStandardItem *parent = static_cast<QStandardItem*>(indx.internalPointer());
        if (parent == 0) return;

        if (parent == root && indx.row() < root->rowCount()){
            // point to header
            QTime tmTarget(0,0,0);
            QStandardItem *parentItem = root->child(indx.row());

            if (!getTimeRangeAppend(indx,parentItem,tmTarget)){
                return;
            }
            //
            insertRows(parentItem->rowCount(),1,indx);
            insertColumns(parentItem->rowCount(),2,indx);

            setData(index(parentItem->rowCount() - 1,0,indx),tmTarget);
            setData(index(parentItem->rowCount() - 1,1,indx),accomodateTimeRange(tmTarget,false));
        }
        else{
            QModelIndex indexParent = this->indexFromItem(parent);
            QStandardItem *parentParent = static_cast<QStandardItem*>(indexParent.internalPointer());

            if (parentParent == 0) return;
            if (parentParent == root){
                // point to twin
                QTime tmTarget;

                if (bInsert && !getTimeRangeInsert(indx,parent,tmTarget)){
                    return;
                }
                else if(!bInsert && !getTimeRangeAppend(indexParent,parent,tmTarget)){
                    return;
                }
                //
                int iRow = indx.row();

                if(!bInsert) {
                    insertRows(parent->rowCount(),1,indexParent);
                    //insertColumns(parent->rowCount(),2,indexParent);

                    setData(index(parent->rowCount() - 1,0,indexParent),tmTarget);
                    setData(index(parent->rowCount() - 1,1,indexParent),accomodateTimeRange(tmTarget,false));
                }
                else{
                    insertRows(iRow,1,indexParent);
                    //insertColumns(iRow+1,2,indexParent);

                    setData(index(iRow,0,indexParent),tmTarget);
                    setData(index(iRow,1,indexParent),accomodateTimeRange(tmTarget,false));
                }
            }
        }
    }
}
//------------------------------------------------------------------------------------------------------------------
bool modelSessions::getTimeRangeAppend(const QModelIndex& indxParent,const QStandardItem *parentItem,QTime &t)
{
    t = QTime(0,0,0);

    for (int iRow = 0 ; iRow < parentItem->rowCount(); ++iRow){
        QModelIndex dx = index(iRow,1,indxParent);
        QTime timeD = modelSessions::accomodateTimeRange(qvariant_cast<QTime>(dx.data()));

        if (t < timeD)  {
            t = timeD;
        }
    }
    if (accomodateTimeRange(t,false) >= QTime(23,59)) return false;
    t = t.addSecs(60);

    return true;
}
//------------------------------------------------------------------------------------------------------------------
bool modelSessions::getTimeRangeInsert(const QModelIndex& indx,const QStandardItem *parentItem,QTime &tmTarget)
{
    QModelIndex indexParent = this->indexFromItem(parentItem);

    QModelIndex dxSt = index(indx.row(),0,indexParent);
    QModelIndex dxEn;// = index(indx.row(),1,indexParent);
    QTime timeSt = modelSessions::accomodateTimeRange(qvariant_cast<QTime>(dxSt.data()));
    QTime timeEn;// = modelSessions::accomodateTimeRange(qvariant_cast<QTime>(dxEn.data()));

    tmTarget = QTime(timeSt.hour(),timeSt.minute(),0);
    tmTarget = tmTarget.addSecs(-60);

    bool bWas = true;
    while(bWas){
        bWas = false;
        for (int iRow = 0 ; iRow < parentItem->rowCount(); ++iRow){
            dxSt = index(iRow,0,indexParent);
            dxEn = index(iRow,1,indexParent);
            timeSt = modelSessions::accomodateTimeRange(qvariant_cast<QTime>(dxSt.data()));
            timeEn = modelSessions::accomodateTimeRange(qvariant_cast<QTime>(dxEn.data()),false);

            if (timeSt <= tmTarget && tmTarget <= timeEn)  {
                tmTarget = timeSt;
                if (tmTarget == QTime(0,0,0)) return false;
                tmTarget = tmTarget.addSecs(-60);
                bWas = true;
                break;
            }
        }
    }

    return true;
}
//------------------------------------------------------------------------------------------------------------------
void modelSessions::DeleteTimeRange(const QModelIndex& indx)
{

    if ( indx !=QModelIndex() && indx.isValid()){

        QStandardItem *root = invisibleRootItem();
        QStandardItem *parent = static_cast<QStandardItem*>(indx.internalPointer());
        if (parent == 0) return;

        QModelIndex indexParent = this->indexFromItem(parent);
        QStandardItem *parentParent = static_cast<QStandardItem*>(indexParent.internalPointer());
        if (parentParent == 0) return;
        if (parentParent == root){
            if (parent->rowCount() > 1) // do not remove last
                removeRow(indx.row(),indexParent);
        }
    }
}
//------------------------------------------------------------------------------------------------------------------
bool modelSessions::setData(const QModelIndex &indx,const QVariant &value,int nRole)
{
    QStandardItem *parent = static_cast<QStandardItem*>(indx.internalPointer());
    if (parent == 0) return false;
    QModelIndex indexParent = this->indexFromItem(parent);

    for (int iRow = 0 ; iRow < parent->rowCount(); ++iRow){
        QModelIndex dx = index(iRow,0,indexParent);
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
            // check if in other range

            QStandardItem *parent = static_cast<QStandardItem*>(indx.internalPointer());
            if (parent == 0) return false;
            QModelIndex indexParent = this->indexFromItem(parent);

            QModelIndex dxSt;
            QModelIndex dxEn;
            QTime timeSt;
            QTime timeEn;

            QTime tmValue = qvariant_cast<QTime>(value);
            for(int iRow = 0; iRow < parent->rowCount(); ++iRow){
                if (iRow == indx.row()) continue;

                dxSt = index(iRow,0,indexParent);
                dxEn = index(iRow,1,indexParent);
                timeSt = modelSessions::accomodateTimeRange(qvariant_cast<QTime>(dxSt.data()));
                timeEn = modelSessions::accomodateTimeRange(qvariant_cast<QTime>(dxEn.data()),false);

                // check if in other range
                if (timeSt <= tmValue && tmValue <= timeEn){
                    return false;
                }
            }
        }
    }
    return QStandardItemModel::setData(indx,value,nRole);
}
//------------------------------------------------------------------------------------------------------------------
QTime modelSessions::accomodateTimeRange(const QTime &t, const bool bFirst)
{
    QTime tR;
    if (bFirst){
        tR = QTime(t.hour(),t.minute(),0,0);
    }
    else{
        tR = QTime(t.hour(),t.minute(),59,0);
    }
    return tR;
}
//------------------------------------------------------------------------------------------------------------------
// typedef  std::vector<std::pair<std::time_t,std::pair<std::time_t,std::vector<std::pair<std::time_t,std::time_t>>>>> SessionTable_type;

Market::SessionTable_type modelSessions::getSessionTable() const
{
    Market::SessionTable_type vRet;

    QStandardItem *root = invisibleRootItem();
    QStandardItem *period;
    std::time_t tDateBeg;
    std::time_t tDateEnd;

    QModelIndex dxSt;
    QModelIndex dxEn;
    std::time_t tTimeBeg;
    std::time_t tTimeEnd;

    for(int iRow = 0; iRow < root->rowCount(); ++iRow){
        QModelIndex idx = index(iRow,0);
        QDate dateX = qvariant_cast<QDate>(idx.data());
        tDateBeg = Bar::DateAccommodate(QDateToStdTime(dateX),Bar::eInterval::pDay);
        tDateEnd = tDateBeg;

        vRet.push_back({tDateBeg,{tDateEnd,{}}});

        period = itemFromIndex(idx);
        if(period != 0 && period->columnCount() > 1){
            std::vector<std::pair<std::time_t,std::time_t>> &vTime (vRet.back().second.second);

            for(int iRowPeriod = 0; iRowPeriod < period->rowCount(); ++iRowPeriod){

                dxSt = index(iRowPeriod,0,idx);
                dxEn = index(iRowPeriod,1,idx);
                tTimeBeg = Market::AccomodateToTime(QTimeToStdTime(qvariant_cast<QTime>(dxSt.data())));
                tTimeEnd = Market::AccomodateToTime(QTimeToStdTime(qvariant_cast<QTime>(dxEn.data())));

                vTime.push_back({tTimeBeg,tTimeEnd});
            }
            std::sort(vTime.begin(),vTime.end(),[](const auto &l, const auto &r){return l.first < r.first;});
        }
    }
    // sort
    std::sort(vRet.begin(),vRet.end(),[](const auto &l, const auto &r){return l.first < r.first;});

    std::tm tmPer;
    {
        tmPer.tm_year   = 2100 - 1900;
        tmPer.tm_mon    = 1 - 1;
        tmPer.tm_mday   = 1;
        tmPer.tm_hour   = 0;
        tmPer.tm_min    = 0;
        tmPer.tm_sec    = 0;
        tmPer.tm_isdst  = 0;
    }
    std::time_t tLast   = mktime_gm(&tmPer);

    auto It (vRet.rbegin());
    while(It !=vRet.rend()){
        It->second.first = tLast;
        tLast = It->first - 1;
        ++It;
    }
    return vRet;
}
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
