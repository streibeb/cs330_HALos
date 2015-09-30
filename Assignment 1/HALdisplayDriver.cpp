//
// HALdisplayDriver.cpp
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#include "HALdisplayDriver.h"

int main ()
{
    Initialize ();
    HALdisplayDriver ();

    return 0;
}

void HALdisplayDriver ()
{
    do
    {
        BlockSignals ();
        if (messageFromHALos)
        {
            messageFromHALos = 0;
            GetMessageFromHALos ();
            ProcessIORequest ();
            SendMessageToHALos ();
        }
        UnblockSignals ();
    } while (1);

    return;
}

void GetMessageFromHALos ()
{
    ifstream ioRequestFile;

    ioRequestFile.open ("HALosToHALdisplayDriver");
    if (!ioRequestFile)
    {
        cout << "HALdisplayDriver: unable to initialize io request buffer" << endl;
        return;
    }

    getline (ioRequestFile, pid);
    getline (ioRequestFile, systemCall);
    getline (ioRequestFile, buffer);

    if (!ioRequestFile)
    {
        cout << "HALdisplayDriver: message not received from HALos" << endl;
        return;
    }

    ioRequestFile.close ();

    return;
}

void ProcessIORequest ()
{
    if (systemCall == "WRITE")
    {
        cout << buffer << flush;
        result = "DISPLAY_WRITE_OK";
    }
    else if (systemCall == "NEWLINE")
    {
        cout << endl;
        result = "DISPLAY_NEWLINE_OK";
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

    ioResponseFile.open ("HALdisplayDriverToHALos");
    if (!ioResponseFile)
    {
        cout << "HALdisplayDriver: unable to initialize io response buffer" << endl;
        return;
    }

    ioResponseFile << "INTERRUPT" << endl;
    ioResponseFile << "DISPLAY_DRIVER" << endl;
    ioResponseFile << pid << endl;
    ioResponseFile << result << endl;

    ioResponseFile.close ();

    if (sigqueue (HALosPid, SIGRTMIN, dummyValue) == -1)
    {
        cout << "HALdisplayDriver: io response signal not sent to HALos" << endl;
        exit (1);
    }

    return;
}

void Initialize ()
{
    if ((sigemptyset (&interruptMask) == -1) ||
        (sigaddset (&interruptMask, SIGRTMIN) == -1))
    {
        cout << "HALdisplayDriver: unable to initialize signal mask" << endl;
        exit (1);
    }
    act.sa_sigaction = &SignalHandler;
    act.sa_mask = interruptMask;
    act.sa_flags = SA_SIGINFO;
    if ((sigemptyset (&act.sa_mask) == -1) ||
        (sigaction (SIGRTMIN, &act, NULL) == -1))
    {
        cout << "HALdisplayDriver: unable to connect to HALos" << endl;
        exit (1);
    }
    signal (SIGINT, SIG_IGN);

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
}
