//
// HAL9000.h
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

#ifndef HAL_GLOBALS_H
    #include "HALglobals.h"
#endif

#ifndef HAL_MEMORY_H
    #include "HALmemory.h"
#endif

#ifndef HAL_9000_INIT_H
    #include "HAL9000Init.h"
#endif

MemoryType ram (MEMORY_SIZE);

extern char **environ;
pid_t HALstartPid;
pid_t HALosPid;
int returnPid;
string returnValue = "";

void Initialize ();
void InitializeMemory ();
void GetMemorySegmentParameters ();
void SetMemorySegmentBoundaries ();
void SetKernelVariables ();
void SetTableAndStackStartAddresses ();
int GetMemorySegmentBoundary (string segmentStartAddressDescription, int& segmentSize);

void HAL9000 ();
string GetMessageFromHALos (int& pid, string& command, string arguments [], string& buffer, string& result);
bool ExecuteCommand (int pid, string command, string arguments []);
void StartMain (string arguments []);
bool ExecuteOneInstruction (string address, string buffer, string result);
void FetchInstruction (memoryCell& contents);
bool DecodeInstruction (memoryCell contents);
bool ExecuteInstruction (int currentProgramTextAddress, memoryCell contents,
                         string restartInstructionStatusFlag, string buffer, string result);
void EndMain ();

// "Built-in" Instructions

void Comment ();
void Call (int currentProgramTextAddress, memoryCell contents);
void Return (memoryCell contents);
void Put (memoryCell contents);
void Set (memoryCell contents);
void Compare (string dataType);
void CompareEndOfFile ();
void CompareDataTypes (string dataType);
void CompareValues ();
void Unput ();
void Jump (memoryCell contents);
void DoTheMath (string operation);

// System Calls

void Open (string symbol, string restartInstructionStatusFlag, string result);
void Read (string symbol, string restartInstructionStatusFlag, string buffer, string result);
void Write (string symbol, string restartInstructionStatusFlag, string result);
void Newline (string symbol, string restartInstructionStatusFlag, string result);
void Close (string symbol, string restartInstructionStatusFlag);

int GetLocalSymbolAddress (string value);
int GetGlobalSymbolAddress (string value);

memoryCell DetermineMemoryCellContentsForReturnAndPutCommands (memoryCell contents1);

void DeleteProcessImage (int programTextStartAddress);

void ProcessImageToMemory (int pid);
void ProcessImageToFile (int pid, string location);
bool AddressField (string symbol, int i, int globalSymbolsTableStartAddress, int globalSymbolsTableEndAddress,
                   int functionCallStackStartAddress, int functionCallStackEndAddress);
void CoreSnapShot ();
void CoreDump ();

void AllocateLocalSymbol (memoryCell contents);
void AssignTypeToLocalSymbol (memoryCell contents);
void AllocateLocalArray (memoryCell contents);

int GetKernelVariableIntegerValue (string kernelVariableDescription);
string GetKernelVariableStringValue (string kernelVariableDescription);
void SetKernelVariableValue (string kernelVariableDescription, string value);

void Shutdown ();
void Restart ();
void SendMessageToHALos (string messageType, string parameter1, string parameter2, string parameter3,
                         string parameter4, string parameter5);
string GetDataType (string symbol);
bool IsInteger (string value);
bool IsFloat (string value);
string itos (int i);
string dtos (double d);

static volatile sig_atomic_t somethingToExecute = 0;
static void SignalHandler (int signalNo, siginfo_t* info, void* context);
sigset_t interruptMask;
struct sigaction act;

inline void BlockSignals ()
{
    if (sigprocmask (SIG_BLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HAL9000: unable to block signals" << endl;
        exit (1);
    }
}

inline void UnblockSignals ()
{
    if (sigprocmask (SIG_UNBLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HAL9000: unable to unblock signals" << endl;
        exit (1);
    }
}
