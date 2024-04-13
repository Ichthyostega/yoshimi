/*
    ddump.h - DEBUG / diagnostics helper for dump-logging

    Copyright 2024,  Ichthyostega

    This file is part of yoshimi, which is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the License,
    or (at your option) any later version.

    yoshimi is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License (version 2
    or later) for more details.

    You should have received a copy of the GNU General Public License
    along with yoshimi.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef DEBUG_DDUMP_H
#define DEBUG_DDUMP_H

#include "globals.h"
#include "Misc/FormatFuncs.h"
#include <iostream>
#include <iomanip>
#include <fstream>

using std::cout;
using std::endl;


/**
 * Debug logging helper for output to console or file.
 * See: https://stackoverflow.com/a/40424272/444796
 */
class DebugDump
{
    typedef std::ostream&  (*ManipFun)(std::ostream&);
    typedef std::ios_base& (*FlagsFun)(std::ios_base&);

    std::ostream* sink;
    std::ofstream logfile;
    std::string   target;

public:
    bool enabled;

    DebugDump()
        : sink(& std::cout)
        , logfile{}
        , target{"COUT"}
        , enabled{true}
        { }

    /* ======== forward stream operations ===== */

    /** forward any value to the stream sink */
    template<class VAL>
    DebugDump& operator<<(VAL const& data)
    {
        if (enabled)
            *sink << data;
        return *this;
    }

    /** handle stream manipulator, e.g. endl, flush, setw, setfill... */
    DebugDump& operator<<(ManipFun manipulator)
    {
        if (enabled)
            manipulator(*sink);
        return *this;
    }

    /** handle flags on the stream, e.g. setiosflags, resetiosflags... */
    DebugDump& operator<<(FlagsFun flagSetter)
    {
        if (enabled)
            flagSetter(*sink);
        return *this;
    }

    void flush()
    {
        if (enabled)
            std::flush (*sink);
    }

    /**
     * attempt to open / truncate the designated file and activate logging
     * @return `true` if logging to this file could be enabled
     */
    bool log2file(std::string filename);
    void log2std(bool toERR);

    std::string showTarget()
    {
        return target;
    }
};


extern DebugDump dDump;


#endif /*DEBUG_DDUMP_H*/
