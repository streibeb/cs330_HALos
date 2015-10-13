//
// HALshell.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#define HAL_SHELL_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>

using namespace std;

#ifndef HAL_GLOBALS_H
    #include "HALglobals.h"
#endif

pid_t HALosPid;
string returnPid = "";
string returnValue = "";
string returnMessage = "";

void Initialize ();
void HALshell ();
string GetCommandLine ();
void ProcessCommand (string command);
bool SendCommandLine (string commandLine);
void Wait ();
void GetMessageFromHALos ();
static volatile sig_atomic_t commandHandled = 0;
static volatile sig_atomic_t cullProcess = 0;
static void SignalHandler (int signalNo, siginfo_t* info, void* context);
sigset_t interruptMask;
struct sigaction act;

inline void BlockSignals ()
{
    if (sigprocmask (SIG_BLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HALshell: unable to block signals" << endl;
        exit (1);
    }
}

inline void UnblockSignals ()
{
    if (sigprocmask (SIG_UNBLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HALshell: unable to unblock signals" << endl;
        exit (1);
    }
}
