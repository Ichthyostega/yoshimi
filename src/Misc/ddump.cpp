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


#include "Misc/ddump.h"

#include <filesystem>
#include <string>

using std::string;

namespace fs = std::filesystem;


namespace { // filesystem permission checks...

    inline bool has_perm(fs::path const& p, fs::perms permissionMask)
    {
    return (fs::status(p).permissions() & permissionMask) == permissionMask;
    }

    inline bool can_write(fs::path const& p)
    {
        if (fs::exists(p) and fs::is_regular_file(p))
            return has_perm(p, fs::perms::owner_write);

        if (p.has_filename())
        {// path for not(yet) existing file....
            fs::path dir = p.parent_path();
            return fs::exists(dir) and has_perm(dir, fs::perms::owner_write);
        }
        return false;
    }
}



bool DebugDump::log2file(string filename)
{
    fs::path p{filename};
    p = fs::absolute(p);
    if (can_write(p))
    {
        logfile.open(p, std::ios_base::out |std::ios_base::app);
        if (logfile.good())
        {
            logfile << "\n=============================" <<endl;
            if (logfile.good())
            {
                this->enabled = true;
                this->sink = & logfile;
                this->target = fs::canonical(p);
                return true;
            }
        }
    }
    std::cerr << "Unable to open for writing: "<<p<<endl;
    return false;
}


void DebugDump::log2std(bool toERR)
{
    if (logfile.is_open())
        logfile.close();

    sink   = toERR? &std::cerr : &std::cout;
    target = toERR? "STDERR"   : "STDOUT";
    enabled = true;
}



/** global debug output stream */
DebugDump dDump;
