//
// HALdiskDriver.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <limits.h>

using namespace std;

pid_t HALosPid;
string pid = "";
string systemCall = "";
string fileName = "";
string mode = "";
string markerPosition = "";
string buffer = "";
string result = "";

void Initialize ();
void HALdiskDriver ();
void GetMessageFromHALos ();
void ProcessIORequest ();
void SendMessageToHALos ();

string itos (int i);

static volatile sig_atomic_t messageFromHALos = 0;
static void SignalHandler (int signalNo, siginfo_t* info, void* context);
sigset_t interruptMask;
struct sigaction act;

inline void BlockSignals ()
{
    if (sigprocmask (SIG_BLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HALdiskDriver: unable to block signals" << endl;
        exit (1);
    }
}

inline void UnblockSignals ()
{
    if (sigprocmask (SIG_UNBLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HALdiskDriver: unable to unblock signals" << endl;
        exit (1);
    }
}
