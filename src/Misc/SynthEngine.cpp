/*
    SynthEngine.cpp

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2005 Nasca Octavian Paul
    Copyright 2009-2011, Alan Calvert
    Copyright 2009, James Morris
    Copyright 2014-2020, Will Godfrey & others

    Copyright 2022-2023, Will Godfrey, Rainer Hans Liffers
    Copyright 2024-2025, Will Godfrey, Ichthyostega, Kristian Amlie & others

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
    Street, Fifth Floor, Boston, MA 02110-1301, USA.

    This file is derivative of original ZynAddSubFX code.

*/

#include "Misc/SynthEngine.h"
#include "Effects/EffectMgr.h"
#include "Misc/TextMsgBuffer.h"
#include "Misc/XMLStore.h"


using func::asString;



SynthEngine::SynthEngine(uint instanceID)
    : uniqueId{instanceID}
    , Runtime{*this}
    , bank{*this}
    , interchange{*this}
    , midilearn{*this}
    , mididecode{this}
    , vectorcontrol{this}
    , audioOut{}
    , partlock{}
    , legatoPart{0}
    , masterMono{false}
    // part[]
    , fadeAll{0}
    , fadeStep{0}
    , fadeStepShort{0}
    , fadeLevel{0}
    , samplerate{48000}
    , samplerate_f{float(samplerate)}
    , halfsamplerate_f{float(samplerate / 2)}
    , buffersize{512}
    , buffersize_f{float(buffersize)}
    , bufferbytes{int(buffersize*sizeof(float))}
    , oscilsize{1024}
    , oscilsize_f{float(oscilsize)}
    , halfoscilsize{oscilsize / 2}
    , halfoscilsize_f{float(halfoscilsize)}
    , oscil_sample_step_f{1.0}
    , oscil_norm_factor_pm{1.0}
    , oscil_norm_factor_fm{1.0}
    , sent_buffersize{0}
    , sent_bufferbytes{0}
    , sent_buffersize_f{0}
    , fixed_sample_step_f{0}
    , TransVolume{0}
    , Pvolume{0}
    , ControlStep{0}
    , Paudiodest{0}
    , Pkeyshift{0}
    , PbpmFallback{0}
    // Psysefxvol[][]
    // Psysefxsend[][]
    , syseffnum{0}
    // syseffEnable[]
    , inseffnum{0}
    // sysefx[]
    // insefx[]
    // Pinsparts[]
    , sysEffectUiCon{interchange.guiDataExchange.createConnection<EffectDTO>()}
    , insEffectUiCon{interchange.guiDataExchange.createConnection<EffectDTO>()}
    , partEffectUiCon{interchange.guiDataExchange.createConnection<EffectDTO>()}
    , sysEqGraphUiCon{interchange.guiDataExchange.createConnection<EqGraphDTO>()}
    , insEqGraphUiCon{interchange.guiDataExchange.createConnection<EqGraphDTO>()}
    , partEqGraphUiCon{interchange.guiDataExchange.createConnection<EqGraphDTO>()}
    , ctl{NULL}
    , microtonal{this}
    , fft{}
    , textMsgBuffer{TextMsgBuffer::instance()}
    , VUpeak{}
    , VUcopy{}
    , VUdata{}
    , VUcount{0}
    , VUready{false}
    , volume{0.0}
    // sysefxvol[][]
    // sysefxsend[][]
    , keyshift{0}
    , callbackGuiClosed{}
    , windowTitle{"Yoshimi" + asString(uniqueId)}
    , needsSaving{false}
    , channelTimer{0}
    , LFOtime{0}
    , songBeat{0.0}
    , monotonicBeat{0.0}
    , bpm{90}
    , bpmAccurate{false}
{
////////////////OOO Strip-down: core impl elided
}


void SynthEngine::defaults()
{
////////////////OOO Strip-down: core impl elided
}


void SynthEngine::setAllPartMaps()
{
////////////////OOO Strip-down: core impl elided
}

void SynthEngine::setReproducibleState(int seed)
{
////////////////OOO Strip-down: core impl elided
}


int SynthEngine::ReadBankRoot()
{
////////////////OOO Strip-down: core impl elided
}


int SynthEngine::ReadBank()
{
////////////////OOO Strip-down: core impl elided
}

char SynthEngine::partonoffRead(uint npart)
{
////////////////OOO Strip-down: core impl elided
}

int SynthEngine::MasterAudio(float *outl [NUM_MIDI_PARTS + 1], float *outr [NUM_MIDI_PARTS + 1], int to_process)
{
////////////////OOO Strip-down: core impl elided
}


void SynthEngine::ShutUp()
{
////////////////OOO Strip-down: core impl elided
}


bool SynthEngine::installBanks()
{
////////////////OOO Strip-down: core impl elided
}


bool SynthEngine::saveBanks()
{
////////////////OOO Strip-down: core impl elided
}


bool SynthEngine::loadHistory()
{
////////////////OOO Strip-down: core impl elided
}


bool SynthEngine::saveHistory()
{
////////////////OOO Strip-down: core impl elided
}


void SynthEngine::add2XML(XMLStore& xml)
{
////////////////OOO Strip-down: core impl elided
}


bool SynthEngine::loadXML(string const& filename)
{
////////////////OOO Strip-down: core impl elided
}


bool SynthEngine::getfromXML(XMLStore& xml)
{
////////////////OOO Strip-down: core impl elided
}
