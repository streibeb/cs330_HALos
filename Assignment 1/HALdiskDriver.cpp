//
// HALdiskDriver.cpp
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#include "HALdiskDriver.h"

int main ()
{
    Initialize ();
    HALdiskDriver ();

    return 0;
}

void HALdiskDriver ()
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

    ioRequestFile.open ("HALosToHALdiskDriver");
    if (!ioRequestFile)
    {
        cout << "HALdiskDriver: unable to open io request buffer" << endl;
        return;
    }

    getline (ioRequestFile, pid);
    getline (ioRequestFile, systemCall);
    getline (ioRequestFile, fileName);
    getline (ioRequestFile, mode);
    getline (ioRequestFile, markerPosition);
    getline (ioRequestFile, buffer);

    if (!ioRequestFile)
    {
        cout << "HALdiskDriver: message from HALos corrupted" << endl;
        return;
    }

    ioRequestFile.close ();

    return;
}

void ProcessIORequest ()
{
    fstream inOutFile;
    struct stat statusBuffer;
    size_t eofMarker;

    if (systemCall == "OPEN")
    {
        if (mode == "input")
        {
            // check for existence of file
            if (stat (fileName.c_str (), &statusBuffer) != -1)
            {
                result = "FILE_OPEN_OK";
            }
            else
            {
                result = "FILE_OPEN_FAILED";
            }
        }
        else // (mode == "output")
        {
            // create empty file
            inOutFile.open (fileName.c_str (), fstream::trunc | fstream::out);
            if (inOutFile)
            {
                result = "FILE_OPEN_OK";
            }
            else
            {
                result = "FILE_OPEN_FAILED";
            }
            inOutFile.close ();
        }
    }
    else if (systemCall == "READ")
    {
        inOutFile.open (fileName.c_str (), fstream::ate | fstream::in);
        if (inOutFile)
        {
            eofMarker = inOutFile.tellg ();
            if (atoi (markerPosition.c_str ()) != eofMarker)
            {
                inOutFile.seekg (atoi (markerPosition.c_str ()));
                getline (inOutFile, buffer);
                if (inOutFile)
                {
                    markerPosition = itos (inOutFile.tellg ());
                    result = "FILE_READ_OK";
                }
                else
                {
                    buffer = "";
                    markerPosition = "";
                    result = "FILE_READ_FAILED";
                }
            }
            else
            {
                buffer = "";
                markerPosition = "EOF";
                result = "FILE_AT_END";
            }
        }
        else
        {
            buffer = "";
            markerPosition = "";
            result = "FILE_READ_FAILED";
        }
        inOutFile.close ();
    }
    else if (systemCall == "WRITE")
    {
        inOutFile.open (fileName.c_str (), fstream::app | fstream ::out);
        if (inOutFile)
        {
            inOutFile << buffer << flush;
            markerPosition = "";
            result = "FILE_WRITE_OK";
        }
        else
        {
            markerPosition = "";
            result = "FILE_WRITE_FAILED";
        }
        inOutFile.close ();
    }
    else if (systemCall == "NEWLINE")
    {
        inOutFile.open (fileName.c_str (), fstream::app | fstream ::out);
        if (inOutFile)
        {
            inOutFile << endl;
            markerPosition = "";
            result = "FILE_NEWLINE_OK";
        }
        else
        {
            markerPosition = "";
            result = "FILE_NEWLINE_FAILED";
        }
        inOutFile.close ();
    }
    else if (systemCall == "SHUTDOWN")
    {
        exit (0);
    }

    return;
}

void SendMessageToHALos ()
{
    static int seqNo = 0;
    static string fileNamePrefix = "HALdiskDriverToHALos_";
    string fileName;
    ofstream ioResponseFile;
    union sigval dummyValue;

    seqNo ++;
    fileName = fileNamePrefix + itos (seqNo);
    ioResponseFile.open (fileName.c_str ());
    if (!ioResponseFile)
    {
        cout << "HALdiskDriver: unable to open io response buffer" << endl;
        return;
    }

    ioResponseFile << "INTERRUPT" << endl;
    ioResponseFile << "DISK_DRIVER" << endl;
    ioResponseFile << pid << endl;
    ioResponseFile << markerPosition << endl;
    ioResponseFile << buffer << endl;
    ioResponseFile << result << endl;

    ioResponseFile.close ();

    if (sigqueue (HALosPid, SIGRTMIN, dummyValue) == -1)
    {
        cout << "HALdiskDriver: error sending io response signal to HALos" << endl;
    }

    if (seqNo == INT_MAX)
    {
        seqNo = 0;
    }

    return;
}

string itos (int i)
{
    stringstream s;

    s << i;

    return s.str ();
}

void Initialize ()
{
    if ((sigemptyset (&interruptMask) == -1) ||
        (sigaddset (&interruptMask, SIGRTMIN) == -1))
    {
        cout << "HALdiskDriver: unable to initialize signal mask" << endl;
        exit (1);
    }
    act.sa_sigaction = &SignalHandler;
    act.sa_mask = interruptMask;
    act.sa_flags = SA_SIGINFO;
    if ((sigemptyset (&act.sa_mask) == -1) ||
        (sigaction (SIGRTMIN, &act, NULL) == -1))
    {
        cout << "HALdiskDriver: unable to communicate with HALos" << endl;
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
