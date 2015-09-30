//
// HALdisplayDriver.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#define HAL_DISPLAY_DRIVER_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <signal.h>

using namespace std;

pid_t HALosPid;
string pid = "";
string systemCall = "";
string buffer = "";
string result = "";

void Initialize ();
void HALdisplayDriver ();
void GetMessageFromHALos ();
void ProcessIORequest ();
void SendMessageToHALos ();

static volatile sig_atomic_t messageFromHALos = 0;
static void SignalHandler (int signalNo, siginfo_t* info, void* context);
sigset_t interruptMask;
struct sigaction act;

inline void BlockSignals ()
{
    if (sigprocmask (SIG_BLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HALdisplayDriver: unable to block signals" << endl;
        exit (1);
    }
}

inline void UnblockSignals ()
{
    if (sigprocmask (SIG_UNBLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HALdisplayDriver: unable to unblock signals" << endl;
        exit (1);
    }
}
