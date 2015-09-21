/*
File: dataStructures.h
Purpose: To contain the declarations of student-created classes and structs, so as not overcrowd the HALshell.h file.
*/
#ifndef __DATA_STRUCTURES_H__
#define __DATA_STRUCTURES_H__

#include <cstddef>
#include <iostream>
#include <string>

using namespace std;
/*
This class is for storing the history of commands entered into the command line, 
and allows for that history to be reused and printed to the screen, among other commands.
*/
class HistQueue
{
private:
    string* queue;
    int front;
    int back;
    int length;
    int QUEUE_SIZE;
public:
	HistQueue (); //Default Constructor

	HistQueue (int queueSize); //Constructor that creates a queue of the appropriate size.

	~HistQueue (); //Default constructor.

	int Length (); //Returns queue length.

	bool IsEmpty (); //Returns whether or not queue is empty.

	bool IsFull (); //Returns whether or not queue is full.

	void Enqueue (string recentCmd); //Adds an item to the back of the queue.

	string Dequeue (); //Removes an item from the queue.

	void PrintHistory(); //Prints the history of commands entered, from least recent to most recent.
	
	string retrieveCmd(); //Retrieves the most recent command.

	string retrieveCmd(int recCmd); //Retrieves the specified command.
	
	void changeSize(int newSize); //Resizes the queue according the provided new size.
};

/*
This class stores alias', with member functions for adding new alias', clearing the list, and deleting a specific alias.
*/
class Array
{
private:
    string* data;
    unsigned int size;
    unsigned int MAX_SIZE;

public:
    Array(); //Default constructor
    Array(unsigned int newSize); //Constructor that creates a list of the appropriate size.
    ~Array(); //Default constructor.
    void SetSize(unsigned int i); //changes the size of the list
    
    string& operator [] (unsigned int index); //Overloaded the [] operator in order to retrieve the encapsulated data.
    bool IsFull(); //Returns whether or not list is full.
    unsigned int Find(const string& str, const int); //Function to return the position of a requested alias, or 99999 if not found.
    unsigned int Length(); //Returns list length.
    void Add(string item); //Adds an alias to the list
    void Clear(); //Clears the list of all entries by allowing previous entries to be overwritten (ie. setting length/size to 0).
    void Delete(unsigned int index); //Deletes the alias located at the specified index
};

#endif
