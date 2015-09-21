/*
File: dataStructures.cpp
Purpose: To contain the definitions of student-created class functions and struct functions, so as not overcrowd the HALshell.h file.
*/
#include "dataStructures.h"

//
// Functions for the Shell 'Command Line History Queue' class
//
HistQueue::HistQueue()
{
	
}

HistQueue::HistQueue (int queueSize)
{
	QUEUE_SIZE = queueSize;
	queue = new string [QUEUE_SIZE];
	length = 0;
	front = 0;
	back = front;
}

HistQueue::~HistQueue()
{
	
}

int HistQueue::Length ()
{
	return length;
}

bool HistQueue::IsEmpty ()
{
	if (length == 0)
	{
		return true;
	}
		return false;
}

bool HistQueue::IsFull ()
{
	if (length == QUEUE_SIZE)
	{
		return true;
	}
		return false;
}

void HistQueue::Enqueue (string recentCmd)
{
	//Branch to deal with the case of a full queue, allowing overwriting of the oldest entry without increasing the length.
	// length > 0 ensures this doesn't happen at the start of the program
	if(length > 0 && back == front)
	{
		front = (front + 1) % QUEUE_SIZE;
		queue [back] = recentCmd;
		back = (back + 1) % QUEUE_SIZE;
	}
	//Branch that deals with entering items into a queue that is not full.
	else
	{
		queue [back] = recentCmd;
		back = (back + 1) % QUEUE_SIZE;
		length++;
	}
	
	return;
}

string HistQueue::Dequeue ()
{
	front = (front + 1) % QUEUE_SIZE;
	length --;
	return (queue [front]);
}

void HistQueue::PrintHistory()
{
	//Ensures that there is history to print
	if(length != 0)
	{
		int current = front;
		//If the queue is not full, then we only need to iterate until we hit the back, with no modular looping required.
		if(length != QUEUE_SIZE)
		{
			while(current != back)
			{
				if(queue[current] != "")
				{
					cout << queue[current] << '\n';
				}
				else
				{
					current = back;
				}
				
				if(current % (QUEUE_SIZE-1) == 0 && current != 0)
				{
					current = 0;
				}
				else
				{
					current ++;
				}
			}
		}
		//If the queue is full, we need to carefully do a modular progression through the queue.
		else
		{
			do
			{
				cout << queue[current] << '\n';
				if(current % (QUEUE_SIZE-1) == 0  && current != 0) //Modular loop back check.
				{
					current = 0;
				}
				else
				{
					current ++;
				}
			}while(current != front);
		}
	}
	//Case for no history.
	else
	{
		cout << "There was no history to display. Your history now contains 'showhistory'.";
	}
	return;
}

string HistQueue::retrieveCmd()
{
	//Assures that we don't try to return a command located at position [-1]
    if(back == 0)
    {
		return queue[(back+(QUEUE_SIZE-1))];
    }
    else
	{
		return queue[(back-1)];
	}
}

string HistQueue::retrieveCmd(int recCmd)
{
	//Cannot retrieve the 0th command (does not make sense), and cannot retrive a command that does not exist (out of bounds).
    if(recCmd > 0 && recCmd < QUEUE_SIZE)
    {
		//Assures that we don't try to return a command located at position [-1]
		if(back == 0 || recCmd > back)
       	{
			return queue[(back+(QUEUE_SIZE-recCmd))];
        }
       	else
        {
           	    return queue[(back-recCmd)];
	    }
	}
	else
	{
		string error = "Invalid history request!";
		return error;
	}
}

void HistQueue::changeSize(int newSize)
{
	//Checks to see if we are increasing or decreasing the queue size.
	if(newSize > QUEUE_SIZE)
	{
		//This branch creates a new queue of the new size and then copies the 'old' queue into the new queue.
		
		string* oldArray = queue;
		int oldSize = QUEUE_SIZE;
		QUEUE_SIZE = newSize;
		queue = new string [QUEUE_SIZE];
		
		//The copy loop.
		for(int i = 0; i < oldSize; i++)
		{
			queue[i] = oldArray[i];
		}
		delete [] oldArray;
		oldArray = NULL;
	}
	else
	{
		//This branch does effectively the same as the first branch, but ensures that the user actually wants to reduce the history size.
		char ans;
		cout << "Are you sure you would like to reduce history size? Some history may be lost. (You must enter 'y' or 'n')" << endl << "Reduce size? ";
		
		//Ensures valid input.
		while(ans != 'y' && ans != 'n')
		{
			cin >> ans;
			ans = tolower(ans);
			cout << "A: " << ans << endl;
		}
		
		//Aborts size change if user enters 'n'
		if(ans == 'n')
		{
			cout << "Canceling size change..." << endl;
			return;
		}
		//Resizes the queue.
		else
		{
			string* oldArray = queue;
			int oldSize = QUEUE_SIZE;
			QUEUE_SIZE = newSize;
			queue = new string [QUEUE_SIZE];
		
			for(int i = 0; i < QUEUE_SIZE; i++)
			{
				queue[i] = oldArray[i];
			}
			delete [] oldArray;
			oldArray = NULL;
		}
	}
	return;
}

//
// Functions for the Shell 'Alias List' class
//
Array::Array()
{

}

Array::Array(unsigned int newSize)
{
    size = 0;
    MAX_SIZE = newSize;
    data = new string[MAX_SIZE];
}

Array::~Array()
{

}

void Array::SetSize(unsigned int i)
{
	//Creates a new list wiht the specified new size, into which the current list is copied into to, 
    string* tempData = new string[i];
    unsigned int newMaxSize = (i < MAX_SIZE) ? i : MAX_SIZE;
	
	//Data copy loop.
    for (int i = 0; i < newMaxSize; i++)
    {
        tempData[i] = data[i];
    }
    data = tempData;
    return;
}

string& Array::operator [](unsigned int index)
{
    return data[index];
}

bool Array::IsFull()
{
    return (size == MAX_SIZE);
}

unsigned int Array::Find(const string& str, const int length)
{
	//Iterates through list to see if the alias exists.
    for (int i = 0; i < size; i++)
    {
	if (data[i].substr(0, length) == str)
        {
	    return i;
	}
    }
	//Returns if the specified alias is not found.
    return 99999;
}

unsigned int Array::Length()
{
    return size;
}

void Array::Add(string item)
{
    if (!IsFull())
    {
        data[size] = item;
        size++;
    }
    return;
}

void Array::Clear()
{
    size = 0;
    return;
}

void Array::Delete(unsigned int index)
{
	//Ensures that the index selected is within bounds
    if (index >= 0 && index < size)
    {
        for (int i = index; i < size-1; i++)
        {
            data[i] = data[i+1];
        }
    }
    return;
}
