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
    int size;
    int MAX_SIZE;

public:
    Array();
    Array(int newSize);
    ~Array();
    void SetSize(int i);
    
    string& operator [] (int index);
    bool IsFull();
    int Length();
    void Add(string item);
    void Clear();
    void Delete(int index);
};

#endif
