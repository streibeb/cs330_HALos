//
// HALshell.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#define HAL_SHELL_H

#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include "dataStructures.h"

using namespace std;

#ifndef HAL_GLOBALS_H
    #include "HALglobals.h"
#endif

struct Config
{
    string shellName;
    string terminator;
    int historySize;
    unsigned int newNameSize;

    Config()
    {
        shellName = "HALshell";
	terminator = ">";
	historySize = 10;
	newNameSize = 10;
    }

    void Load()
    {
        ifstream fin;
        fin.open("config");
        if (fin)
        {
            fin >> shellName;
            fin >> terminator;
            fin >> historySize;
	    fin >> newNameSize;
            fin.close();
        }
    }

    void Save()
    {
        ofstream fout;
        fout.open("config");
        if (fout)
        {
            fout << shellName << endl;
            fout << terminator << endl;
            fout << historySize << endl;
	    fout << newNameSize << endl;
            fout.close();
        }
    }
};

pid_t HALosPid;
string returnPid = "";
string returnValue = "";
string returnMessage = "";
Config config;
HistQueue queue;
Array aliasList;

void Initialize ();
void HALshell ();
string GetCommandLine (int commandCount);
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
