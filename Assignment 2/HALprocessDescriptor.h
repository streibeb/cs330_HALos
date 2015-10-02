//
// HALprocessDescriptor.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#define HAL_PROCESS_DESCRIPTOR_H

#include <string>

using namespace std;

#ifndef HAL_FILE_TABLE_H
    #include "HALfileTable.h"
#endif

#ifndef HAL_GLOBALS_H
    #include "HALglobals.h"
#endif

extern int FILE_TABLE_SIZE;

struct processDescriptor
{
    processDescriptor () : fileTable (FILE_TABLE_SIZE) {}
    string pid;
    string type;
    string status;
    string command;
    string arguments [MAX_COMMAND_LINE_ARGUMENTS];
    string returnValue;
    int queueNo;
    string direction;
    int contextSwitches;
    int priority;
    FileTableType fileTable;
    int systemCallFileIndex;
    string systemCall;
    string systemCallParameter1;
    string systemCallParameter2;
    string systemCallParameter3;
    string systemCallBuffer;
    string systemCallResult;
};
