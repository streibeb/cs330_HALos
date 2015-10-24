//
// HALmemory.cpp
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#include "HALmemory.h"

MemoryType::MemoryType (int pageSize, int noOfPages)
{
    PAGE_SIZE = pageSize;
    NO_OF_PAGES = noOfPages;
    memory = new memoryCell [PAGE_SIZE * NO_OF_PAGES];

    for (p = 0; p < PAGE_SIZE * NO_OF_PAGES; p ++)
    {
        memory [p].symbol = "";
        memory [p].value = "";
    }
}

MemoryType::~MemoryType ()
{
}

void MemoryType::ResetP ()
{
    p = -1;

    return;
}

void MemoryType::IterateUp ()
{
    p ++;

    return;
}

void MemoryType::IterateDown ()
{
    p --;

    return;
}

bool MemoryType::IsPSet ()
{
    if (p < 0 || p > PAGE_SIZE - 1)
    {
        return false;
    }

    return true;
}

int MemoryType::GetP ()
{
    return p;
}

void MemoryType::SetP (int q)
{
    p = q;

    return;
}

void MemoryType::Move (int toFrame, int fromFrame)
{
    for (p = 0; p < PAGE_SIZE; p ++)
    {
        memory [p + (toFrame * PAGE_SIZE)] = memory [p + (fromFrame * PAGE_SIZE)];
    }

    return;
}

memoryCell MemoryType::Read ()
{
    memoryCell contents;

    contents.symbol = memory [p].symbol;
    contents.value = memory [p].value;

    return contents;
}

void MemoryType::Write (string symbol, string value)
{
    memory [p].symbol = symbol;
    memory [p].value = value;

    return;
}

void MemoryType::ReWrite (string symbol, string value)
{
    if (symbol.length () > 0)
    {
        memory [p].symbol = symbol;
    }
    memory [p].value = value;

    return;
}

void MemoryType::Push (string symbol, string value)
{
    p --;
    memory [p].symbol = symbol;
    memory [p].value = value;

    return;
}

memoryCell MemoryType::Pop ()
{
    memoryCell contents;

    contents.symbol = memory [p].symbol;
    contents.value = memory [p].value;
    
    memory [p].symbol = "";
    memory [p].value = "";

    p ++;

    return contents;
}

bool MemoryType::Find (string symbol, int startAddress, int endAddress)
{
    if (endAddress >= startAddress)
    {
        for (p = startAddress; p <= endAddress; p ++)
        {
            if (symbol == memory [p].symbol)
            {
                return true;
            }
        }
    }
    else
    {
        for (p = startAddress; p >= endAddress; p --)
        {
            if (symbol == memory [p].symbol)
            {
                return true;
            }
        }
    }
 
    return false;
}

void MemoryType::Clear (int startAddress)
{
    for (p = startAddress; p < PAGE_SIZE; p ++)
    {
        memory [p].symbol = "";
        memory [p].value = "";
    }

    return;
}
