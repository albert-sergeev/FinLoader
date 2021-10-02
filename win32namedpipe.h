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

#ifndef WIN32NAMEDPIPE_H
#define WIN32NAMEDPIPE_H

#ifdef _WIN32

#include<string>
#include<atomic>
#include<memory>

#include<Windows.h>

//////////////////////////////////////////////////////////////
/// \brief Cover for Windows named pipes
/// can work in two mode (ePipeMode_type::Byte_Nonblocking by default)
///
class Win32NamedPipe
{
    // enums for extrnal use
public:
    enum ePipeState_type:int {Null,Connected,Closed,Error};
    enum ePipeMode_type:int {Byte_Nonblocking = 1,Message_Nonblocking = 2};
private:
    // windows behavor part
    SECURITY_ATTRIBUTES secattr;
    HANDLE hPipe;
    std::string sPipePath;

    // state & mode
    ePipeState_type pipeState;
    ePipeMode_type iMode;

public:
    //---------------------------------------------------------------
    // constructors & standart interface
    Win32NamedPipe(const std::string Path = ""):
        sPipePath{Path},
        pipeState{ePipeState_type::Null},
        iMode{ePipeMode_type::Byte_Nonblocking}
    {};
    //----------------------------------------------------
    Win32NamedPipe(const Win32NamedPipe &o) {
        secattr     = o.secattr;
        hPipe       = o.hPipe;//std::move(o.hPipe);
        pipeState   = o.pipeState;
        iMode       = o.iMode;
    };
    //----------------------------------------------------
    ~Win32NamedPipe();
    //----------------------------------------------------
    Win32NamedPipe& operator= (const Win32NamedPipe &o) {
        secattr     = o.secattr;
        hPipe       = o.hPipe;//std::move(o.hPipe);
        pipeState   = o.pipeState;
        iMode       = o.iMode;
        return *this;
    };
    // constructors & standart interface end
    //---------------------------------------------------------------

    //---------------------------------------------------------------
    // pipe manipulation

    /// check pipe mode
    ePipeMode_type Mode()        const         {return iMode;};

    /// set pipe mode if allowed
    void setMode(ePipeMode_type m)   { if (pipeState == Null || pipeState == Closed ) iMode = m;};

    // set pipename(i.e. path) if not set in constructor
    void setPipePath(const std::string &s)     {sPipePath  = s;}

    /// open (connect) pipe
    bool open();

    /// reinit closed or halted pipe
    bool reinit(bool bForce);

    /// check pipe state (true if connected)
    bool good() const {return pipeState == ePipeState_type::Connected;}

    /// close pipe
    void close();

    // pipe manipulation end
    //---------------------------------------------------------------

    /// read connected pipe
    /// ret true if success & bytesRead - real bytes read
    /// if pipe has more data, return false too (Windows behavor) - so need to check GetLastError()!=ERROR_MORE_DATA
    /// in GetLastError() - error
    bool read(char * buff, int buffsize, int &bytesRead);
    //---------------------------------------------------------------

private:
    /// mode-dependent opening procedures
    bool open_bytemode();
    bool open_messagemode();
};

#endif

#endif // WIN32NAMEDPIPE_H
