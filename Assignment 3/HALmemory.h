//
// HALmemory.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#define HAL_MEMORY_H

#include <string>

using namespace std;

#ifndef HAL_GLOBALS_H
    #include "HALglobals.h"
#endif

struct memoryCell
{
    string symbol;
    string value;
};

class MemoryType
{
public:
    MemoryType (int pageSize, int noOfPages);
    ~MemoryType ();
    void ResetP ();
    void IterateUp ();
    void IterateDown ();
    bool IsPSet ();
    int GetP ();
    void SetP (int q);
    void Move (int toFrame, int fromFrame);
    memoryCell Read ();
    void Write (string symbol, string value);
    void ReWrite (string symbol, string value);
    void Push (string symbol, string value);
    memoryCell Pop ();
    bool Find (string symbol, int startAddress, int endAddress);
    void Clear (int startAddress);
private:
    memoryCell* memory;
    int p;
    int PAGE_SIZE;
    int NO_OF_PAGES;
};
