/*
    InterChange.cpp - General communications

    Copyright 2016-2019, Will Godfrey & others
    Copyright 2020-2020, Kristian Amlie, Will Godfrey, & others
    Copyright 2021 Will Godfrey, Rainer Hans Liffers, & others
    Copyright 2023-2025, Will Godfrey, Ichthyostega & others

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

*/


#include "Interface/InterChange.h"

enum envControl: uchar {
    input,
    undo,
    redo
};

#include "MasterUI.h"



InterChange::InterChange(SynthEngine& synthInstance)
    : synth(synthInstance),
    fromCLI(),
    decodeLoopback(),
    fromGUI(),
    toGUI(),
    fromMIDI(),
    returnsBuffer(),
    muteQueue(),
    guiDataExchange{[](CommandBlock const&){ /* no communication GUI */ }},
    syncWrite(false),
    lowPrioWrite(false),
    swapRoot1(UNUSED),
    swapBank1(UNUSED),
    swapInstrument1(UNUSED),
    searchInst(0),
    searchBank(0),
    searchRoot(0),
    partsChanged{}
{
    noteSeen = false;
    undoLoopBack = false;
    setUndo = false;
    setRedo = false;
    undoStart = false;
    cameFrom = envControl::input;
    undoMarker.data.part = TOPLEVEL::section::undoMark;
    ////////////////OOO Strip-down: core impl elided
}


InterChange::~InterChange()
{
    ////////////////OOO Strip-down: core impl elided
}


