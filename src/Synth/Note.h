/*
    Note.h - Interface and shared functionality of all Note entities

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2005 Nasca Octavian Paul
    Copyright 2009-2011, Alan Calvert
    Copyright 2014-2019, Will Godfrey
    Copyright 2019 Ichthyostega

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

    Extracted as common base class from original ZynAddSubFX code in 2019-10
*/

#ifndef NOTE_H
#define NOTE_H

#include "Synth/LegatoTypes.h"

class SynthEngine;
class Controller;
class Envelope;
class Filter;
class LFO;


class Note
{
    protected:
        bool ready_;
        bool active_;
        SynthEngine *synth;
        Controller *ctl;
        
        struct Legato {
            bool silent;
            float lastfreq;
            LegatoMsg msg;
            int decounter;
            struct {
                // Fade In/Out vars
                int length;
                float m;
                float step;
            } fade;

            struct {
                // Note parameters
                float freq;
                float vel;
                int portamento;
                int midinote;
            } param;
        };
        
        Legato legato;
        
        Note(SynthEngine *_synth, Controller *_ctl) :
            ready_(false),
            active_(true),
            synth(_synth),
            ctl(_ctl)
        { }
        

    public:
        virtual ~Note() { } // This is an interface

        // note output; return 0 when note is finished
        virtual int noteout(float *outl,float *outr)  =0;
        virtual void releasekey(void)                 =0;

        virtual void setupLegatonote(float freq, float velocity, int portamento_, int midinote, bool externcall) =0;
        
        bool isReady()  { return ready_; };  // Initialisation complete
        bool isActive() { return active_; }; // Note produces sound


    private:
};

#endif /*NOTE_H*/
