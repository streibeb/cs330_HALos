#ifndef __DATA_STRUCTURES_H__
#define __DATA_STRUCTURES_H__

#include <cstddef>
#include <iostream>
#include <string>

using namespace std;
//
// A class for storing the history of commands entered on the command line, integrated into HALshell.
//
class HistQueue
{
private:
    string* queue;
    int front;
    int back;
    int length;
    int QUEUE_SIZE;
public:
	HistQueue ();

	HistQueue (int queueSize);

	~HistQueue ();

	int Length ();

	bool IsEmpty ();

	bool IsFull ();

	void Enqueue (string recentCmd);

	string Dequeue ();

	void PrintHistory();
	
	string retrieveCmd();

	string retrieveCmd(int recCmd);
	
	void changeSize(int newSize);
};


class Array
{
private:
    string* data;
    unsigned int size;
    unsigned int MAX_SIZE;

public:
    Array();
    Array(unsigned int newSize);
    ~Array();
    void SetSize(unsigned int i);
    
    string& operator [] (unsigned int index);
    bool IsFull();
    unsigned int Length();
    void Add(string item);
    void Clear();
    void Delete(unsigned int index);
};

#endif
