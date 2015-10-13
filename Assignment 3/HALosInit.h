//
// HALosInit.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#define HAL_OS_INIT_H

#include <iostream>
#include <fstream>

using namespace std;

int READY_QUEUE_SIZE;
int FILE_TABLE_SIZE;
int IO_QUEUE_SIZE;
int MEMORY_SIZE;

int GetHALosVariables ()
{
    ifstream osFile;
    string variableDescription;
    string variableValue;

    osFile.open ("HALosVariables");
    if (!osFile)
    {
        cout << "HALos: unable to read os variables file" << endl;
        exit (1);
    }

    osFile >> variableDescription;
    osFile.ignore (256, ':');
    getline (osFile, variableValue);
    READY_QUEUE_SIZE = atoi (variableValue.c_str ());

    osFile >> variableDescription;
    osFile.ignore (256, ':');
    getline (osFile, variableValue);
    FILE_TABLE_SIZE = atoi (variableValue.c_str ());

    osFile >> variableDescription;
    osFile.ignore (256, ':');
    getline (osFile, variableValue);
    IO_QUEUE_SIZE = atoi (variableValue.c_str ());

    osFile.close ();

    return 0;
}

int GetHALbiosMemorySizeVariable ()
{
    ifstream biosFile;
    string uselessString;
    string memorySize;

    biosFile.open ("HALbiosVariables");
    if (!biosFile)
    {
        cout << "HALos: unable to determine memory size" << endl;
        exit (1);
    }

    biosFile >> uselessString;
    biosFile.ignore (256, ':');
    getline (biosFile, memorySize);
    MEMORY_SIZE = atoi (memorySize.c_str ());

    return 0;
}

class Dummy
{
public:
    Dummy (int dummy);
private:
    int DUMMY;
};

Dummy::Dummy (int dummy)
{
    DUMMY = dummy;
}

Dummy A (GetHALosVariables ());
Dummy B (GetHALbiosMemorySizeVariable ());
