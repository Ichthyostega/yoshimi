/*
    CmdInterface.cpp

    Copyright 2015-2019, Will Godfrey & others.

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

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>
#include <cstdio>
#include <cerrno>
#include <cfloat>
#include <sys/types.h>
#include <ncurses.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <algorithm>
#include <iterator>
#include <map>
#include <list>
#include <sstream>
#include <thread>

#include "Misc/Bank.h"

#include "CLI/CmdInterface.h"
#include "CLI/Parser.h"
#include "Interface/TextLists.h"
#include "Misc/FormatFuncs.h"
#include "Misc/CliFuncs.h"

using std::this_thread::sleep_for;
using std::chrono_literals::operator ""ms;

using std::string;

using func::asString;
using cli::readControl;



CmdInterface::CmdInterface() :
    interpreter{}
{ }


Config& CmdInterface::getRuntime()
{
    return interpreter.synth->getRuntime();
}

void CmdInterface::Log(const string& msg, char toStderr)
{
    getRuntime().Log(msg, toStderr);
}


void CmdInterface::cmdIfaceCommandLoop()
{
    // Initialise the history functionality
    // Set up the history filename
    string hist_filename;

    { // put this in a block to lose the password afterwards
        struct passwd *pw = getpwuid(getuid());
        hist_filename = string(pw->pw_dir) + string("/.yoshimi_history");
    }
    cli::Parser parser;
    parser.setHistoryFile(hist_filename);
    interpreter.synth = & Config::instances().findSynthByID(0);
    bool exit{false};
    while (not exit)
    {
        parser.readline();
        if (parser.isTooLarge())
            std::cout << "*** Error: line too long" << std::endl;
        else if (parser.isValid())
        {
            // in case it's been changed from elsewhere
            interpreter.synth = & Config::instances().findSynthByID(interpreter.currentInstance);

            cli::Reply reply = interpreter.cmdIfaceProcessCommand(parser);
            exit = (reply.code == REPLY::exit_msg);

            if (reply.code == REPLY::what_msg)
                Log(reply.msg + replies[REPLY::what_msg]);
            else if (reply.code > REPLY::done_msg)
                Log(replies[reply.code]);
        }

        exit |= not getRuntime().runSynth.load(std::memory_order_acquire);
        if (not exit)
        {
            do
            { // create enough delay for most ops to complete
                sleep_for(2ms);
            }
            while (getRuntime().runSynth.load(std::memory_order_relaxed)
                   and not getRuntime().finishedCLI);
        }
        if (getRuntime().runSynth.load(std::memory_order_acquire))
        {
            string prompt = "yoshimi";
            if (interpreter.currentInstance > 0)
                prompt += (":" + asString(interpreter.currentInstance));
            int expose = readControl(interpreter.synth, 0, CONFIG::control::exposeStatus, TOPLEVEL::section::config);
            if (expose == 1)
            {
                string status = interpreter.buildStatus(true);
                if (status == "" )
                    status = " Top";
                Log("@" + status, _SYS_::LogError);
            }
            else if (expose == 2)
                prompt += interpreter.buildStatus(true);
            prompt += "> ";

            parser.setPrompt(prompt);
        }

        if (not exit and getRuntime().runSynth.load(std::memory_order_relaxed))
            sleep_for(20ms);
    }
}
