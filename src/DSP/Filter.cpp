/*
    Filter.cpp - Filters, uses analog,formant,etc. filters

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2009 Nasca Octavian Paul
    Copyright 2009-2011, Alan Calvert

    This file is part of yoshimi, which is free software: you can redistribute
    it and/or modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    yoshimi is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.   See the GNU General Public License (version 2 or
    later) for more details.

    You should have received a copy of the GNU General Public License along with
    yoshimi; if not, write to the Free Software Foundation, Inc., 51 Franklin
    Street, Fifth Floor, Boston, MA  02110-1301, USA.

    This file is derivative of ZynAddSubFX original code, modified January 2011
*/

#include "DSP/Filter.h"

Filter::Filter(FilterParams *pars)
{
    unsigned char Ftype = pars->Ptype;
    unsigned char Fstages = pars->Pstages;

    category = pars->Pcategory;

    switch (category)
    {
        case 1:
            filter = new FormantFilter(pars);
            break;
        case 2:
            filter = new SVFilter(Ftype, 1000.0f, pars->getq(), Fstages);
            filter->outgain = dB2rap(pars->getgain());
            if (filter->outgain > 1.0f)
                filter->outgain = sqrtf(filter->outgain);
            break;
        default:
            filter = new AnalogFilter(Ftype, 1000.0f, pars->getq(), Fstages);
            if (Ftype >= 6 && Ftype <= 8)
                filter->setgain(pars->getgain());
            else
                filter->outgain = dB2rap(pars->getgain());
            break;
    }
}

Filter::~Filter()
{
    delete filter;
}

void Filter::filterout(float *smp)
{
    filter->filterout(smp);
}

void Filter::setfreq(float frequency)
{
    filter->setfreq(frequency);
}

void Filter::setfreq_and_q(float frequency, float q_)
{
    filter->setfreq_and_q(frequency, q_);
}

void Filter::setq(float q_)
{
    filter->setq(q_);
}

float Filter::getrealfreq(float freqpitch)
{
    if (category == 0 || category == 2)
        return powf(2.0f, freqpitch + 9.96578428f); // log2(1000)=9.95748
    else
        return freqpitch;
}
