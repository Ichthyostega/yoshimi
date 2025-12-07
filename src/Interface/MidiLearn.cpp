/*
    MidiLearn.cpp

    Copyright 2016-2020, Will Godfrey
    Copyright 2021-2023, Will Godfrey, Rainer Hans Liffers

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


#include "Interface/MidiLearn.h"



MidiLearn::MidiLearn(SynthEngine& synthInstance)
    : synth(synthInstance)
    , data{}
    , learning{false}
    , midi_list{}
    , learnedName{}
    , learnTransferBlock{}
    { }



void MidiLearn::updateGui(int opp)
{
////////////////OOO Strip-down: core impl elided
}


/** @note when no LearnBlock are captured,
 *   nothing will be added to the XMLStore */
void MidiLearn::insertMidiListData(XMLStore& xml)
{
////////////////OOO Strip-down: core impl elided
}


bool MidiLearn::extractMidiListData(XMLStore& xml)
{
////////////////OOO Strip-down: core impl elided
}

