//
// HALos.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>

using namespace std;

#ifndef HAL_OS_INIT_H
    #include "HALosInit.h"
#endif

#ifndef HAL_READY_QUEUE_H
    #include "HALreadyQueue.h"
#endif

#ifndef HAL_QUEUE_H
    #include "HALqueue.h"
#endif

#ifndef HAL_MEMORY_H
    #include "HALmemory.h"
#endif

#ifndef HAL_GLOBALS_H
    #include "HALglobals.h"
#endif

ReadyQueueType readyQueue (READY_QUEUE_SIZE, NO_OF_READY_QUEUES, cpuSchedulingPolicies);
QueueType displayQueue (IO_QUEUE_SIZE);
QueueType keyboardQueue (IO_QUEUE_SIZE);
QueueType diskQueue (IO_QUEUE_SIZE);
MemoryType ram (MEMORY_SIZE);

extern char **environ;

pid_t HAL9000Pid;
pid_t HALshellPid;
pid_t HALkeyboardDriverPid;
pid_t HALdisplayDriverPid;
pid_t HALdiskDriverPid;

int nextPid = 1;

string messageType;
string parameter1;
string parameter2;
string parameter3;
string parameter4;
string parameter5;

processDescriptor cpuProcess;
processDescriptor keyboardProcess;
processDescriptor displayProcess;
processDescriptor diskProcess;
processDescriptor nullProcess;

void Initialize ();
void HALos ();
string GetMessageFromHALshell (string arguments [], string& type);
void HandleCommand (string command, string arguments [], string type);
void HandleFinishedProcess (bool okToScheduleNextProcess);
void HandleHAL9000Interrupt (bool okToScheduleNextProcess);
void HandleSystemCall (bool okToScheduleNextProcess);
void HandleHALkeyboardDriverInterrupt ();
void HandleHALdisplayDriverInterrupt ();
void HandleHALdiskDriverInterrupt ();

string CreateProcessImage (int pid, string command);
void GetProcessImageTemplate ();
void ProcessImageToFile (int pid, string location);
bool AddressField (string symbol, int i, int globalSymbolsTableStartAddress, int globalSymbolsTableEndAddress,
                   int functionCallStackStartAddress, int functionCallStackEndAddress);
void SetKernelVariableValue (string kernelVariableDescription, string value);
int GetMemorySegmentBoundary (string segmentStartAddressDescription, int& segmentSize);
void AllocateGlobalSymbol (memoryCell contents, int currentProgramTextAddress);
void AssignTypeToGlobalSymbol (memoryCell contents);
void AllocateGlobalArray (memoryCell contents);
int GetKernelVariableIntegerValue (string kernelVariableDescription);
int GetGlobalSymbolAddress (string value);
bool IsInteger (string value);

void ShutdownAndRestart (string command, string arguments []);
void Cull (string command, string arguments []);

void SendMessageToHAL9000 (processDescriptor process);
void SendReturnStatusToHALshell (string pid, string returnValue, string message);
void SendMessageToHALkeyboardDriver (string pid, string systemCall);
void SendMessageToHALdisplayDriver (string pid, string systemCall, string buffer);
void SendMessageToHALdiskDriver (string pid, string systemCall, string fileName, string mode,
                                 string markerPosition, string buffer);
void GetMessageFromHAL9000 ();
void GetMessageFromHALkeyboardDriver ();
void GetMessageFromHALdisplayDriver ();
void GetMessageFromHALdiskDriver ();

processDescriptor NullProcess ();

string itos (int i);

static volatile sig_atomic_t commandToExecuteFromHALshell = 0;
static volatile sig_atomic_t messageFromHAL9000 = 0;
static volatile sig_atomic_t messageFromHALkeyboardDriver = 0;
static volatile sig_atomic_t messageFromHALdisplayDriver = 0;
static volatile sig_atomic_t messageFromHALdiskDriver = 0;
static void SignalHandler (int signalNo, siginfo_t* info, void* context);
sigset_t interruptMask;
struct sigaction act;

inline void BlockSignals ()
{
    if (sigprocmask (SIG_BLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HALos: unable to block signals" << endl;
        exit (1);
    }
}

inline void UnblockSignals ()
{
    if (sigprocmask (SIG_UNBLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HALos: unable to unblock signals" << endl;
        exit (1);
    }
}
