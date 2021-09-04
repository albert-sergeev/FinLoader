#ifndef AMIPIPERFORM_H
#define AMIPIPERFORM_H

#include <QWidget>
#include "modelmarketslist.h"
#include "modeltickerslist.h"
#include "amipipeholder.h"

namespace Ui {
class AmiPiperForm;
}

class AmiPiperForm : public QWidget
{
    Q_OBJECT

protected:

    int iDefaultTickerMarket;

    modelMarketsList * const modelMarket;
    modelTickersList * const modelTicker;
    TickerProxyListModel proxyTickerModel;

    AmiPipeHolder &pipes;

public:
    explicit AmiPiperForm(modelMarketsList *modelM, int DefaultTickerMarket,
                          modelTickersList *modelT,
                          AmiPipeHolder & p,
                          QWidget *parent = nullptr);
    ~AmiPiperForm();

public:
signals:
    void SendToMainLog(QString);

protected slots:
    void btnCheckClicked();
private:
    Ui::AmiPiperForm *ui;
};

#endif // AMIPIPERFORM_H
