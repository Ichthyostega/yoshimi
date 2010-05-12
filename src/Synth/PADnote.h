/*
    PADnote.h - The "pad" synthesizer

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

    This file is a derivative of the ZynAddSubFX original, modified January 2010
*/

#ifndef PAD_NOTE_H
#define PAD_NOTE_H

#include "Params/PADnoteParameters.h"
#include "Params/Controller.h"
#include "Synth/Envelope.h"
#include "Synth/LFO.h"
#include "DSP/Filter.h"
#include "Params/Controller.h"

class PADnote
{
    public:
        PADnote(PADnoteParameters *parameters, Controller *ctl_, float freq,
                float velocity, int portamento_, int midinote, bool besilent);
        ~PADnote();

        void PADlegatonote(float freq, float velocity,
                           int portamento_, int midinote, bool externcall);

        int noteout(float *outl,float *outr);
        bool finished(void) { return finished_; };
        void relasekey(void);

        bool ready;

    private:
        void fadein(float *smps);
        void computecurrentparameters();
        bool finished_;
        PADnoteParameters *pars;

        int poshi_l;
        int poshi_r;
        float poslo;

        float basefreq;
        bool firsttime;
        bool released;

        int nsample, portamento;

        int Compute_Linear(float *outl, float *outr, int freqhi,
                           float freqlo);
        int Compute_Cubic(float *outl, float *outr, int freqhi,
                          float freqlo);


        struct {
            //****************************
            // FREQUENCY GLOBAL PARAMETERS
            //****************************
            float Detune;//cents

            Envelope *FreqEnvelope;
            LFO *FreqLfo;

            //****************************
            // AMPLITUDE GLOBAL PARAMETERS
            //****************************
            float Volume; // [ 0 .. 1 ]

            float Panning; // [ 0 .. 1 ]

            Envelope *AmpEnvelope;
            LFO *AmpLfo;

            struct {
                int Enabled;
                float initialvalue;
                float dt;
                float t;
            } Punch;

            //*************************
            // FILTER GLOBAL PARAMETERS
            //*************************
            Filter *GlobalFilterL;
            Filter *GlobalFilterR;

            float FilterCenterPitch;//octaves
            float FilterQ;
            float FilterFreqTracking;

            Envelope *FilterEnvelope;

            LFO *FilterLfo;
        } NoteGlobalPar;


        float globaloldamplitude;
        float globalnewamplitude;
        float velocity;
        float realfreq;
        float *tmpwave;
        Controller *ctl;

        // Legato vars
        struct {
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
        } Legato;

        unsigned int samplerate;
        int buffersize;
        int oscilsize;
};

#endif
