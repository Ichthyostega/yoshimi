/*
    EffectMgr.h - Effect manager, an interface between the program and effects

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2005 Nasca Octavian Paul
    Copyright 2009, Alan Calvert
    Copyright 2018,2023, Will Godfrey

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

    This file is a derivative of the ZynAddSubFX original.
*/

#ifndef EFFECTMGR_H
#define EFFECTMGR_H

#include "globals.h"
#include "Effects/Effect.h"
#include "Effects/Reverb.h"
#include "Effects/Echo.h"
#include "Effects/Chorus.h"
#include "Effects/Phaser.h"
#include "Effects/Alienwah.h"
#include "Effects/Distorsion.h"
#include "Effects/EQ.h"
#include "Effects/DynamicFilter.h"
#include "Misc/Alloc.h"
#include "Misc/XMLwrapper.h"
#include "Misc/SynthEngine.h"
#include "Params/FilterParams.h"



class EffectMgr : public ParamBase
{
    public:
       ~EffectMgr() = default;
        EffectMgr(const bool insertion_, SynthEngine *_synth);

        void add2XML(XMLwrapper& xml);
        void defaults(void);
        void getfromXML(XMLwrapper& xml);

        void out(float *smpsl, float *smpsr);

        void setdryonly(bool value);

        float sysefxgetvolume(void);

        void cleanup(void);

        void changeeffect(int nefx_);
        int geteffect(void);

        void changepreset(unsigned char npreset);
        unsigned char getpreset(void);
        void seteffectpar(int npar, unsigned char value);
        unsigned char geteffectpar(int npar);
        void getAllPar(EffectParArray&) const;

        SynthEngine *getSynthEngine() {return synth;}

        Samples efxoutl;
        Samples efxoutr;
        bool insertion; // the effect is connected as insertion effect (or not)

        // used by UI
        float getEQfreqresponse(float freq);

        FilterParams *filterpars;

    private:
        int nefx;
        bool dryonly;
        unique_ptr<Effect> efx;
};

class LimitMgr
{
    public:
        float geteffectlimits(CommandBlock *getData);
};




/**
 * Data record used to transport effect settings into the UI
 */
struct EffectDTO
{
    int effType{-1};
    uchar effNum{0};
    bool enabled{false};
    bool changed{false};
    bool isInsert{false};
    uchar currPreset{0};
    int insertFxRouting{-1};
    uchar partFxRouting{1};
    bool  partFxBypass{false};

    EffectParArray param{0};
    //////////////////////////////////////////////////TODO 2/24 as partial workaround until all further direct core accesses are addressed
    EffectMgr* eff_in_core_TODO_deprecated;
};

/**
 * Graph data for the EQ frequency response display in the UI
 */
struct EqGraphDTO
{
    std::array<float, MAX_EQ_BANDS> response{0};  ////TODO 2/24 probably not MAX_EQ_BANDS slots, rather granularity of display in the UI
};

#endif /*EFFECTMGR_H*/

