#ifndef DATAAMIPIPEANSWER_H
#define DATAAMIPIPEANSWER_H



class dataAmiPipeAnswer
{
public:
    enum eAnswer_type:int {Nop, ProcessNewComplite, PipeConnected, PipeDisconnected, PipeHalted, PipeOff};
public:
    int iTickerID{0};
    eAnswer_type Type {eAnswer_type::Nop};
public:
    dataAmiPipeAnswer(){};
    dataAmiPipeAnswer(const dataAmiPipeAnswer &o) = default;

    dataAmiPipeAnswer& operator=(dataAmiPipeAnswer &o) = delete;
};

#endif // DATAAMIPIPEANSWER_H
