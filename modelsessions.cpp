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
void modelSessions::setSessionTable(const Market::SessionTable_type &Table)  {
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

        for(int j = 0; j < It->second.second.size(); ++j){
            sSubB = threadfree_gmtime_time_to_str(&It->second.second[j].first);
            sSubE = threadfree_gmtime_time_to_str(&It->second.second[j].second);

            setData(index(j,0,indx),stdTimeToQTime(It->second.second[j].first));
            setData(index(j,1,indx),stdTimeToQTime(It->second.second[j].second));
        }
        ++It; ++iI;
    }
}

