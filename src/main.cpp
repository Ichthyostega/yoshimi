/*
    main.cpp

    Copyright 2009-2011, Alan Calvert
    Copyright 2014-2024, Will Godfrey & others
    Copyright 2024, Ichthyostega

    This file is part of yoshimi, which is free software: you can
    redistribute it and/or modify it under the terms of the GNU General
    Public License as published by the Free Software Foundation, either
    version 2 of the License, or (at your option) any later version.

    yoshimi is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with yoshimi.  If not, see <http://www.gnu.org/licenses/>.

*/

/* ***** THIS IS A CRIPLED VERSION OF YOSHIMI and starts only an empty GUI shell ***** */


#include <iostream>
#include <memory>

#include "Interface/InterfaceAnchor.h"
#include "MasterUI.h"

using std::cout;
using std::endl;





/******************************//**
 * Run the Yoshimi Application
 */
int main()
{
    cout << "\nHello World - this is a gutted Yoshimi..." << endl;

    auto guiMaster = std::make_unique<MasterUI>(InterfaceAnchor(/*uninitialised for this test*/));

    guiMaster->Init();
    cout << "\n\n... FLTK UI created" << endl;

    uint loopCnt{0};

    while (guiMaster and guiMaster->runGUI)
    {
        guiMaster->checkBuffer();
        Fl::wait(33333); // process GUI events

        if (++loopCnt < 10)
        {
            cout << "... FLTK event loop "<<loopCnt << endl;
        }
        else
        if (loopCnt == 10)
        {
            cout << "... FLTK event loop running *WITHOUT* crash !!!!!\n" << endl;
        }
    }

    cout << "\nStill no crash yet ("<<loopCnt<<" turns) -- farewell cruel World!" << endl;
    exit(0);
}
