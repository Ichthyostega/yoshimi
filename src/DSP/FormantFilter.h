/*
    FormantFilter.h - formant filter

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2005 Nasca Octavian Paul

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

#ifndef FORMANT_FILTER_H
#define FORMANT_FILTER_H

#include "DSP/Filter_.h"
#include "DSP/AnalogFilter.h"
#include "Params/FilterParams.h"


class FormantFilter : public Filter_
{
    public:
        FormantFilter(FilterParams *pars);
        ~FormantFilter();
        void filterout(float *smp);
        void setfreq(float frequency);
        void setfreq_and_q(float frequency, float q_);
        void setq(float q_);
        void cleanup(void);

    private:
        void setpos(float input);


        AnalogFilter *formant[FF_MAX_FORMANTS];
        float *inbuffer, *tmpbuf;

        struct {
            float freq, amp, q; // frequency,amplitude,Q
        } formantpar[FF_MAX_VOWELS][FF_MAX_FORMANTS],
          currentformants[FF_MAX_FORMANTS];

        struct {
            unsigned char nvowel;
        } sequence [FF_MAX_SEQUENCE];

        float oldformantamp[FF_MAX_FORMANTS];

        int sequencesize, numformants, firsttime;
        float oldinput, slowinput;
        float Qfactor, formantslowness, oldQfactor;
        float vowelclearness, sequencestretch;
};


#endif

