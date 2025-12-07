/*
    InstanceManager.h - manage lifecycle of Synth-Engine instances

    Copyright 2024,  Ichthyostega

    Based on existing code from main.cpp
    Copyright 2009-2011, Alan Calvert
    Copyright 2014-2021, Will Godfrey & others

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


#include "Misc/InstanceManager.h"




class InstanceManager::SynthGroom
{
    public: // can be moved and swapped, but not copied...
       ~SynthGroom()                             = default;
        SynthGroom(SynthGroom &&)                = default;
        SynthGroom(SynthGroom const&)            = delete;
        SynthGroom& operator=(SynthGroom &&)     = delete;
        SynthGroom& operator=(SynthGroom const&) = delete;

        // can be default created
        SynthGroom() = default;

};


InstanceManager::InstanceManager()
    : groom{/*defunct*/}
    { }

InstanceManager::~InstanceManager() { }


Config& InstanceManager::accessPrimaryConfig()
{
////////////////OOO Strip-down: core impl elided
}

