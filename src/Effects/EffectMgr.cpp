/*
    EffectMgr.cpp - Effect manager, an interface between the program and effects

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2009 Nasca Octavian Paul
    Copyright 2009-2011, Alan Calvert
    Copyright 2017-2023, Will Godfrey

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

    This file is derivative of ZynAddSubFX original code.

*/

#include <iostream>

#include "Misc/XMLStore.h"
#include "Misc/SynthEngine.h"
#include "Effects/EffectMgr.h"
#include "Effects/EQ.h"

EffectMgr::EffectMgr(const bool insertion_, SynthEngine& _synth) :
    ParamBase{_synth},
    efxoutl{size_t(_synth.buffersize)},
    efxoutr{size_t(_synth.buffersize)},
    insertion{insertion_},
    filterpars{NULL},
    effectType{0}, // type none resolves to zero internally
    dryonly{false},
    efx{}
{
    defaults();
}




void EffectMgr::defaults()
{
    changeeffect(0); // type none resolves to zero internally
    setdryonly(false);
}


// Change the effect
void EffectMgr::changeeffect(int _nefx)
{
    cleanup();
    if (effectType == _nefx)
        return;
    effectType = _nefx;
    switch (effectType + EFFECT::type::none)
    {
        case EFFECT::type::reverb:
            efx.reset(new Reverb{insertion, efxoutl.get(), efxoutr.get(), synth});
            break;

        case EFFECT::type::echo:
            efx.reset(new Echo{insertion, efxoutl.get(), efxoutr.get(), synth});
            break;

        case EFFECT::type::chorus:
            efx.reset(new Chorus{insertion, efxoutl.get(), efxoutr.get(), synth});
            break;

        case EFFECT::type::phaser:
            efx.reset(new Phaser{insertion, efxoutl.get(), efxoutr.get(), synth});
            break;

        case EFFECT::type::alienWah:
            efx.reset(new Alienwah{insertion, efxoutl.get(), efxoutr.get(), synth});
            break;

        case EFFECT::type::distortion:
            efx.reset(new Distorsion{insertion, efxoutl.get(), efxoutr.get(), synth});
            break;

        case EFFECT::type::eq:
            efx.reset(new EQ{insertion, efxoutl.get(), efxoutr.get(), synth});
            break;

        case EFFECT::type::dynFilter:
            efx.reset(new DynamicFilter{insertion, efxoutl.get(), efxoutr.get(), synth});
            break;

            // put more effect here
        default:
            efx.reset();
            break; // no effect (thru)
    }
    if (efx)
        filterpars = efx->filterpars;
}


// Obtain the effect number
int EffectMgr::geteffect()
{
    return (effectType);
}


// Cleanup the current effect
void EffectMgr::cleanup()
{
    memset(efxoutl.get(), 0, synth.bufferbytes);
    memset(efxoutr.get(), 0, synth.bufferbytes);
    if (efx)
        efx->cleanup();
}


// Get the preset of the current effect
uchar EffectMgr::getpreset()
{
    return efx? efx->Ppreset
              : 0;
}


// Change the preset of the current effect
void EffectMgr::changepreset(uchar npreset)
{
    if (efx)
        efx->setpreset(npreset);
}


// Change a parameter of the current effect
void EffectMgr::seteffectpar(int npar, uchar value)
{
    if (!efx)
        return;
    efx->changepar(npar, value);
}


// Get a parameter of the current effect
uchar EffectMgr::geteffectpar(int npar)
{
    if (!efx)
        return 0;
    return efx->getpar(npar);
}

void EffectMgr::getAllPar(EffectParArray& target) const
{
    if (efx)
        efx->getAllPar(target);
    else
        target = {0};
}



// Apply the effect
void EffectMgr::out(float *smpsl, float *smpsr)
{
    if (!efx)
    {
        if (!insertion)
        {
            memset(smpsl, 0, synth.sent_bufferbytes);
            memset(smpsr, 0, synth.sent_bufferbytes);
            memset(efxoutl.get(), 0, synth.sent_bufferbytes);
            memset(efxoutr.get(), 0, synth.sent_bufferbytes);
        }
        return;
    }
    memset(efxoutl.get(), 0, synth.sent_bufferbytes);
    memset(efxoutr.get(), 0, synth.sent_bufferbytes);
    efx->out(smpsl, smpsr);

    if (effectType == (EFFECT::type::eq - EFFECT::type::none))
    {   // this is need only for the EQ effect
        memcpy(smpsl, efxoutl.get(), synth.sent_bufferbytes);
        memcpy(smpsr, efxoutr.get(), synth.sent_bufferbytes);
        return;
    }

    // Insertion effect
    if (insertion != 0)
    {
        for (int i = 0; i < synth.sent_buffersize; ++i)
        {
            float volume = efx->volume.getAndAdvanceValue();
            float v1, v2;
            if (volume < 0.5f)
            {
                v1 = 1.0f;
                v2 = volume * 2.0f;
            }
            else
            {
                v1 = (1.0f - volume) * 2.0f;
                v2 = 1.0f;
            }
            if (effectType == (EFFECT::type::reverb - EFFECT::type::none)
                || effectType==(EFFECT::type::echo - EFFECT::type::none))
                v2 *= v2; //  wet function is not linear for Reverb/Echo

            if (dryonly)
            {
                // this is used for instrument effect only
                smpsl[i] *= v1;
                smpsr[i] *= v1;
                efxoutl[i] *= v2;
                efxoutr[i] *= v2;
            }
            else
            {
                // normal instrument/insertion effect
                smpsl[i] = smpsl[i] * v1 + efxoutl[i] * v2;
                smpsr[i] = smpsr[i] * v1 + efxoutr[i] * v2;
            }
        }
    }
    else
    { // System effect
        for (int i = 0; i < synth.sent_buffersize; ++i)
        {
            float volume = efx->volume.getAndAdvanceValue();
            efxoutl[i] *= 2.0f * volume;
            efxoutr[i] *= 2.0f * volume;
            smpsl[i] = efxoutl[i];
            smpsr[i] = efxoutr[i];
        }
    }
}


// Get the effect volume for the system effect
float EffectMgr::sysefxgetvolume()
{
    return (!efx) ? 1.0f : efx->outvolume.getValue();
}


/**
 * Prepare a LUT for the UI to display the current
 * amplitude/frequency response of the EQ's filter
 */
void EffectMgr::renderEQresponse(EQGraphArray& lut) const
{
    if (effectType != (EFFECT::type::eq - EFFECT::type::none))
        return;
    auto eqImpl = static_cast<EQ const*> (efx.get());
    eqImpl->renderResponse(lut);
}


void EffectMgr::setdryonly(bool value)
{
    dryonly = value;
}


void EffectMgr::add2XML(XMLtree& xmlEffect)
{
    xmlEffect.addPar_int("type", geteffect());

    if (!efx or 0 == geteffect())
        return;

    xmlEffect.addPar_int("preset", efx->Ppreset);
    XMLtree xmlParams = xmlEffect.addElm("EFFECT_PARAMETERS");
        for (uint n = 0; n < 128; ++n)
        {   // \todo evaluate who should oversee saving and loading of parameters
            uchar paramVal = geteffectpar(n);
            if (paramVal == 0)
                continue;
            XMLtree xmlPar = xmlParams.addElm("par_no", n);
            xmlPar.addPar_int("par", paramVal);
        }
        if (filterpars)
        {
            XMLtree xmlFilter = xmlParams.addElm("FILTER");
            filterpars->add2XML(xmlFilter);
        }
}


void EffectMgr::getfromXML(XMLtree& xmlEffect)
{
    assert(xmlEffect);
    // Instantiate Effect implementation according to the given effect-type
    changeeffect(xmlEffect.getPar_127("type", geteffect()));
    if (!efx or 0 == geteffect())
        return;   // disabled; no further settings to retrieve

    bool isChanged{false};
    changepreset(xmlEffect.getPar_127("preset", efx->Ppreset));
    if (XMLtree xmlParams = xmlEffect.getElm("EFFECT_PARAMETERS"))
    {
        for (uint n = 0; n < 128; ++n)
        {
            uchar defaultVal = geteffectpar(n); // find default
            seteffectpar(n, 0); // erase effect parameter
            if (XMLtree xmlPar = xmlParams.getElm("par_no", n))
            {
                seteffectpar(n, xmlPar.getPar_127("par", int(defaultVal)));
                isChanged |= (defaultVal != geteffectpar(n));
            }
        }
        seteffectpar(-1, isChanged);
        if (filterpars)
            if (XMLtree xmlFilter = xmlParams.getElm("FILTER"))
                filterpars->getfromXML(xmlFilter);
    }
    cleanup();
}


float LimitMgr::geteffectlimits(CommandBlock *getData)
{
    int effType = getData->data.kit;
    float value = 0;
    switch (effType)
    {
        case EFFECT::type::none:
            value = 0;
            break;
        case EFFECT::type::reverb:
            Revlimit reverb;
            value = reverb.getlimits(getData);
            break;
        case EFFECT::type::echo:
            Echolimit echo;
            value = echo.getlimits(getData);
            break;
        case EFFECT::type::chorus:
            Choruslimit chorus;
            value = chorus.getlimits(getData);
            break;
        case EFFECT::type::phaser:
            Phaserlimit phaser;
            value = phaser.getlimits(getData);
            break;
        case EFFECT::type::alienWah:
            Alienlimit alien;
            value = alien.getlimits(getData);
            break;
        case EFFECT::type::distortion:
            Distlimit dist;
            value = dist.getlimits(getData);
            break;
        case EFFECT::type::eq:
            EQlimit EQ;
            value = EQ.getlimits(getData);
            break;
        case EFFECT::type::dynFilter:
            Dynamlimit dyn;
            value = dyn.getlimits(getData);
            break;
        default:
            value = EFFECT::type::count - EFFECT::type::none;
            break;
    }
    return value;
}
