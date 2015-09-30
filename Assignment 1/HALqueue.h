//
// HALqueue.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#define HAL_QUEUE_H

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
    FileTableType fileTable;
    int systemCallFileIndex;
    string systemCall;
    string systemCallParameter1;
    string systemCallParameter2;
    string systemCallParameter3;
    string systemCallBuffer;
    string systemCallResult;
};

class QueueType
{
public:
    QueueType (int queueSize);
    ~QueueType ();
    int Length ();
    bool IsEmpty ();
    bool IsFull ();
    void Enqueue (processDescriptor process);
    processDescriptor Dequeue ();
private:
    processDescriptor* queue;
    int front;
    int back;
    int length;
    int QUEUE_SIZE;
};
