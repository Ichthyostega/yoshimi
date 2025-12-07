/*
    Bank.cpp - Instrument Bank

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2005 Nasca Octavian Paul
    Copyright 2009-2010, Alan Calvert
    Copyright 2014-2023, Will Godfrey & others

    This file is part of yoshimi, which is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    yoshimi is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.   See the GNU General Public License (version 2 or
    later) for more details.

    You should have received a copy of the GNU General Public License along with
    yoshimi; if not, write to the Free Software Foundation, Inc., 51 Franklin
    Street, Fifth Floor, Boston, MA  02110-1301, USA.

    This file is a derivative of a ZynAddSubFX original.

*/


#include "Misc/Bank.h"
#include "Misc/SynthEngine.h"
#include "Misc/FileMgrFuncs.h"

const int BANKS_VERSION = 11;



Bank::Bank(SynthEngine& _synth)
    : version{BANKS_VERSION}
    , banksInRoots{0}
    , instrumentsInBanks{0}
    , defaultInsName{}
    , foundLocal{file::localDir() + "/found/"}
    , roots{}
    , synth(_synth)
    { }

