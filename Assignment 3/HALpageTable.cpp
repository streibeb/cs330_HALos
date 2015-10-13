//
// HALpageTable.cpp
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#include "HALpageTable.h"

PageTableType::PageTableType (int pageTableSize)
{
    PAGE_TABLE_SIZE = pageTableSize;
    pageTable = new pageDescriptor [PAGE_TABLE_SIZE];

    for (p = 2; p < PAGE_TABLE_SIZE; p ++)
    {
        pageTable [p].pid = "";
        pageTable [p].age = 0;
    }

    p = -1;
}

PageTableType::~PageTableType ()
{
}

void PageTableType::ResetP ()
{
    p = -1;

    return;
}

void PageTableType::Iterate ()
{
    p ++;

    return;
}

int PageTableType::GetP ()
{
    return p;
}

void PageTableType::SetP (int q)
{
    p = q;

    return;
}

int PageTableType::Size ()
{
    return PAGE_TABLE_SIZE;
}

pageDescriptor PageTableType::Read ()
{
    return (pageTable [p]);
}

void PageTableType::SetAge (int toFrame, int fromFrame)
{
    pageTable [toFrame].age = pageTable [fromFrame].age;

    return;
}

void PageTableType::IncrementAges ()
{
    pageTable [0].age ++;
    for (p = 2; p < PAGE_TABLE_SIZE; p ++)
    {
        if (pageTable [p].pid.length () > 0)
        {
            pageTable [p].age ++;
        }
    }

    return;
}

void PageTableType::Insert (string pid)
{
    pageTable [p].pid = pid;
    pageTable [p].age = 0;

    return;
}

bool PageTableType::Find (string pid)
{
    p = 2;
    while (p < PAGE_TABLE_SIZE)
    {
        if (pid == pageTable [p].pid)
        {
            return true;
        }
        p ++;
    }

    return false;
}

string PageTableType::FindOldest ()
{
    int age = -1;
    int q;

    p = 2;
    while (p < PAGE_TABLE_SIZE)
    {
        if (pageTable [p].age > age)
        {
            age = pageTable [p].age;
            q = p;
        }
        p ++;
    }
    p = q;

    return (pageTable [p].pid);
}

void PageTableType::Delete ()
{
    pageTable [p].pid = "";
    pageTable [p].age = 0; 

    return;
}

void PageTableType::Clear ()
{
    for (p = 2; p < PAGE_TABLE_SIZE; p ++)
    {
        pageTable [p].pid = "";
        pageTable [p].age = 0;
    }

    return;
}

