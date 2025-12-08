/*
    Config.cpp - Configuration file functions

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2005 Nasca Octavian Paul
    Copyright 2009-2011, Alan Calvert
    Copyright 2013, Nikita Zlobin
    Copyright 2014-2024, Will Godfrey & others

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

#include <regex>

#include "ConfBuild.h"
#include "Misc/Config.h"
#include "Misc/FormatFuncs.h"
#include "Misc/TextMsgBuffer.h"

#ifdef GUI_FLTK
#ifdef YOSHIMI_FORCE_X11
    extern bool fl_disable_wayland = true;
#endif
    #include "MasterUI.h"
#endif

using func::string2int;
using func::string2uint;

using std::string;


namespace { // Implementation details...

    const uint XML_UNCOMPRESSED{0};

    TextMsgBuffer& textMsgBuffer = TextMsgBuffer::instance();


    static std::regex VERSION_SYNTAX{R"~((\d+)(?:\.(\d+))?(?:\.(\d+))?)~", std::regex::optimize};

    VerInfo parseVersion(string const& spec)
    {
        std::smatch mat;
        if (std::regex_search(spec, mat, VERSION_SYNTAX))
            return VerInfo{                string2uint(mat[1])
                          ,mat[2].matched? string2uint(mat[2]) : 0
                          ,mat[3].matched? string2uint(mat[3]) : 0
                          };
        else
            return VerInfo{0};
    }
}

/**
 * Implementation: parse string with program version specification
 */
VerInfo::VerInfo(string const& spec)
    : VerInfo{parseVersion(spec)}
    { }



uchar panLaw = 1;

bool       Config::showSplash{true};
bool       Config::singlePath{false};
bool       Config::autoInstance{false};
bitset<32> Config::activeInstances{0};
int        Config::showCLIcontext{1};

string Config::globalJackSessionUuid = "";

const VerInfo Config::VER_YOSHI_CURR{YOSHIMI_VERSION};
const VerInfo Config::VER_ZYN_COMPAT{2,4,3};


Config::Config(SynthEngine& synthInstance)
    : synth{synthInstance}
    , isLV2{false}
    , isMultiFeed{false}
    , build_ID{BUILD_NUMBER}
    , loadedConfigVer{VER_YOSHI_CURR}
    , incompatibleZynFile{false}
    , runSynth{false}          // will be set by Instance::startUp()
    , finishedCLI{true}
    , isLittleEndian{true}
    , virKeybLayout{0}
    , engineChanged{false}
    , midiChanged{false}
    , alsaMidiType{1}          // search
    , audioDevice{"default"}
    , midiDevice{"default"}
    , jackServer{"default"}
    , jackMidiDevice{"default"}
    , startJack{false}
    , connectJackaudio{true}
    , connectJackChanged{false}
    , alsaAudioDevice{"default"}
    , alsaMidiDevice{"default"}
    , loadDefaultState{false}
    , defaultStateName{}
    , defaultSession{}
    , configFile{}
    , paramsLoad{}
    , instrumentLoad{}
    , load2part{0}
    , midiLearnLoad{}
    , rootDefine{}
    , stateFile{}
    , guiThemeID{0}
    , guiTheme{}
    , remoteGuiTheme{}
    , restoreJackSession{false}
    , jackSessionFile{}
    , sessionStage{_SYS_::type::Normal}
    , Interpolation{0}
    , xmlType{0}
    , instrumentFormat{1}
    , enableProgChange{true}   // default will be inverted
    , toConsole{false}
    , consoleTextSize{12}
    , hideErrors{false}
    , showTimes{false}
    , logXMLheaders{false}
    , xmlmax{false}
    , gzipCompression{3}
    , enablePartReports{false}
    , samplerate{48000}
    , rateChanged{false}
    , buffersize{256}
    , bufferChanged{false}
    , oscilsize{512}
    , oscilChanged{false}
    , showGui{true}
    , storedGui{true}
    , guiChanged{false}
    , showCli{true}
    , storedCli{true}
    , cliChanged{false}
    , banksChecked{false}
    , panLaw{1}
    , configChanged{false}
    , handlePadSynthBuild{0}
    , rtprio{40}
    , midi_bank_root{0}        // 128 is used as 'disabled'
    , midi_bank_C{32}          // 128 is used as 'disabled'
    , midi_upper_voice_C{128}  // disabled
    , enableOmni{true}
    , enable_NRPN{true}
    , ignoreResetCCs{false}
    , monitorCCin{false}
    , showLearnedCC{true}
    , numAvailableParts{NUM_MIDI_CHANNELS}
    , currentPart{0}
    , currentBank{0}
    , currentRoot{0}
    , bankHighlight{false}
    , lastBankPart{UNUSED}
    , presetsRootID{0}
    , tempBank{0}
    , tempRoot{0}
    , VUcount{0}
    , channelSwitchType{0}
    , channelSwitchCC{128}     // disabled
    , channelSwitchValue{0}
    , nrpnL{127}               // off
    , nrpnH{127}               // off
    , dataL{0xff}              // disabled
    , dataH{0x80}
    , nrpnActive{false}
    , vectordata{}
    , logList{}
    , manualFile{}
    , exitType{}
    , genTmp1{}
    , genTmp2{}
    , genTmp3{}
    , genTmp4{}
    , genMixl{}
    , genMixr{}
    , sigIntActive{0}
    , ladi1IntActive{0}
    , jsessionSave{0}
    , programcommand{"yoshimi"}
    , jackSessionDir{}
    , baseConfig{}
    , presetList{}
    , presetDir{}
    , logHandler{[this](string const& msg, char tostderr){ this->Log(msg,tostderr); }}
{
    std::cerr.precision(4);
}



void Config::Log(string const& msg, char tostderr)
{
////////////////OOO Strip-down: core impl elided
}


