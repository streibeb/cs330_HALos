//
// HALkeyboardDriver.cpp
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#include "HALkeyboardDriver.h"

int main ()
{
    Initialize ();
    HALkeyboardDriver ();

    return 0;
}

void HALkeyboardDriver ()
{
    do
    {
        BlockSignals ();
        if (messageFromHALos)
        {
            messageFromHALos = 0;
            GetMessageFromHALos ();
            ProcessIORequest ();
            if (result == "KEYBOARD_READ_OK")
            {
                SendMessageToHALos ();
            }
        }
        UnblockSignals ();
    } while (1);

    return;
}

void GetMessageFromHALos ()
{
    ifstream ioRequestFile;

    ioRequestFile.open ("HALosToHALkeyboardDriver");
    if (!ioRequestFile)
    {
        cout << "HALkeyboardDriver: unable to initialize io request buffer" << endl;
        return;
    }

    getline (ioRequestFile, pid);
    getline (ioRequestFile, systemCall);

    if (!ioRequestFile)
    {
        cout << "HALkeyboardDriver: message not received from HALos" << endl;
        return;
    }

    ioRequestFile.close ();

    return;
}

void ProcessIORequest ()
{
    cullProcess = 0;

    if (systemCall == "READ")
    {
        getline (cin, buffer);
        if (cullProcess == 1)
        {
            cin.clear ();
            result = "PROCESS_CULLED";
        }
        else
        {
            result = "KEYBOARD_READ_OK";
        }
    }
    else if (systemCall == "SHUTDOWN")
    {
        exit (0);
    }

    return;
}

void SendMessageToHALos ()
{
    ofstream ioResponseFile;
    union sigval dummyValue;

    ioResponseFile.open ("HALkeyboardDriverToHALos");
    if (!ioResponseFile)
    {
        cout << "HALkeyboardDriver: unable to initialize io response buffer" << endl;
        return;
    }

    ioResponseFile << "INTERRUPT" << endl;
    ioResponseFile << "KEYBOARD_DRIVER" << endl;
    ioResponseFile << pid << endl;
    ioResponseFile << buffer << endl;
    ioResponseFile << result << endl;

    ioResponseFile.close ();

    if (sigqueue (HALosPid, SIGRTMIN, dummyValue) == -1)
    {
        cout << "HALkeyboardDriver: io response signal not sent to HALos" << endl;
        exit (1);
    }

    return;
}

void Initialize ()
{
    if ((sigemptyset (&interruptMask) == -1) ||
        (sigaddset (&interruptMask, SIGRTMIN) == -1))
    {
        cout << "HALkeyboardDriver: unable to initialize signal mask" << endl;
        exit (1);
    }
    act.sa_sigaction = &SignalHandler;
    act.sa_mask = interruptMask;
    act.sa_flags = SA_SIGINFO;
    if ((sigemptyset (&act.sa_mask) == -1) ||
        (sigaction (SIGRTMIN, &act, NULL) == -1) ||
        (sigaction (SIGINT, &act, NULL) == -1))
    {
        cout << "HALkeyboardDriver: unable to connect to HALos" << endl;
        exit (1);
    }

    HALosPid = getppid ();

    return;
}

static void SignalHandler (int signalNo, siginfo_t* info, void* context)
{
    if (signalNo == SIGRTMIN)
    {
        if (info -> si_pid == HALosPid)
        {
            messageFromHALos = 1;
        }
    }
    else if (signalNo == SIGINT)
    {
        cullProcess = 1;
    }
}
