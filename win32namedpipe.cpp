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
///
/// \brief Win32NamedPipe::open open pipe dependent of mode set in constructor
/// \return true if success. error is in GetLastError()
///
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
///
/// \brief Win32NamedPipe::open_messagemode windows behavor procedure for opening a pipe in non-blocking message mode
/// \return
///
bool Win32NamedPipe::open_messagemode()
{
    // check pipe state
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
    // change state not needed - all correct on creation

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


    pipeState =ePipeState_type::Connected;
    return true;

}
//----------------------------------------------------------------------------------------------------------------------------
///
/// \brief Win32NamedPipe::open_bytemode windows behavor procedure for opening a pipe in non-blocking byte mode
/// \return
///
bool Win32NamedPipe::open_bytemode()
{
    // check state
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
    /// set pipe Handle workstate (change mode to byte)

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

    pipeState = ePipeState_type::Connected;
    return true;
}
//----------------------------------------------------------------------------------------------------------------------------
/// read connected pipe
/// ret true if success & bytesRead - real bytes read
/// if pipe has more data, return false too (Windows behavor) - so need to check GetLastError()!=ERROR_MORE_DATA
/// in GetLastError() - error
///
bool Win32NamedPipe::read(char * buff, int buffsize, int &bytesRead)
{
    if (pipeState != ePipeState_type::Connected){
        return false;
    }
    //////////////////////////////////////////////////////////////////////
    /// \brief fSuccess
    DWORD  cbRead;
    bool fSuccess;

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
    bytesRead = cbRead;
    //////////////////////////////////////////////////////////////////////
    return fSuccess;
}
//----------------------------------------------------------------------------------------------------------------------------
///
/// \brief Win32NamedPipe::close close pipe (only if connected)
///
void Win32NamedPipe::close()
{
    if (pipeState == ePipeState_type::Connected){
        pipeState = ePipeState_type::Closed;
        CloseHandle(hPipe);
    }
}
//----------------------------------------------------------------------------------------------------------------------------
///
/// \brief Win32NamedPipe::reinit reinit pipe.
/// Not usual procedure, so use cerefully
/// \param bForce
/// \return
///
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
/// close pipe on destruction
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
