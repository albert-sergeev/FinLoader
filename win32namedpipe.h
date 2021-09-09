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

private:
    SECURITY_ATTRIBUTES secattr;
    HANDLE hPipe;
    std::string sPipePath;
    std::atomic<ePipeState_type> pipeState;

public:
    //Win32NamedPipe() = delete;
    Win32NamedPipe(const std::string Path = ""):sPipePath{Path},pipeState{ePipeState_type::Null}{};
    Win32NamedPipe(const Win32NamedPipe &o) {
        secattr     = o.secattr;
        hPipe       = std::move(o.hPipe);
        pipeState   = o.pipeState.load();
    };
    ~Win32NamedPipe();

    Win32NamedPipe& operator= (const Win32NamedPipe &o) {
        secattr     = o.secattr;
        hPipe       = std::move(o.hPipe);
        pipeState   = o.pipeState.load();
        return *this;
    };

    void setPipePath(const std::string &s)     {sPipePath  = s;}

    bool open();
    bool reinit();
    bool good() const {return pipeState.load() == ePipeState_type::Connected;}
    void close();

    bool read(char * buff, int buffsize, int &bytesRead);

};

#endif

#endif // WIN32NAMEDPIPE_H
