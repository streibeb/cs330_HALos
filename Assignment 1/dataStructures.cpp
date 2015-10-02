
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
	for(int i = 0; i < QUEUE_SIZE; i++)
	{
		if(queue[i] != "")
		{}
		else
		{
			queue [i] = "";
		}
	}
 	return;
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
    if(recCmd > 0 && recCmd <= QUEUE_SIZE)
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
	if(newSize == QUEUE_SIZE)
	{
		return;
	}
	
	//Common declarations.
	int oldSize = QUEUE_SIZE;
	string* oldArray = queue;
	queue = new string [newSize];
	if(IsFull()) //Working with full history
	{
		if(newSize > oldSize) //The history size is getting bigger.
		{
			/*
				If back and front are at the start of the history, we know that the most recent command is at the end of old queue.
				So we move the back to the end of the new queue, just after the most recent command, and copy the old queue to the new queue.
			*/
			if(back == 0)
			{
				back = oldSize;
				QUEUE_SIZE = newSize;
				
				for(int i = 0; i < oldSize; i++)
				{
					queue[i] = oldArray[i];
				}
			}
			/*
				If the back and the front are not at 0, then we know that we have not completed a queue rotation, and as such, need to move the back forward
				by rotating the old queue forward (by use of a temp variable) to go from a [2,1,0,3] order to a [3,2,1,0] order. Once the back reaches the old size,
				we know it has reached the end of the rotating. Front must then be set to the very front, position 0, and we copy the old (reordered)
				queue into the new queue.
			*/
			else
			{
				while(back != oldSize)
				{
					string temp = oldArray[oldSize-1];
					for(int i = oldSize-1; i > 0; i--)
					{
						oldArray[i] = oldArray[i-1];
					}
					oldArray[0] = temp;
					back ++;
				}
				
				front = 0;
				QUEUE_SIZE = newSize;
				
				for(int i = 0; i < oldSize; i++)
				{
					queue[i] = oldArray[i];
				}
			}
		}
		else if(newSize < oldSize) //The history size is getting smaller.
		{
			/*
				If the back is currently beyond the new size, then we need to rotate the old queue backwards, thereby losing the oldest history. We move the
				back index along with it. Once the back index = 0 (or if it started out that way), we move the back and the front indices to 0, as the most
				recent history is now at the back of the queue, meaning that 0 is the position in 'front' of it. We then copy the old queue into the new queue.
				Note: As the queue is full, and has been reduced, length becomes the new size.
			*/
			if(back >= newSize)
			{
				cout << "Problem 1." << endl;
				length = newSize;
				QUEUE_SIZE = newSize;
				while(back != newSize)
				{
					for(int i = 1; i < oldSize - 1; i++)
					{
						oldArray[i-1] = oldArray[i];
					}
					back --;
				}
				back = 0;
				front = 0;
				for(int i = 0; i < newSize; i++)
				{
					queue[i] = oldArray[i];
				}
			}
			/*
				If the back index is currently less than the new size, then (since the queue is full), we need to rotate the queue forward,
				so that the most recent command is at the end of the queue. This is done using a temp variable. Once done, back and front
				go to 0, and we copy the old queue into the new queue.
			*/
			else
			{
				while(back != newSize)
				{
					string temp = oldArray[oldSize-1];
					for(int i = oldSize-1; i > 0; i--)
					{
						oldArray[i] = oldArray[i-1];
					}
					oldArray[0] = temp;
					back ++;
				}
				length = newSize;
				back = 0;
				front = 0;
				QUEUE_SIZE = newSize;
				for(int i = 0; i < newSize; i++)
				{
					queue[i] = oldArray[i];
				}
			}
		}
	}
	/* 
		The array is shrinking, and back is beyond the new size, while the array is not full. The same as the 'array is full' version of this case,
		separated only because nothing needs to happen (except the size change and copying over) when:
		- The array is getting smaller, but back is less than newSize.
		- The array is getting bigger. If the array isn't full, then we just add more empty space.
		As 'nothing' needs to happen in the above case, the only case for a 'not full' array is covered below.
	
	*/
	else if(newSize < oldSize && back >= newSize)
	{
		length = newSize;
		QUEUE_SIZE = newSize;
		while(back != newSize)
		{
			for(int i = 1; i < oldSize - 1; i++)
			{
				oldArray[i-1] = oldArray[i];
			}
			back --;
		}
		back = 0;
		front = 0;
		for(int i = 0; i < newSize; i++)
		{
			queue[i] = oldArray[i];
		}
	}
	else
	{
		QUEUE_SIZE = newSize;
		if(newSize < oldSize)
		{
			for(int i = 0; i < newSize; i++)
			{
				queue[i] = oldArray[i];
			}
		}
		else
		{
			for(int i = 0; i < oldSize; i++)
			{
				queue[i] = oldArray[i];
			}
		}
	}
	//Initialize any empty array spaces.
	for(int i = 0; i < QUEUE_SIZE; i++)
	{
		if(queue[i] != "")
		{
		}
		else
		{
			queue [i] = "";
		}
	}
 	return;
}

Array::Array()
{

}

Array::Array(unsigned int newSize)
{
    size = 0;
    MAX_SIZE = newSize;
    data = new string[MAX_SIZE];
    for (int i = 0; i < MAX_SIZE; i++)
        {
            data[i] = "z";
        }
}

Array::~Array()
{

}

void Array::SetSize(unsigned int i)
{
    string* tempData = new string[i];
    unsigned int newMaxSize = (i < MAX_SIZE) ? i : MAX_SIZE;
    for (int j = 0; j < newMaxSize; j++)
    {
        tempData[j] = data[j];
    }
	MAX_SIZE = i;
    delete [] data;
    data = tempData;
    for (int k = 0; k < MAX_SIZE; k++)
    {
		if(data[k] != "")
		{
		}
		else
		{
			data[k] = "z";
       	} 
	}
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
    for (int i = 0; i < size; i++)
    {
	if (data[i].substr(0, length) == str)
        {
	    return i;
	}
    }
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
    if (index >= 0 && index <= size)
    {
        for (int i = index; i < size; i++)
        {
            data[i] = data[i+1];
        }
	size --;
    }
    return;
}
