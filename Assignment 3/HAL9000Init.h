//
// HAL9000Init.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#define HAL_9000_INIT_H

#include <iostream>
#include <fstream>

using namespace std;

const int MAX_BIOS_VARIABLES = 10;

int PAGE_SIZE;
int NO_OF_PAGES;
int QUANTUM_LENGTH;
string HALos;

memoryCell biosVariables [MAX_BIOS_VARIABLES];

int GetHALbiosVariables ()
{
    ifstream biosFile;
    int i;
    string variableDescription;
    string variableValue;

    cout << "HALstart: HAL9000 OK" << endl;
    usleep (SLEEP_DELAY);

    cout << "HAL9000: initializing ..." << endl;
    usleep (SLEEP_DELAY);

    cout << "HAL9000: boot sequence started ..." << endl;
    usleep (SLEEP_DELAY);

    cout << "HAL9000: loading HALbios ..." << endl;

    for (i = 0; i < MAX_BIOS_VARIABLES; i ++)
    {
        biosVariables [i].symbol = "";
        biosVariables [i].value = "";
    }

    biosFile.open ("HALbiosVariables");
    if (!biosFile)
    {
        cout << "HAL9000: unable to read bios variables file" << endl;
        exit (1);
    }

    biosFile >> variableDescription;
    biosFile.ignore (256, ':');
    getline (biosFile, variableValue);
    PAGE_SIZE = atoi (variableValue.c_str ());
    biosVariables [0].symbol = variableDescription;
    biosVariables [0].value = variableValue;

    biosFile >> variableDescription; 
    biosFile.ignore (256, ':');
    getline (biosFile, variableValue);
    NO_OF_PAGES = atoi (variableValue.c_str ());
    biosVariables [1].symbol = variableDescription;
    biosVariables [1].value = variableValue;

    biosFile >> variableDescription;
    biosFile.ignore (256, ':');
    getline (biosFile, variableValue);
    QUANTUM_LENGTH = atoi (variableValue.c_str ());
    biosVariables [2].symbol = variableDescription;
    biosVariables [2].value = variableValue;

    biosFile >> variableDescription;
    biosFile.ignore (256, ':');
    getline (biosFile, variableValue);
    HALos = variableValue;
    biosVariables [3].symbol = variableDescription;
    biosVariables [3].value = variableValue;

    biosFile.close ();
    usleep (SLEEP_DELAY);

    cout << "HAL9000: HALbios OK" << endl;
    usleep (SLEEP_DELAY);

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

Dummy A (GetHALbiosVariables ());
