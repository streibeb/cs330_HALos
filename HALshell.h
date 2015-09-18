//
// HALshell.h
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#define HAL_SHELL_H

#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>

using namespace std;

#ifndef HAL_GLOBALS_H
    #include "HALglobals.h"
#endif

struct Config
{
    string shellName;
    string terminator;
    int historySize;

    Config()
    {
        shellName = "HALshell";
		terminator = ">";
		historySize = 10;
    }

    void Load()
    {
        ifstream fin;
        fin.open("config");
        if (fin)
        {
            fin >> shellName;
            fin >> terminator;
            fin >> historySize;
            fin.close();
        }
    }

    void Save()
    {
        ofstream fout;
        fout.open("config");
        if (fout)
        {
            fout << shellName << endl;
            fout << terminator << endl;
            fout << historySize << endl;
            fout.close();
        }
    }
};

class HistQueue
{
private:
    string* queue;
    int front;
    int back;
    int length;
    int QUEUE_SIZE;
public:
	HistQueue ()
	{
		
	}

	HistQueue (int queueSize)
	{
		QUEUE_SIZE = queueSize;
		queue = new string [QUEUE_SIZE];

		length = 0;
		front = 0;
		back = front;
	}

	~HistQueue ()
	{
	}

	int Length ()
	{
		return length;
	}

	bool IsEmpty ()
	{
		if (length == 0)
		{
			return true;
		}

		return false;
	}

	bool IsFull ()
	{
		if (length == QUEUE_SIZE)
		{
			return true;
		}

		return false;
	}

	void Enqueue (string recentCmd)
	{
		if(length == 0)
		{
			queue[front] = recentCmd;
			back = (back + 1) % QUEUE_SIZE;
			length++;
		}
		else if(length > 0 && back == front)
		{
			front = (front + 1) % QUEUE_SIZE;
			queue [back] = recentCmd;
			back = (back + 1) % QUEUE_SIZE;
		}
		else
		{
			queue [back] = recentCmd;
			back = (back + 1) % QUEUE_SIZE;
			length++;
		}
		
		return;
	}

	string Dequeue ()
	{
		front = (front + 1) % QUEUE_SIZE;

		length --;

		return (queue [front]);
	}

	void PrintHistory()
	{
		if(length != 0)
		{
			int current = front;
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
			else
			{
				do
				{
					cout << queue[current] << '\n';
					if(current % (QUEUE_SIZE-1) == 0  && current != 0)
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
		else
		{
			cout << "There was no history to display. Your history now contains 'showhistory'.";
		}
	return;
	}
};

pid_t HALosPid;
string returnPid = "";
string returnValue = "";
string returnMessage = "";
Config config;
HistQueue queue;

void Initialize ();
void HALshell ();
string GetCommandLine (int commandCount);
void ProcessCommand (string command);
bool SendCommandLine (string commandLine);
void Wait ();
void GetMessageFromHALos ();
static volatile sig_atomic_t commandHandled = 0;
static volatile sig_atomic_t cullProcess = 0;
static void SignalHandler (int signalNo, siginfo_t* info, void* context);
sigset_t interruptMask;
struct sigaction act;

inline void BlockSignals ()
{
    if (sigprocmask (SIG_BLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HALshell: unable to block signals" << endl;
        exit (1);
    }
}

inline void UnblockSignals ()
{
    if (sigprocmask (SIG_UNBLOCK, &interruptMask, NULL) == -1)
    {
        cout << "HALshell: unable to unblock signals" << endl;
        exit (1);
    }
}
