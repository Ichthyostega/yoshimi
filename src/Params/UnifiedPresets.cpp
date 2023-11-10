/*
    UnifiedPresets.cpp - Presets and Clipboard management

    Copyright 2018-2023 Will Godfrey

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

#include <string>

#include "Misc/XMLwrapper.h"
#include "Misc/SynthEngine.h"
#include "Misc/TextMsgBuffer.h"
#include "Misc/FileMgrFuncs.h"
#include "Interface/TextLists.h"
#include "Effects/EffectMgr.h"
#include "Params/ADnoteParameters.h"
#include "Params/SUBnoteParameters.h"
#include "Params/PADnoteParameters.h"
#include "Params/FilterParams.h"
#include "Params/LFOParams.h"
#include "Params/EnvelopeParams.h"


using std::string;

/*
   type flags (set)
        List  - all entries of section type
        Group - preset extensin and name
        Copy  - from section to file
        Paste - from file to section

   no name given - from/to clipboard))
 */

string UnifiedPresets::section(SynthEngine *_synth, CommandBlock *getData)
{
    synth = _synth;
    //synth->CBtest(getData, true);
    int type = getData->data.type;
    int value = getData->data.value;
    if (type == TOPLEVEL::type::List && value == 1)
    {
        string group = findPresetType(getData, value);
        value = synth->textMsgBuffer.push(group);
        return findPresetType(getData, value); // human readble extension
    }

    string name = findPresetType(getData);
    if (name.empty())
    {
        name = "No section presets in this context";
        return name;
    }
    string dirname = synth->getRuntime().presetsDirlist[synth->getRuntime().presetsRootID];
    if (dirname.empty())
    {
        name = "Directory empty";
        return name;
    }

    if (type == TOPLEVEL::type::List)
    {
        list(dirname, name);
    }
    else
    {
        if (type & TOPLEVEL::type::Copy)
        {
            saveUnif(getData);
            name = "";
        }
        else if (type & TOPLEVEL::type::Paste)
        {
            if (value == 0)
                load(getData);
            else
                remove(getData);
        }
    }
    return name;
}

string UnifiedPresets::listpos(int count)
{
    return presetgroups[count * 2 + human];
}

string UnifiedPresets::findPresetType(CommandBlock *getData, int _human)
{
    human = _human;
    int npart = getData->data.part;
    int kitItem = getData->data.kit;
    int engineType = getData->data.engine;
    int insert = getData->data.insert;
    int parameter = getData->data.parameter;
    int offset = getData->data.offset;
    string name = "";

    if (npart != TOPLEVEL::section::systemEffects && npart != TOPLEVEL::section::insertEffects && npart > TOPLEVEL::section::part64)
        return name;
    if (kitItem >= EFFECT::type:: none && kitItem < EFFECT::type::count)
    {
        if (insert == TOPLEVEL::insert::filterGroup)
        {
            if (offset == UNUSED)
                return listpos(0);//"Pfilter";
            else
                return listpos(1);//"Pfiltern";
        }
        else
            return listpos(2);//"Peffect";
    }

    switch (insert)
    {
        case TOPLEVEL::insert::filterGroup:
            {
                if (offset == UNUSED)
                    name = listpos(3);//"Pfilter";
                else
                    name = listpos(4);//"Pfiltern";
            }
            break;

        case TOPLEVEL::insert::oscillatorGroup:
            name = listpos(5);//"Poscilgen";
            break;
        case TOPLEVEL::insert::resonanceGroup:
            name = listpos(6);//"Presonance";
            break;
        case TOPLEVEL::insert::LFOgroup:
            switch (parameter)
            {
                case 0:
                    name = listpos(7);//"Plfoamplitude";
                    break;
                case 1:
                    name = listpos(8);//"Plfofrequency";
                    break;
                case 2:
                    name = listpos(9);//"Plfofilter";
                    break;
            }
            break;
        case TOPLEVEL::insert::envelopeGroup:
            switch (parameter)
            {
                case 0:
                    name = listpos(10);//"Penvamplitude";
                    break;
                case 1:
                    name = listpos(11);//"Penvfrequency";
                    break;
                case 2:
                    name = listpos(12);//"Penvfilter";
                    break;
                case 3:
                    name = listpos(13);//"Penvbandwidth";
                    break;
            }
            break;
    }
    if (!name.empty())
        return name;

    if (engineType >= PART::engine::addVoice1 && engineType < PART::engine::addVoiceModEnd)
    {
        return listpos(14);//"Padsythn"; // all voice and modulator level have the same extension
    }

    switch (engineType)
    {
        case PART::engine::addSynth:
            name = listpos(15);//"Padsyth";
            break;
        case PART::engine::subSynth:
            name = listpos(16);//presetgroups[32+human];//"Psubsyth";
            break;
        case PART::engine::padSynth:
            name = listpos(17);//"Ppadsyth";
            break;
    }
    return name;
}


void UnifiedPresets::list(string dirname, string& name)
{

        string list = "";
        file::presetsList(dirname, name, presetList);
        if(presetList.size() > 1)
        {
            sort(presetList.begin(), presetList.end());
        }

        for (auto it = begin (presetList); it != end (presetList); ++it)
        {
            string tmp = file::findLeafName(*it);
            size_t pos = tmp.rfind('.');
            tmp = tmp.substr(0, pos);
            list += (tmp + "\n");
        }
        if (list.empty())
            name = "No presets of this type found";
        else
            name = list;
        return;
}


string UnifiedPresets::findXML(XMLwrapper *xml, CommandBlock *getData, bool isLoad)
{
    int value = int(getData->data.value);
    int npart = getData->data.part;
    int kitItem = getData->data.kit;
    int engineType = getData->data.engine;
    int insert = getData->data.insert;

    string name = "";

    if (kitItem == EFFECT::type::dynFilter && insert == TOPLEVEL::insert::filterGroup)
    {
        ; // passed on to filters
    }
    else if (kitItem >= EFFECT::type:: none && kitItem < EFFECT::type::count)
    {
        {
            EffectMgr *sectionType;
            if (npart == TOPLEVEL::section::systemEffects)
            {
                sectionType = synth->sysefx[value];
            }
            else if (npart == TOPLEVEL::section::insertEffects)
            {
                sectionType = synth->insefx[value];
            }
            else
            {
                sectionType = synth->part[npart]->partefx[value];
            }
            name = "Peffect";

            if (isLoad)
            {
                sectionType->defaults();
                xml->enterbranch(name);
                sectionType->getfromXML(xml);
                xml->exitbranch();
            }
            else
            {
                xml->beginbranch(name);
                sectionType->add2XML(xml);
                xml->endbranch();
            }
        }
    }

    if (name.empty())
    {

        switch (insert)
        {
            case TOPLEVEL::insert::resonanceGroup:
            {
                name = resonanceXML(xml, getData, isLoad);
            }
            break;

            case TOPLEVEL::insert::oscillatorGroup:
            {
                name = oscilXML(xml, getData, isLoad);
            }
            break;

            case TOPLEVEL::insert::filterGroup:
            {
                name = filterXML(xml, getData, isLoad);
            }
            break;

            case TOPLEVEL::insert::LFOgroup:
            {
                name = lfoXML(xml, getData, isLoad);
            }
            break;

            case TOPLEVEL::insert::envelopeGroup:
            {
                name = envelopeXML(xml, getData, isLoad);
            }
            break;
        }
    }
    if (!name.empty())
        return name;

    if (engineType == PART::engine::addSynth)
    {
        name = "Padsyth";
        ADnoteParameters *sectionType = synth->part[npart]->kit[kitItem].adpars;

        if (isLoad)
        {
            sectionType->defaults();
            xml->enterbranch(name);
            sectionType->getfromXML(xml);
            xml->exitbranch();
        }
        else
        {
            xml->beginbranch(name);
            sectionType->add2XML(xml);
            xml->endbranch();
        }
    }

    else if (engineType >= PART::engine::addVoice1)
    {
        name = "Padsythn";
        ADnoteParameters *sectionType = synth->part[npart]->kit[kitItem].adpars;
        size_t voice = engineType - PART::engine::addVoice1;

        if (isLoad)
        {
            sectionType->voiceDefaults(voice);
            xml->enterbranch(name);
            sectionType->getfromXMLsection(xml, voice);
            xml->exitbranch();
        }
        else
        {
            xml->beginbranch(name);
            sectionType->add2XMLsection(xml, voice);
            xml->endbranch();
        }
    }

    else if (engineType == PART::engine::subSynth)
    {
        name = "Psubsyth";
        SUBnoteParameters *sectionType = synth->part[npart]->kit[kitItem].subpars;

        if (isLoad)
        {
            sectionType->defaults();
            xml->enterbranch(name);
            sectionType->getfromXML(xml);
            xml->endbranch();
        }
        else
        {
            xml->beginbranch(name);
            sectionType->add2XML(xml);
            xml->endbranch();
        }
    }

    else if (engineType == PART::engine::padSynth)
    {
        name = "Ppadsyth";
        PADnoteParameters * sectionType = synth->part[npart]->kit[kitItem].padpars;

        if (isLoad)
        {
            sectionType->defaults();
            xml->enterbranch(name);
            sectionType->getfromXML(xml);
            xml->exitbranch();
        }
        else
        {
            xml->beginbranch(name);
            sectionType->add2XML(xml);
            xml->endbranch();
        }
    }

    return name;
}


string UnifiedPresets::resonanceXML(XMLwrapper *xml,CommandBlock *getData, bool isLoad)
{
    int npart = getData->data.part;
    int kitItem = getData->data.kit;
    int engineType = getData->data.engine;
    string name = "Presonance";
    Resonance *sectionType;

    if (engineType == PART::engine::addSynth)
    {
        sectionType = synth->part[npart]->kit[kitItem].adpars->GlobalPar.Reson;
    }
    else if (engineType == PART::engine::padSynth)
    {
        sectionType = synth->part[npart]->kit[kitItem].padpars->resonance.get();
    }
    else
        return "";

    if (isLoad)
    {
        xml->enterbranch(name);
        sectionType->getfromXML(xml);
        xml->exitbranch();
    }
    else
    {
        xml->beginbranch(name);
        sectionType->add2XML(xml);
        xml->endbranch();
    }

    return name;
}


string UnifiedPresets::oscilXML(XMLwrapper *xml,CommandBlock *getData, bool isLoad)
{
    int npart = getData->data.part;
    int kitItem = getData->data.kit;
    int engineType = getData->data.engine;
    string name = "Poscilgen";

    OscilParameters *sectionType;

    if (engineType >= (PART::engine::addVoice1))
    {
        if (engineType >= PART::engine::addMod1)
        {
            engineType -= NUM_VOICES;
            sectionType = synth->part[npart]->kit[kitItem].adpars->VoicePar[engineType - PART::engine::addVoice1].POscilFM;
        }
        else
        {
            sectionType = synth->part[npart]->kit[kitItem].adpars->VoicePar[engineType - PART::engine::addVoice1].POscil;
        }
    }
    else if (engineType == PART::engine::padSynth)
    {
        sectionType = synth->part[npart]->kit[kitItem].padpars->POscil.get();
    }
    else
        return "";

    if (isLoad)
    {
        xml->enterbranch(name);
        sectionType->getfromXML(xml);
        xml->exitbranch();
    }
    else
    {
        xml->beginbranch(name);
        sectionType->add2XML(xml);
        xml->endbranch();
    }

    return name;
}


string UnifiedPresets::filterXML(XMLwrapper *xml, CommandBlock *getData, bool isLoad)
{
    int npart = getData->data.part;
    int kitItem = getData->data.kit;
    int engineType = getData->data.engine;
    int offset = getData->data.offset;
    string name;
    if (offset == UNUSED)
    {
        name = "Pfilter";
    }
    else
    {
        name = "Pfiltern";
    }

    FilterParams *sectionType;

    // top level
    if (npart == TOPLEVEL::section::systemEffects)
    {
        sectionType = synth->sysefx[0]->filterpars;
    }
    else if (npart == TOPLEVEL::section::insertEffects)
    {
        sectionType = synth->insefx[0]->filterpars;
    }

    // part level
    else if (kitItem == EFFECT::type::dynFilter)
    {
        sectionType = synth->part[npart]->partefx[0]->filterpars;
    }
    else if (engineType == PART::engine::addSynth)
    {
        sectionType = synth->part[npart]->kit[kitItem].adpars->GlobalPar.GlobalFilter;
    }
    else if (engineType >= PART::engine::addVoice1)
    {
        sectionType = synth->part[npart]->kit[kitItem].adpars->VoicePar[engineType - PART::engine::addVoice1].VoiceFilter;
    }
    else if (engineType == PART::engine::subSynth)
    {
        sectionType = synth->part[npart]->kit[kitItem].subpars->GlobalFilter;
    }
    else if (engineType == PART::engine::padSynth)
    {
        sectionType = synth->part[npart]->kit[kitItem].padpars->GlobalFilter.get();
    }
    else
    {
        return "";
    }

    if (isLoad)
    {
        if (offset == UNUSED)
        {
            xml->enterbranch(name);
            sectionType->getfromXML(xml);
            xml->exitbranch();
        }
        else
        {
            xml->enterbranch(name);
            sectionType->getfromXMLsection(xml, offset);
            xml->exitbranch();
        }
    }
    else
    {
        if (offset == UNUSED)
        {
            xml->beginbranch(name);
            sectionType->add2XML(xml);
            xml->endbranch();
        }
        else
        {
            xml->beginbranch(name);
            sectionType->add2XMLsection(xml, offset);
            xml->endbranch();
        }
    }

    return name;
}


string UnifiedPresets::lfoXML(XMLwrapper *xml,CommandBlock *getData, bool isLoad)
{
    int npart = getData->data.part;
    int kitItem = getData->data.kit;
    int engineType = getData->data.engine;
    int parameter = getData->data.parameter;

    string name;
    LFOParams *sectionType;

    if (engineType == PART::engine::addSynth)
    {
        switch (parameter)
        {
            case 0:
                name = "Plfoamplitude";
                sectionType = synth->part[npart]->kit[kitItem].adpars->GlobalPar.AmpLfo;
            break;
            case 1:
                name = "Plfofrequency";
                sectionType = synth->part[npart]->kit[kitItem].adpars->GlobalPar.FreqLfo;
            break;
            case 2:
                name = "Plfofilter";
                sectionType = synth->part[npart]->kit[kitItem].adpars->GlobalPar.FilterLfo;
            break;
        }
    }
    else if (engineType >= PART::engine::addVoice1)
    {
        switch (parameter)
        {
            case 0:
                name = "Plfoamplitude";
                sectionType = synth->part[npart]->kit[kitItem].adpars->VoicePar[engineType - PART::engine::addVoice1].AmpLfo;
            break;
            case 1:
                name = "Plfofrequency";
                sectionType = synth->part[npart]->kit[kitItem].adpars->VoicePar[engineType - PART::engine::addVoice1].FreqLfo;
            break;
            case 2:
                name = "Plfofilter";
                sectionType = synth->part[npart]->kit[kitItem].adpars->VoicePar[engineType - PART::engine::addVoice1].FilterLfo;
            break;
        }
    }
    else if (engineType == PART::engine::padSynth)
    {
        switch (parameter)
        {
            case 0:
                name = "Plfoamplitude";
                sectionType = synth->part[npart]->kit[kitItem].padpars->AmpLfo.get();
            break;
            case 1:
                name = "Plfofrequency";
                sectionType = synth->part[npart]->kit[kitItem].padpars->FreqLfo.get();
            break;
            case 2:
                name = "Plfofilter";
                sectionType = synth->part[npart]->kit[kitItem].padpars->FilterLfo.get();
            break;
        }
    }
    if (name.empty())
        return "";


    if (isLoad)
    {
        xml->enterbranch(name);
        sectionType->getfromXML(xml);
        xml->exitbranch();
    }
    else
    {
        xml->beginbranch(name);
        sectionType->add2XML(xml);
        xml->endbranch();
    }

    return name;
}


string UnifiedPresets::envelopeXML(XMLwrapper *xml,CommandBlock *getData, bool isLoad)
{
    int npart = getData->data.part;
    int kitItem = getData->data.kit;
    int engineType = getData->data.engine;
    int parameter = getData->data.parameter;

    string name;
    EnvelopeParams *sectionType;
    if (engineType == PART::engine::addSynth)
    {
        switch (parameter)
        {
            case 0:
                name = "Penvamplitude";
                sectionType = synth->part[npart]->kit[kitItem].adpars->GlobalPar.AmpEnvelope;
            break;
            case 1:
                name = "Penvfrequency";
                sectionType = synth->part[npart]->kit[kitItem].adpars->GlobalPar.FreqEnvelope;
            break;
            case 2:
                name = "Penvfilter";
                sectionType = synth->part[npart]->kit[kitItem].adpars->GlobalPar.FilterEnvelope;
            break;
        }
    }

    else if (engineType >= PART::engine::addVoice1)
    {
        switch (parameter)
        {
            case 0:
                name = "Penvamplitude";
                sectionType = synth->part[npart]->kit[kitItem].adpars->VoicePar[engineType - PART::engine::addVoice1].AmpEnvelope;
            break;
            case 1:
                name = "Penvfrequency";
                sectionType = synth->part[npart]->kit[kitItem].adpars->VoicePar[engineType - PART::engine::addVoice1].FreqEnvelope;
            break;
            case 2:
                name = "Penvfilter";
                sectionType = synth->part[npart]->kit[kitItem].adpars->VoicePar[engineType - PART::engine::addVoice1].FilterEnvelope;
            break;
        }
    }

    else if (engineType == PART::engine::subSynth)
    {
        switch (parameter)
        {
            case 0:
                name = "Penvamplitude";
                sectionType = synth->part[npart]->kit[kitItem].subpars->AmpEnvelope;
            break;
            case 1:
                name = "Penvfrequency";
                sectionType = synth->part[npart]->kit[kitItem].subpars->FreqEnvelope;
            break;
            case 2:
                name = "Penvfilter";
                sectionType = synth->part[npart]->kit[kitItem].subpars->GlobalFilterEnvelope;
            break;
            case 3:
                name = "Penvbandwidth";
                sectionType = synth->part[npart]->kit[kitItem].subpars->BandWidthEnvelope;
            break;
        }
    }

    else if (engineType == PART::engine::padSynth)
    {
        switch (parameter)
        {
            case 0:
                name = "Penvamplitude";
                sectionType = synth->part[npart]->kit[kitItem].padpars->AmpEnvelope.get();
            break;
            case 1:
                name = "Penvfrequency";
                sectionType = synth->part[npart]->kit[kitItem].padpars->FreqEnvelope.get();
            break;
            case 2:
                name = "Penvfilter";
                sectionType = synth->part[npart]->kit[kitItem].padpars->FilterEnvelope.get();
            break;
        }
    }
    if (name.empty())
        return "";


    if (isLoad)
    {
        xml->enterbranch(name);
        sectionType->getfromXML(xml);
        xml->exitbranch();
    }
    else
    {
        xml->beginbranch(name);
        sectionType->add2XML(xml);
        xml->endbranch();
    }

    return name;
}


bool UnifiedPresets::saveUnif(CommandBlock *getData)
{
    synth->getRuntime().xmlType = TOPLEVEL::XML::Presets;
    XMLwrapper *xml = new XMLwrapper(synth, false);
    string type = findXML(xml, getData, false);
    if (type.empty())
        synth->getRuntime().Log("Unrecognised preset type");
    else
    {
        string dirname;
        string name = synth->textMsgBuffer.fetch(getData->data.miscmsg);
        if (name.empty())
        {
            dirname = file::localDir() + "/clipboard";
            if (file::createDir(dirname))
                synth->getRuntime().Log("Failed to open clipboard directory");
            else
                xml->saveXMLfile(dirname + "/section." + type + EXTEN::presets);
        }
        else
        {
            dirname = synth->getRuntime().presetsDirlist[synth->getRuntime().presetsRootID];
            xml->saveXMLfile(dirname + "/" + name + "." + type + EXTEN::presets);
        }
    }
    delete xml;
    return false;
}


bool UnifiedPresets::load(CommandBlock *getData)
{
    synth->getRuntime().xmlType = TOPLEVEL::XML::Presets;
    string type = findPresetType(getData);
    XMLwrapper *xml = new XMLwrapper(synth, false);
    string name = synth->textMsgBuffer.fetch(getData->data.miscmsg);
    string dirname;
    string prefix;

    if (name.empty())
    {
        dirname = file::localDir() + "/clipboard";
        if (file::createDir(dirname))
        {
            synth->getRuntime().Log("Failed to open clipboard directory");
        }
        else
        {
            prefix = dirname + "/section.";
        }
    }
    else
    {
        dirname = synth->getRuntime().presetsDirlist[synth->getRuntime().presetsRootID];
        prefix = dirname + "/" + name + ".";
    }
    string filename = prefix + type + EXTEN::presets;

    if (file::isRegularFile(prefix + type + EXTEN::presets) == 0)
    {
        synth->getRuntime().Log("Can't match " + filename + " here.");
        return false;
    }

    xml->loadXMLfile(filename);
    findXML(xml, getData, true);
    delete xml;
    return false;
}

bool UnifiedPresets::remove(CommandBlock *getData)
{
    string type = findPresetType(getData);
    string name = synth->textMsgBuffer.fetch(getData->data.miscmsg);
    string dirname = synth->getRuntime().presetsDirlist[synth->getRuntime().presetsRootID];
    string filename = dirname + "/" + name + "." + type + EXTEN::presets;
    file::deleteFile(filename);
    return false;
}
