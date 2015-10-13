//
// HALpageTable.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#define HAL_PAGE_TABLE_H

#include <string>

using namespace std;

#ifndef HAL_GLOBALS_H
    #include "HALglobals.h"
#endif

struct pageDescriptor
{
    string pid;
    int age;
};

class PageTableType
{
public:
    PageTableType (int PageTableSize);
    ~PageTableType ();
    void ResetP ();
    void Iterate ();
    int GetP ();
    void SetP (int q);
    int Size ();
    pageDescriptor Read ();
    void SetAge (int toFrame, int fromFrame);
    void IncrementAges ();
    void Insert (string pid);
    bool Find (string pid);
    string FindOldest ();
    void Delete ();
    void Clear ();
private:
    pageDescriptor* pageTable;
    int p;
    int PAGE_TABLE_SIZE;
};
