#include "win32namedpipe.h"

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
    if (pipeState.load() == ePipeState_type::Connected){
        return true;
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
                 GENERIC_READ   // read
                 //| GENERIC_WRITE //and write access
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

//    {
//        ThreadFreeCout pcout;
//        pcout <<"hPipe = {"<<hPipe<<"}\n";
//    }

    ePipeState_type pipeOldState = ePipeState_type::Null;
    while(!pipeState.compare_exchange_weak(pipeOldState,ePipeState_type::Connected)){
        if (pipeOldState != ePipeState_type::Null){
            return false;
        }
        //pipeOldState = ePipeState_type::Null;
    }
    return true;
}
//----------------------------------------------------------------------------------------------------------------------------
bool Win32NamedPipe::read(char * buff, int buffsize, int &bytesRead)
{
    if (pipeState.load() != ePipeState_type::Connected){
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
//    do
//    {
    // Read from the pipe.
       fSuccess = ReadFile(
          hPipe,    // pipe handle
          buff,    // buffer to receive reply
          buffsize,  // size of buffer
          &cbRead,  // number of bytes read
          NULL);    // not overlapped

//       if ( ! fSuccess && GetLastError() != ERROR_MORE_DATA )
//          break;
//    } while ( ! fSuccess);  // repeat loop if ERROR_MORE_DATA

    if (!fSuccess && GetLastError() != ERROR_MORE_DATA){
        ThreadFreeCout pcout;
        pcout <<"ReadFile error. GLE={"<<GetLastError()<<"} <"<<cbRead<<">\n";

        ePipeState_type oldState = ePipeState_type::Connected;
        while(!pipeState.compare_exchange_weak(oldState,ePipeState_type::Error)){
            if (oldState != ePipeState_type::Connected){
                return false;
            }
        }
        return false;
    }
    else{
//        ThreadFreeCout pcout;
//        pcout << "pcout: "<<cbRead<<"\n";
    }
    bytesRead = cbRead;
    //////////////////////////////////////////////////////////////////////
    return true;
}
//----------------------------------------------------------------------------------------------------------------------------
void Win32NamedPipe::close()
{
    if (pipeState == ePipeState_type::Connected){
        ePipeState_type oldState = ePipeState_type::Connected;
        while(!pipeState.compare_exchange_weak(oldState,ePipeState_type::Closed)){
            if (oldState != ePipeState_type::Connected){
                return;
            }
        }
        CloseHandle(hPipe);
    }
}
//----------------------------------------------------------------------------------------------------------------------------
bool Win32NamedPipe::reinit()
{
    if (pipeState == ePipeState_type::Closed){
        ePipeState_type oldState = ePipeState_type::Closed;
        while(!pipeState.compare_exchange_weak(oldState,ePipeState_type::Null)){
            if (oldState != ePipeState_type::Closed){
                return false;
            }
        }
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
