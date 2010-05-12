/*
    Presets.h - Presets and Clipboard management

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

#ifndef PRESETS_H
#define PRESETS_H

#include "Misc/XMLwrapper.h"
#include "Params/PresetsStore.h"

class Presets
{
    public:
        Presets();
        virtual ~Presets() { };

        void copy(const char *name); // <if name==NULL, the clipboard is used
        void paste(int npreset);     // npreset==0 for clipboard
        bool checkclipboardtype();
        void deletepreset(int npreset);

        char type[MAX_PRESETTYPE_SIZE];
        void setelement(int n);
        void rescanforpresets(void);
        unsigned int getSamplerate(void) { return samplerate; };
        int getBuffersize(void) { return buffersize; };
        int getOscilsize(void) { return oscilsize; };

    protected:
        void setpresettype(const char *type);
        unsigned int samplerate;
        int buffersize;
        int oscilsize;
        int half_oscilsize;

    private:
        virtual void add2XML(XMLwrapper *xml) = 0;
        virtual void getfromXML(XMLwrapper *xml) = 0;
        virtual void defaults(void) = 0;
        virtual void add2XMLsection(XMLwrapper *xml, int n) { };
        virtual void getfromXMLsection(XMLwrapper *xml, int n) { };
        virtual void defaults(int n) { };
        int nelement;
};

#endif
