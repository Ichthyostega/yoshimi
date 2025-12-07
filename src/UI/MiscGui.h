/*
    MiscGui.h - common link between GUI and synth

    Copyright 2016-2023 Will Godfrey & others

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

#ifndef MISCGUI_H
#define MISCGUI_H

#include <string>

#include "Misc/Config.h"
#include "Misc/FileMgrFuncs.h"
#include "Misc/FormatFuncs.h"
#include "Misc/MirrorData.h"

#include "UI/Themes.h"

#include "Interface/InterfaceAnchor.h"
using RoutingTag = GuiDataExchange::RoutingTag;

using std::string;
using file::saveText;
using file::loadText;
using func::string2int;

class InterChange;
#include "Effects/EffectMgr.h"

enum ValueType {
    VC_plainValue,
    VC_plainReverse,
    VC_pitchWheel,
    VC_percent127,
    VC_percent128,
    VC_percent255,
    VC_percent64_127,
    VC_PhaseOffset,
    VC_WaveHarmonicMagnitude,
    VC_GlobalFineDetune,
    VC_MasterVolume,
    VC_LFOfreq,
    VC_LFOfreqBPM,
    VC_LFOdepthFreq,
    VC_LFOdepthAmp,
    VC_LFOdepthFilter,
    VC_LFOdelay,
    VC_LFOstartphase,
    VC_LFOstartphaseRand,
    VC_EnvelopeDT,
    VC_EnvelopeFreqVal,
    VC_EnvelopeFilterVal,
    VC_EnvelopeAmpSusVal,
    VC_EnvelopeLinAmpSusVal,
    VC_EnvelopeBandwidthVal,
    VC_FilterFreq0, // Analog
    VC_FilterFreq1, // Formant
    VC_FilterFreq2, // StateVar
    VC_FilterFreqTrack0,
    VC_FilterFreqTrack1,
    VC_FilterQ,
    VC_FilterVelocityAmp,
    VC_FilterVelocitySense,
    VC_FormFilterClearness,
    VC_FormFilterSlowness,
    VC_FormFilterStretch,
    VC_InstrumentVolume,
    VC_ADDVoiceVolume,
    VC_ADDVoiceDelay,
    VC_PitchBend,
    VC_PartVolume,
    VC_PartHumaniseDetune,
    VC_PartHumaniseVelocity,
    VC_PanningRandom,
    VC_PanningStd,
    VC_EnvStretch,
    VC_LFOStretch,
    VC_FreqOffsetHz,
    VC_FixedFreqET,
    VC_FilterGain,
    VC_AmpVelocitySense,
    VC_FilterQAnalogUnused,
    VC_BandWidth,
    VC_SubBandwidth,
    VC_SubBandwidthScale,
    VC_SubBandwidthRel,
    VC_SubHarmonicMagnitude,
    VC_FXSysSend,
    VC_FXEchoVol,
    VC_FXEchoDelay,
    VC_FXEchoLRdel,
    VC_FXEchoDW,
    VC_FXReverbVol,
    VC_FXReverbTime,
    VC_FXReverbIDelay,
    VC_FXReverbHighPass,
    VC_FXReverbLowPass,
    VC_FXReverbDW,
    VC_FXReverbBandwidth,
    VC_FXdefaultVol,
    VC_FXdefaultFb,
    VC_FXChorusDepth,
    VC_FXChorusDelay,
    VC_FXlfoStereo,
    VC_FXlfofreq,
    VC_FXlfofreqBPM,
    VC_FXdefaultDW,
    VC_FXEQfreq,
    VC_FXEQq,
    VC_FXEQgain,
    VC_FXEQfilterGain,
    VC_FXDistVol,
    VC_FXDistLevel,
    VC_FXDistLowPass,
    VC_FXDistHighPass,
    VC_XFadeUpdate,
    VC_Retrigger,
    VC_RandWalkSpread,
};


int setSlider(float current, float normal);
int setKnob(float current, float normal);

string convert_value(ValueType type, float val);

string variable_prec_units(float v, const string& u, int maxPrec, bool roundup = false);
string custom_value_units(float v, const string& u, int prec=0);
void  custom_graph_dimensions(ValueType vt, int& w, int& h);
void custom_graphics(ValueType vt, float val,int W,int H);
ValueType getLFOdepthType(int group);
ValueType getLFOFreqType(int bpmEnabled);
ValueType getFilterFreqType(int type);
ValueType getFilterFreqTrackType(int offset);

int millisec2logDial(unsigned int);
unsigned int logDial2millisec(int);


/**
 * Base class mixed into MasterUI, which is the root of the Yoshimi FLTK UI.
 * Provides functions to establish communication with the Core.
 */
class GuiUpdates
{
protected:
    GuiUpdates(InterfaceAnchor&&);

    // must not be copied nor moved
    GuiUpdates(GuiUpdates &&)                =delete;
    GuiUpdates(GuiUpdates const&)            =delete;
    GuiUpdates& operator=(GuiUpdates &&)     =delete;
    GuiUpdates& operator=(GuiUpdates const&) =delete;

public:
    InterfaceAnchor anchor;

////////////////OOO Strip-down: core <-> UI communication severed
///////
private:
};

inline void saveWin(SynthEngine *synth, int w, int h, int x, int y, int o, std::string filename)
{
    std::string ID = std::to_string(synth->getUniqueId()) + "-";
    std::string values =  std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(w) + " " + std::to_string(h) + " " + std::to_string(o);
    saveText(values, file::configDir() + "/windows/" + ID + filename);
}

inline void loadWin(SynthEngine *synth, int& w, int& h, int& x, int& y, int& o, std::string filename)
{
    std::string ID = std::to_string(synth->getUniqueId()) + "-";
    std::string values = loadText(file::configDir() + "/windows/" + ID + filename);
    w = h = o = 0;
    if (values == "")
    {
        x = y = 80;
    }
    else
    {
        size_t pos = values.find(' ');
        if (pos == string::npos)
        {
            x = y = 80;
        }
        else
        {
            x = string2int(values.substr(0, pos));
            if (x < 4)
                x = 4;

            y = string2int(values.substr(pos));
            if (y < 10)
                y = 10;

            pos = values.find(' ', pos + 1);
            if (pos == string::npos)
                return;
            w = string2int(values.substr(pos));

            pos = values.find(' ', pos + 1);
            if (pos == string::npos)
                return;
            h = string2int(values.substr(pos));

            pos = values.find(' ', pos + 1);
            if (pos == string::npos)
                return;
            o = string2int(values.substr(pos));
        }
    }
}

inline int lastSeen(SynthEngine *synth, std::string filename)
{
    std::string ID = std::to_string(synth->getUniqueId()) + "-";
    std::string values = loadText(file::configDir() + "/windows/" + ID + filename);
    size_t pos = values.rfind(' ');
    if (pos == string::npos)
        return false;
    ++pos;
    return string2int(values.substr(pos));
}

inline void setVisible(SynthEngine *synth, bool v, std::string filename)
{
    std::string ID = std::to_string(synth->getUniqueId()) + "-";
    std::string values = loadText(file::configDir() + "/windows/" + ID + filename);
    size_t pos = values.rfind(' ');
    if (pos == string::npos)
        return;
    ++ pos;
    bool vis = string2int(values.substr(pos));
    if (vis == v)
        return;
    values.replace(pos, 1, std::to_string(v));
    saveText(values, file::configDir() + "/windows/" + ID + filename);
}

inline void checkSane(int& x, int& y, int& w, int& h, int defW, int defH, bool halfsize = false)
{
    int minX, minY, maxW, maxH;
    Fl::screen_xywh(minX, minY, maxW, maxH, x, y, w, h);

    // Pretend that this is the center screen, which makes calculations easier.
    // We will reverse this at the bottom.
    x -= minX;
    y -= minY;

    maxW -= 5; // wiggle room
    maxH -= 30; // space for minimal titlebar

    if ((w / defW) != (h / defH)) // ratio
        w = h / defH * defW; // doesn't matter which one we pick!

    int adjustW;
    int adjustH;
    if(halfsize)
    {
        adjustW = maxW / 2;
        adjustH = maxH / 2;
    }
    else
    {
        adjustW = maxW;
        adjustH = maxH;
    }
    if (w > maxW || h > maxH) // size
    {
        h = adjustH;
        w = adjustW;
        if (h / defH > w / defW)
        {
            h = w / defW * defH;
        }
        else
        {
            w = h / defH * defW;
        }
    }

    if ((x + w) > maxW) // position
    {
        x = maxW - w;
        if (x < 5)
            x = 5;
    }
    if ((y + h) > maxH)
    {
        y = maxH - h;
        if (y < 30)
            y = 30;
    }

    // Restore position relative to screen position.
    x += minX;
    y += minY;
}

#endif /*MISCGUI_H*/
