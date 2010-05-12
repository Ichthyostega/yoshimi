/*
    SUBnoteParameters.h - Parameters for SUBnote (SUBsynth)

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2005 Nasca Octavian Paul
    Copyright 2009, Alan Calvert

    This file is part of yoshimi, which is free software: you can redistribute
    it and/or modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    yoshimi is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.   See the GNU General Public License (version 2 or
    later) for more details.

    You should have received a copy of the GNU General Public License along with
    yoshimi; if not, write to the Free Software Foundation, Inc., 51 Franklin
    Street, Fifth Floor, Boston, MA  02110-1301, USA.

    This file is a derivative of the ZynAddSubFX original, modified October 2009
*/

#ifndef SUB_NOTE_PARAMETERS_H
#define SUB_NOTE_PARAMETERS_H

#include "Misc/XMLwrapper.h"
#include "Params/EnvelopeParams.h"
#include "Params/FilterParams.h"
#include "Params/Presets.h"

class SUBnoteParameters : public Presets
{
    public:
        SUBnoteParameters();
        ~SUBnoteParameters();

        void add2XML(XMLwrapper *xml);
        void defaults(void);
        void getfromXML(XMLwrapper *xml);

        // Amplitude Parametrers
        bool Pstereo; // streo = true, mono = false
        unsigned char PVolume;
        unsigned char PPanning;
        unsigned char PAmpVelocityScaleFunction;
        EnvelopeParams *AmpEnvelope;

        // Frequency Parameters
        unsigned short int PDetune;
        unsigned short int PCoarseDetune;
        unsigned char PDetuneType;
        unsigned char PFreqEnvelopeEnabled;
        EnvelopeParams *FreqEnvelope;
        unsigned char PBandWidthEnvelopeEnabled;
        EnvelopeParams *BandWidthEnvelope;

        // Filter Parameters (Global)
        unsigned char PGlobalFilterEnabled;
        FilterParams *GlobalFilter;
        unsigned char PGlobalFilterVelocityScale;
        unsigned char PGlobalFilterVelocityScaleFunction;
        EnvelopeParams *GlobalFilterEnvelope;

        // Other Parameters

        // If the base frequency is fixed to 440 Hz
        unsigned char Pfixedfreq;

        /* Equal temperate (this is used only if the Pfixedfreq is enabled)
           If this parameter is 0, the frequency is fixed (to 440 Hz);
           if this parameter is 64, 1 MIDI halftone -> 1 frequency halftone */
        unsigned char PfixedfreqET;

        // how many times the filters are applied
        unsigned char Pnumstages;

        // bandwidth
        unsigned char Pbandwidth;

        // How the magnitudes are computed (0=linear,1=-60dB,2=-60dB)
        unsigned char Phmagtype;

        // Magnitudes
        unsigned char Phmag[MAX_SUB_HARMONICS];

        // Relative BandWidth ("64"=1.0)
        unsigned char Phrelbw[MAX_SUB_HARMONICS];

        // how much the bandwidth is increased according to lower/higher frequency; 64-default
        unsigned char Pbwscale;

        // how the harmonics start("0"=0,"1"=random,"2"=1)
        unsigned char Pstart;
};

#endif



