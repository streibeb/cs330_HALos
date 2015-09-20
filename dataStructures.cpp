
#include "dataStructures.h"

//
// Functions for the Shell Command History Queue class
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

string HistQueue::Dequeue ()
{
	front = (front + 1) % QUEUE_SIZE;
	length --;
	return (queue [front]);
}

void HistQueue::PrintHistory()
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

string HistQueue::retrieveCmd()
{
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
    if(recCmd > 0 && recCmd < QUEUE_SIZE)
    {
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
	if(newSize > QUEUE_SIZE)
	{
		string* oldArray = queue;
		int oldSize = QUEUE_SIZE;
		QUEUE_SIZE = newSize;
		queue = new string [QUEUE_SIZE];
	
		for(int i = 0; i < oldSize; i++)
		{
			queue[i] = oldArray[i];
		}
		delete oldArray;
		oldArray = NULL;
	}
	else
	{
		char ans;
		cout << "Are you sure you would like to reduce history size? Some history may be lost. (You must enter 'y' or 'n')" << endl << "Reduce size? ";
		while(ans != 'y' || ans != 'n')
		{
			cin >> ans;
			ans = tolower(ans);
			cout << "A: " << ans << endl;
		}
		if(ans == 'n')
		{
			cout << "Canceling size change..." << endl;
			return;
		}
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
			delete oldArray;
			oldArray = NULL;
		}
	}
	return;
}



