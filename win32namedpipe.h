#ifndef WIN32NAMEDPIPE_H
#define WIN32NAMEDPIPE_H

#ifdef _WIN32

#include<string>
#include<atomic>
#include<memory>

#include<Windows.h>

class Win32NamedPipe
{
public:
    enum ePipeState_type:int {Null,Connected,Closed,Error};

    enum ePipeMode_type:int {Byte_Nonblocking = 1,Message_Nonblocking = 2};
private:
    SECURITY_ATTRIBUTES secattr;
    HANDLE hPipe;
    std::string sPipePath;

    ePipeState_type pipeState;
    ePipeMode_type iMode;

public:
    //Win32NamedPipe() = delete;
    Win32NamedPipe(const std::string Path = ""):
        sPipePath{Path},
        pipeState{ePipeState_type::Null},
        iMode{ePipeMode_type::Byte_Nonblocking}
    {};
    Win32NamedPipe(const Win32NamedPipe &o) {
        secattr     = o.secattr;
        hPipe       = o.hPipe;//std::move(o.hPipe);
        pipeState   = o.pipeState;
        iMode       = o.iMode;
    };
    ~Win32NamedPipe();

    Win32NamedPipe& operator= (const Win32NamedPipe &o) {
        secattr     = o.secattr;
        hPipe       = o.hPipe;//std::move(o.hPipe);
        pipeState   = o.pipeState;
        iMode       = o.iMode;
        return *this;
    };
    //---------------------------------------------------------------


    ePipeMode_type Mode()        const         {return iMode;};
    void setMode(ePipeMode_type m)   { if (pipeState == Null || pipeState == Closed ) iMode = m;};

    void setPipePath(const std::string &s)     {sPipePath  = s;}

    bool open();
    bool reinit(bool bForce);
    bool good() const {return pipeState == ePipeState_type::Connected;}
    void close();

    bool read(char * buff, int buffsize, int &bytesRead);

    //---------------------------------------------------------------
private:
    bool open_bytemode();
    bool open_messagemode();
};

#endif

#endif // WIN32NAMEDPIPE_H
