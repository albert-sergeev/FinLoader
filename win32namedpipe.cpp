#include "win32namedpipe.h"

#ifdef _WIN32

#include<stdio.h>
#include <conio.h>
#include <tchar.h>
#include "threadfreecout.h"

//Win32NamedPipe::Win32NamedPipe()
//{

//}
//----------------------------------------------------------------------------------------------------------------------------
bool Win32NamedPipe::open()
{
    if(iMode == ePipeMode_type::Byte_Nonblocking){
        return open_bytemode();
    }
    else if(iMode == ePipeMode_type::Message_Nonblocking){
        return open_messagemode();
    }
    else{
        return false;
    }
}
//----------------------------------------------------------------------------------------------------------------------------
bool Win32NamedPipe::open_messagemode()
{
    if (pipeState == ePipeState_type::Connected){
        if(iMode == ePipeMode_type::Message_Nonblocking){
            return true;
        }
        else{
            close();
            if (pipeState != ePipeState_type::Null && pipeState != ePipeState_type::Closed){
                return false;
            }
        }
    }
    ////////////////////////
    std::wstring wstr(sPipePath.begin(), sPipePath.end());

    while(1){
        if (! WaitNamedPipe(wstr.c_str(), 20000)) {
            ThreadFreeCout pcout;
            pcout <<"Could not open pipe: 20 second wait timed out.\n";

            pipeState = ePipeState_type::Error;
            return false;
        }

        secattr.nLength = sizeof (secattr);
        secattr.lpSecurityDescriptor = NULL;
        secattr.bInheritHandle = true;


        hPipe = CreateFile(
                 wstr.c_str(),//lpszPipename,   // pipe name
                 PIPE_READMODE_MESSAGE| PIPE_NOWAIT
                 |FILE_WRITE_ATTRIBUTES // to SetNamedPipeHandleState
                 ,
                 0,//FILE_SHARE_WRITE | FILE_SHARE_READ,//0,              // no sharing
                 &secattr,           // default security attributes
                 OPEN_EXISTING,  // opens existing pipe
                 0,              // default attributes
                 NULL);          // no template file
        // Break if the pipe handle is valid.
        if (hPipe != INVALID_HANDLE_VALUE)
           break;  //return true;

        // Exit if an error other than ERROR_PIPE_BUSY occurs.
        if (GetLastError() != ERROR_PIPE_BUSY){
            ThreadFreeCout pcout;
            pcout <<"Could not open pipe. Pipe is busy. GLE={"<<GetLastError()<<"}\n";

            pipeState = ePipeState_type::Error;
            return false;
        }
    }
    ////////////////////////////////////////////////////////////////////////////////
//    DWORD dwMode = PIPE_READMODE_MESSAGE | PIPE_NOWAIT;// PIPE_READMODE_MESSAGE;
//    bool fSuccess = SetNamedPipeHandleState(
//        hPipe,    // pipe handle
//        &dwMode,  // new pipe mode
//        NULL,     // don't set maximum bytes
//        NULL);    // don't set maximum time
//    ////////////////////////////////////////////////////////////////////////////////
//    if (!fSuccess){
//        ThreadFreeCout pcout;
//        pcout <<"Could not SetNamedPipeHandleState. GLE={"<<GetLastError()<<"}\n";

//        pipeState = ePipeState_type::Error;
//        return false;
//    }

//    {
//        ThreadFreeCout pcout;
//        pcout <<"open message mode hPipe = {"<<hPipe<<"}\n";
//    }

    pipeState =ePipeState_type::Connected;
    return true;

}
//----------------------------------------------------------------------------------------------------------------------------
bool Win32NamedPipe::open_bytemode()
{
    if (pipeState == ePipeState_type::Connected){
        if(iMode == ePipeMode_type::Byte_Nonblocking){
            return true;
        }
        else{
            close();
            if (pipeState != ePipeState_type::Null && pipeState != ePipeState_type::Closed){
                return false;
            }
        }
    }
    ////////////////////////
    std::wstring wstr(sPipePath.begin(), sPipePath.end());

    while(1){
        if (! WaitNamedPipe(wstr.c_str(), 20000)) {
            ThreadFreeCout pcout;
            pcout <<"Could not open pipe: 20 second wait timed out.\n";
            pipeState = ePipeState_type::Error;
            return false;
        }

        secattr.nLength = sizeof (secattr);
        secattr.lpSecurityDescriptor = NULL;
        secattr.bInheritHandle = true;


        hPipe = CreateFile(
                 wstr.c_str(),//lpszPipename,   // pipe name
                 PIPE_TYPE_BYTE | PIPE_NOWAIT
                 |FILE_WRITE_ATTRIBUTES // to SetNamedPipeHandleState
                 ,
                 0,              // no sharing
                 &secattr,       // default security attributes
                 OPEN_EXISTING,  // opens existing pipe
                 0,              // default attributes
                 NULL);          // no template file
        // Break if the pipe handle is valid.
        if (hPipe != INVALID_HANDLE_VALUE)
           break;  //return true;

        // Exit if an error other than ERROR_PIPE_BUSY occurs.
        if (GetLastError() != ERROR_PIPE_BUSY){
            ThreadFreeCout pcout;
            pcout <<"Could not open pipe. Pipe is busy. GLE={"<<GetLastError()<<"}\n";

            pipeState = ePipeState_type::Error;
            return false;
        }
    }
    ////////////////////////////////////////////////////////////////////////////////
    DWORD dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;// PIPE_READMODE_MESSAGE;
    bool fSuccess = SetNamedPipeHandleState(
        hPipe,    // pipe handle
        &dwMode,  // new pipe mode
        NULL,     // don't set maximum bytes
        NULL);    // don't set maximum time
    ////////////////////////////////////////////////////////////////////////////////
    if (!fSuccess){
        ThreadFreeCout pcout;
        pcout <<"Could not SetNamedPipeHandleState. GLE={"<<GetLastError()<<"}\n";

        pipeState = ePipeState_type::Error;
        return false;
    }

//    {
//        ThreadFreeCout pcout;
//        pcout <<"Open byte mode nonblocking hPipe = {"<<hPipe<<"}\n";
//    }

    pipeState = ePipeState_type::Connected;
    return true;
}
//----------------------------------------------------------------------------------------------------------------------------
bool Win32NamedPipe::read(char * buff, int buffsize, int &bytesRead)
{
    if (pipeState != ePipeState_type::Connected){
        return false;
    }
    //////////////////////////////////////////////////////////////////////
    /// \brief fSuccess
    DWORD  cbRead;
    bool fSuccess;
//    {
//        ThreadFreeCout pcout;
//        pcout <<"pipe to read={"<<hPipe<<"}"<<"\n";
//    }
    // Read from the pipe.
    fSuccess = ReadFile(
          hPipe,    // pipe handle
          buff,    // buffer to receive reply
          buffsize,  // size of buffer
          &cbRead,  // number of bytes read
          NULL);    // not overlapped


    if (!fSuccess && GetLastError() != ERROR_MORE_DATA && GetLastError() != ERROR_NO_DATA){
        if (GetLastError() != 233)
        {
            ThreadFreeCout pcout;
            pcout <<"ReadFile error. GLE={"<<GetLastError()<<"} <"<<cbRead<<">\n";
        }
        pipeState = ePipeState_type::Error;
        return false;
    }
    else{
//        ThreadFreeCout pcout;
//        pcout << "pcout: "<<cbRead<<"\n";
    }
    bytesRead = cbRead;
    //////////////////////////////////////////////////////////////////////
    return fSuccess;
}
//----------------------------------------------------------------------------------------------------------------------------
void Win32NamedPipe::close()
{
    if (pipeState == ePipeState_type::Connected){
        pipeState = ePipeState_type::Closed;
        CloseHandle(hPipe);
    }
}
//----------------------------------------------------------------------------------------------------------------------------
bool Win32NamedPipe::reinit(bool bForce)
{
    if (pipeState == ePipeState_type::Closed){
        pipeState = ePipeState_type::Null;
        return true;
    }
    else if (bForce && pipeState == ePipeState_type::Error){
        pipeState = ePipeState_type::Null;
        try{
            CloseHandle(hPipe);
        }
        catch(...){;}
        return true;
    }
    else if (pipeState == ePipeState_type::Null){
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------------------------------------------------------
Win32NamedPipe::~Win32NamedPipe()
{
    close();
}
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
#endif
