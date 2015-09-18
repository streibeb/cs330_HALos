//
// HALshell.cpp
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#include "HALshell.h"

int main ()
{
    Initialize ();
    HALshell ();

    return 0;
}

void HALshell ()
{
    int commandCount = 0;
    string commandLine;

    do
    {
        commandLine = GetCommandLine (++commandCount);
        ProcessCommand (commandLine);
    } while (1);

    return;
}

string GetCommandLine (int commandCount)
{
    string commandLine;

    do
    {
        BlockSignals ();
        cout << config.shellName << "[" << commandCount << "]" 
<< config.terminator << " ";
        getline (cin, commandLine);
        if (cullProcess)
        {
            cullProcess = 0;
            cin.clear ();
            cout << endl;
        }
        UnblockSignals ();
    } while (commandLine.length () == 0);

    return commandLine;
}

void ProcessCommand (string commandLine)
{
    bool commandSent;

    queue.Enqueue(commandLine); 

    if (commandLine == "result")
    {
        if (returnPid != "")
        {
            cout << "PID = " << returnPid << endl;
            cout << "RETURN_VALUE = " << returnValue << endl;
            cout << "MESSAGE = " << returnMessage << endl;
            returnPid = "";
            returnValue = "";
            returnMessage = "";
        }
        return;
    }
    else if (commandLine == "shutdown" ||
             commandLine == "restart")
    {
        cout << endl;
        cout << config.shellName << ": terminating ..." << endl;
        usleep (SLEEP_DELAY);
        SendCommandLine (commandLine);
        exit (0);
    }
    else if (commandLine.substr(0, 12) == "setshellname")
    {
        config.shellName = commandLine.substr(13);
        config.Save();
        return;
    } 
    else if (commandLine.substr(0, 13) == "setterminator")
    {
        config.terminator = commandLine.substr(14);
        config.Save();
        return;
    }
    else if (commandLine.substr(0, 14) == "sethistorysize")
    {
        config.historySize = atoi(commandLine.substr(15).c_str());
        config.Save();
        return;
    }
    else if (commandLine.substr(0,15) == "showhistorysize")
    {
        cout << config.historySize << endl;
        return;
    }
    else if (commandLine.substr(0, 11) == "showhistory")
    {
      	queue.PrintHistory();
        return;
    }

    
    commandSent = SendCommandLine (commandLine);
    if (commandSent)
    {
        Wait ();
        GetMessageFromHALos ();
        if (returnMessage.length () > 0 && returnMessage != "ok")
        {
            cout << config.shellName << ": " << returnMessage << endl;
        }
    }

    return;
}

bool SendCommandLine (string commandLine)
{
    ofstream commandLineFile;
    int noOfArguments = -1;
    int i;
    union sigval dummyValue;

    commandLineFile.open ("HALshellToHALos");
    if (!commandLineFile)
    {
        cout << config.shellName << ": unable to initialize command line buffer" 
<< endl;
        exit (1);
    }
    else
    {
        for (i = 0; i < commandLine.length (); i ++)
        {
            if (commandLine [i] == ' ')
            {
                commandLineFile << endl;
                noOfArguments ++;
            }
            else
            {
                commandLineFile << commandLine [i];
            }
        }
        noOfArguments ++;
        commandLineFile.close ();
        if (noOfArguments > MAX_COMMAND_LINE_ARGUMENTS)
        {
            cout << config.shellName << ": maximum number of command line arguments is " 
<< MAX_COMMAND_LINE_ARGUMENTS << endl;
            return false;
        }
        if (sigqueue (HALosPid, SIGRTMIN, dummyValue) == -1)
        {
            cout << config.shellName << ": command line signal not sent to HALos" 
<< endl;
            exit (1);
        }
    }

    return true;
}

void Wait ()
{
    do
    {
        BlockSignals ();
        if (cullProcess)
        {
            cullProcess = 0;
            SendCommandLine ("cull");
            cin.clear ();
            cout << endl;
        }
        else if (commandHandled)
        {
            commandHandled = 0;
            break;
        }
        UnblockSignals ();
    } while (1);

    return;
}

void GetMessageFromHALos ()
{
    ifstream halOsMessageFile;

    halOsMessageFile.open ("HALosToHALshell");
    if (!halOsMessageFile)
    {
        cout << config.shellName << ": connection to HALos failed" << 
endl;
        exit (1);
    }

    getline (halOsMessageFile, returnPid);
    getline (halOsMessageFile, returnValue);
    getline (halOsMessageFile, returnMessage);

    if (!halOsMessageFile)
    {
        cout << config.shellName << ": message not received from HALos" << 
endl;
        return;
    }

    halOsMessageFile.close ();

    return;
}

void Initialize ()
{
    config.Load();
    queue = HistQueue(config.historySize);

    cout << "HALos: " << config.shellName << " OK" << endl;
    usleep (SLEEP_DELAY);

    cout << config.shellName << ": initializing ..." << endl;
    if ((sigemptyset (&interruptMask) == -1) ||
        (sigaddset (&interruptMask, SIGRTMIN) == -1))
    {
        cout << config.shellName << ": unable to initialize signal mask" 
<< endl;
        exit (1);
    }
    act.sa_sigaction = &SignalHandler;
    act.sa_mask = interruptMask;
    act.sa_flags = SA_SIGINFO;
    if ((sigemptyset (&act.sa_mask) == -1) ||
        (sigaction (SIGRTMIN, &act, NULL) == -1) ||
        (sigaction (SIGINT, &act, NULL) == -1))
    {
        cout << config.shellName << ": unable to connect to HALos" << 
endl;
        exit (1);
    }
    usleep (SLEEP_DELAY);

    cout << endl;

    HALosPid = getppid ();

    return;
}

static void SignalHandler (int signalNo, siginfo_t* info, void* context)
{
    if (signalNo == SIGRTMIN)
    {
        if (info -> si_pid == HALosPid)
        {
            commandHandled = 1;
        }
    }
    else if (signalNo == SIGINT)
    {
        cullProcess = 1;
    }
}
