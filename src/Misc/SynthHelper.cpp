/*
    SynthHelper.cpp

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2005 Nasca Octavian Paul
    Copyright 2009-2010 Alan Calvert

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

    This file is derivative of ZynAddSubFX original code, modified December 2010
*/

#include "Misc/SynthHelper.h"

// Get the detune in cents
float SynthHelper::getDetune(unsigned char type, unsigned short int coarsedetune,
                             unsigned short int finedetune)
{
    float det = 0.0f;
    float octdet = 0.0f;
    float cdet = 0.0f;
    float findet = 0.0f;

    // Get Octave
    int octave = coarsedetune / 1024;
    if (octave >= 8)
        octave -= 16;
    octdet = octave * 1200.0f;

    // Coarse and fine detune
    int cdetune = coarsedetune % 1024;
    if (cdetune > 512)
        cdetune -= 1024;

    int fdetune = finedetune - 8192;

    switch (type)
    {
    //	case 1: is used for the default (see below)
        case 2:
            cdet = fabsf(cdetune * 10.0f);
            findet = fabsf(fdetune / 8192.0f) * 10.0f;
            break;
        case 3:
            cdet = fabsf(cdetune * 100.0f);
            findet = powf(10.0f, fabsf(fdetune / 8192.0f) * 3.0f) / 10.0f - 0.1f;
            break;
        case 4:
            cdet = fabsf(cdetune * 701.95500087f); // perfect fifth
            findet = (powf(2.0f, fabsf(fdetune / 8192.0f) * 12.0f) - 1.0f) / 4095.0f * 1200.0f;
            break;
            // case ...: need to update N_DETUNE_TYPES, if you'll add more
        default:
            cdet = fabsf(cdetune * 50.0f);
            findet = fabsf(fdetune / 8192.0f) * 35.0f; // almost like "Paul's Sound Designer 2"
            break;
    }
    if (finedetune < 8192u)
        findet = -findet;
    if (cdetune < 0)
        cdet = -cdet;

    det = octdet + cdet + findet;
    return det;
}
