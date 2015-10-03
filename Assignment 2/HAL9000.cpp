//
// HAL9000.cpp
//
// Copyright (c) 2015 Robert J. Hilderman.
// All rights reserved.
//

#include "HAL9000.h"

int main (int argc, char* argv [])
{
    Initialize ();
    HAL9000 ();

    return 0;
}

void HAL9000 ()
{
    string messageType;
    int pid;
    string command;
    string arguments [MAX_COMMAND_LINE_ARGUMENTS];
    string buffer;
    string result;
    int runningPid = -1;
    int segmentSize;
    bool userProcess;
    string address;
    bool userProcessDone;

    do
    {
        BlockSignals ();
        if (GetKernelVariableIntegerValue ("QUANTUM_TIME_REMAINING") == 0)
        {
            SetKernelVariableValue ("QUANTUM_TIME_REMAINING", itos (QUANTUM_LENGTH));
            ProcessImageToFile (runningPid, "backingstore");
            DeleteProcessImage (GetMemorySegmentBoundary ("PROGRAM_TEXT_START_ADDRESS", segmentSize));
            SendMessageToHALos ("INTERRUPT", "QUANTUM_EXPIRED", itos (runningPid), "", "", "");
        }

        if (somethingToExecute)
        {
            somethingToExecute = 0;
            messageType = GetMessageFromHALos (pid, command, arguments, buffer, result);
            if (messageType == "PAUSE_ANY_EXECUTING_PROCESS")
            {
                address = GetKernelVariableStringValue ("INSTRUCTION_POINTER");
                if (address != "null")
                {
                    ProcessImageToFile (runningPid, "backingstore");
                    DeleteProcessImage (GetMemorySegmentBoundary ("PROGRAM_TEXT_START_ADDRESS", segmentSize));
                    SendMessageToHALos ("EXECUTING_PROCESS_PAUSED", itos (pid), "", "", "", "");
                }
                else
                {
                    SendMessageToHALos ("NO_EXECUTING_PROCESS", itos (pid), "", "", "", "");
                }
                runningPid = -1;    
                continue;
            }
            else if (messageType == "CONTINUE_EXECUTING_PROCESS")
            {
                runningPid = pid;
                ProcessImageToMemory (runningPid);
            }
            else if (messageType == "NEW_PROCESS")
            {
                runningPid = pid;
                userProcess = ExecuteCommand (runningPid, command, arguments);
                if (!userProcess)
                {
                    SendMessageToHALos ("PROCESS_DONE", itos (runningPid), returnValue, "", "", "");
                    returnValue = "";
                }
                else
                {
                    ProcessImageToMemory (runningPid);
                    StartMain (arguments);
                }
            }
        }
        UnblockSignals ();

        address = GetKernelVariableStringValue ("INSTRUCTION_POINTER");
        if (address != "null")
        {
            userProcessDone = ExecuteOneInstruction (address, buffer, result);
            if (userProcessDone)
            {
                returnPid = GetKernelVariableIntegerValue ("PID");
                returnValue = GetKernelVariableStringValue ("RETURN_VALUE");
                returnOther = "CREATION_TIME: " + GetKernelVariableStringValue("CREATION_TIME") + " RUNNING_TIME: " + GetKernelVariableStringValue("RUNNING_TIME") + " END_TIME: " + GetKernelVariableStringValue("END_TIME");
                SendMessageToHALos ("PROCESS_DONE", itos (returnPid), returnValue, "", returnOther,"");	
                ProcessImageToFile (GetKernelVariableIntegerValue("PID"), "backingstore");
                DeleteProcessImage (GetMemorySegmentBoundary ("PROGRAM_TEXT_START_ADDRESS", segmentSize));
                returnPid = -1;
                returnValue = "";
            }
        }
    } while (1);

    return;
}

void SetClockTicks()
{
    int lock;
    struct flock key;

        ofstream clockFile;
        clockFile.open("HAL9000clock");
        if(!clockFile)
        {
                cout << "No clock ticks stored; unable to open HAL9000clock.";
                CoreDump ();
                exit (1);
        }
        HALclock += 1;

    key.l_type = F_WRLCK;
    key.l_whence = SEEK_SET;
    key.l_start = 0;
    key.l_len = 0;
    key.l_pid = getpid ();
    lock = open ("HAL9000clockLock", O_RDONLY);
    fcntl (lock, F_SETLKW, &key);

        clockFile << HALclock;
        clockFile.close();

    key.l_type = F_UNLCK;
    fcntl (lock, F_SETLK, &key);
    close (lock);

        return;
}

string GetMessageFromHALos (int& pid, string& command, string arguments [], string& buffer, string& result)
{
    ifstream halOsMessageFile;
    string messageType;
    int i;

    halOsMessageFile.open ("HALosToHAL9000");
    if (!halOsMessageFile)
    {
        cout << "HAL9000: connection to HALos failed" << endl;
        CoreDump ();
        exit (1);
    }

    getline (halOsMessageFile, messageType);
    halOsMessageFile >> pid;
    halOsMessageFile.ignore (256, '\n');
    getline (halOsMessageFile, command);
    for (i = 0; i < MAX_COMMAND_LINE_ARGUMENTS; i ++)
    {
        getline (halOsMessageFile, arguments [i]);
    }
    getline (halOsMessageFile, buffer);
    getline (halOsMessageFile, result);

    halOsMessageFile.close ();

    return messageType;
}

bool ExecuteCommand (int pid, string command, string arguments [])
{
    if (command == "shutdown")
    {
        Shutdown ();
    }
    else if (command == "restart")
    {
        Restart ();
    }

    return true;
}

void StartMain (string arguments [])
{
    int functionCallValuesStackStartAddress;
    int topFunctionCallValuesStackAddress;
    int segmentSize;
    memoryCell contents;
    int i;
    int programTextEndAddress;

    functionCallValuesStackStartAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_VALUES_STACK_START_ADDRESS", segmentSize);
    topFunctionCallValuesStackAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
    ram.SetP (topFunctionCallValuesStackAddress);
    if (ram.GetP () == functionCallValuesStackStartAddress - segmentSize)
    {
        cout << "HAL9000: function call values stack segmentation violation" << endl;
        CoreDump ();
        exit (1);
    }

    for (i = 0; i < MAX_COMMAND_LINE_ARGUMENTS; i ++)
    {
        if (arguments [i].length () == 0)
        {
            break;
        }
        contents.value = arguments [i];
        if (IsInteger (contents.value))
        {
            contents.symbol = "integer";
        }
        else if (IsFloat (contents.value))
        {
            contents.symbol = "float";
        }
        else 
        {
            contents.symbol = "string";
        }
        topFunctionCallValuesStackAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
        ram.SetP (topFunctionCallValuesStackAddress);
        ram.Push (contents.symbol, contents.value);
        topFunctionCallValuesStackAddress = ram.GetP ();
        SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (topFunctionCallValuesStackAddress));
    }

    programTextEndAddress = GetKernelVariableIntegerValue ("END_ADDRESS");
    contents.symbol = "call";
    contents.value = "main";
    Call (programTextEndAddress, contents);

    return;
}

bool ExecuteOneInstruction (string address, string buffer, string result)
{
    int instructionPointer;
    int quantumTimeRemaining;    
    memoryCell contents;
    bool executableInstruction;
    int segmentSize;
    bool systemCall;
	
	SetClockTicks();

	int runTime = 0;
	runTime = GetKernelVariableIntegerValue("RUNNING_TIME");
	runTime += 1;
	SetKernelVariableValue("RUNNING_TIME", itos(runTime));
	

    string restartInstructionStatusFlag;

    instructionPointer = atoi (address.c_str ());
    restartInstructionStatusFlag = GetKernelVariableStringValue ("RESTART_INSTRUCTION_STATUS_FLAG");
    if (restartInstructionStatusFlag == "false")
    {
        instructionPointer ++;
        SetKernelVariableValue ("INSTRUCTION_POINTER", itos (instructionPointer));
    }
    quantumTimeRemaining = GetKernelVariableIntegerValue ("QUANTUM_TIME_REMAINING");
    SetKernelVariableValue ("QUANTUM_TIME_REMAINING", itos (quantumTimeRemaining - 1));
    if (instructionPointer <= GetKernelVariableIntegerValue ("END_ADDRESS"))
    {
        ram.SetP (instructionPointer);
        FetchInstruction (contents);
        executableInstruction = DecodeInstruction (contents);
        if (executableInstruction)
        {
            systemCall = ExecuteInstruction (instructionPointer, contents, restartInstructionStatusFlag, buffer, result);
            if (systemCall)
            {
                if (restartInstructionStatusFlag == "false")
                {
                    if (GetKernelVariableIntegerValue ("QUANTUM_TIME_REMAINING") == 0)
                    {
                        SetKernelVariableValue ("QUANTUM_TIME_REMAINING", itos (QUANTUM_LENGTH));
                    }
                    SetKernelVariableValue ("RESTART_INSTRUCTION_STATUS_FLAG", "true");
                    ProcessImageToFile (GetKernelVariableIntegerValue ("PID"), "backingstore");
                    DeleteProcessImage (GetMemorySegmentBoundary ("PROGRAM_TEXT_START_ADDRESS", segmentSize));
                }
                else
                {
                    SetKernelVariableValue ("RESTART_INSTRUCTION_STATUS_FLAG", "false");
                }
            }
        }       
    }
    else
    {        

	EndMain ();
    }

    if (GetKernelVariableStringValue ("RETURN_STATUS_FLAG") == "true")
    {
        return true;
    }

    return false;
}

void FetchInstruction (memoryCell& contents)
{
    contents = ram.Read ();

    return;
}

bool DecodeInstruction (memoryCell contents)
{
    if (contents.symbol == "comment" ||
        contents.symbol == "call" ||
        contents.symbol == "return" ||
        contents.symbol == "put" ||
        contents.symbol == "unput" ||
        contents.symbol == "set" ||
        contents.symbol == "compare" ||
        contents.symbol == "add" ||
        contents.symbol == "subtract" ||
        contents.symbol == "multiply" ||
        contents.symbol == "divide" ||
        contents.symbol == "modulo" ||
        contents.symbol == "variable" ||
        contents.symbol == "constant" ||
        contents.symbol == "reference" ||
        contents.symbol == "jump" ||
        contents.symbol == "jumpless" ||
        contents.symbol == "jumpequal" ||
        contents.symbol == "jumpgreater" ||
        contents.symbol == "file" ||
        contents.symbol == "open" ||
        contents.symbol == "read" ||
        contents.symbol == "write" ||
        contents.symbol == "newline" ||
        contents.symbol == "close" ||
        contents.symbol == "coresnapshot")
    {
        return true;
    }
    else if (GetLocalSymbolAddress (contents.symbol) != -1)
    {
        return true;
    }

    return false;
}

bool ExecuteInstruction (int currentProgramTextAddress, memoryCell contents,
                         string restartInstructionStatusFlag, string buffer, string result)
{
    if (contents.symbol == "comment")
    {
        Comment ();
        return false;
    }
    else if (contents.symbol == "call")
    {
        Call (currentProgramTextAddress, contents);
        return false;
    }
    else if (contents.symbol == "return")
    {
        Return (contents);
        return false;
    }
    else if (contents.symbol == "put")
    {
        Put (contents);
        return false;
    }
    else if (contents.symbol == "unput")
    {
        Unput ();
        return false;
    }
    else if (contents.symbol == "set")
    {
        Set (contents);
        return false;
    }
    else if (contents.symbol == "compare")
    {
        Compare (contents.value);
        return false;
    }
    else if (contents.symbol == "jump" ||
             contents.symbol == "jumpless" ||
             contents.symbol == "jumpequal" ||
             contents.symbol == "jumpgreater")
    {
        Jump (contents);
        return false;
    }
    else if (contents.symbol == "add" ||
             contents.symbol == "subtract" ||
             contents.symbol == "multiply" ||
             contents.symbol == "divide" ||
             contents.symbol == "modulo")
    {
        DoTheMath (contents.symbol);
        return false;
    }
    else if (contents.symbol == "open")
    {
        Open (contents.value, restartInstructionStatusFlag, result);
        return true;
    }
    else if (contents.symbol == "read")
    {
        Read (contents.value, restartInstructionStatusFlag, buffer, result);
        return true;
    }
    else if (contents.symbol == "write")
    {
        Write (contents.value, restartInstructionStatusFlag, result);
        return true;
    }
    else if (contents.symbol == "newline")
    {
        Newline (contents.value, restartInstructionStatusFlag, result);
        return true;
    }
    else if (contents.symbol == "close")
    {
        Close (contents.value, restartInstructionStatusFlag);
        return true;
    }
    else if (contents.symbol == "variable" ||
             contents.symbol == "constant" ||
             contents.symbol == "reference" ||
             contents.symbol == "file")
    {
        AllocateLocalSymbol (contents);
        return false;
    }
    else if (contents.symbol == "coresnapshot")
    {
        CoreSnapShot ();
        return false;
    }
    else if (GetLocalSymbolAddress (contents.symbol) != -1 &&
            (contents.value == "integer" ||
             contents.value == "float" ||
             contents.value == "string" ||
             contents.value == "address" ||
             contents.value == "input" ||
             contents.value == "output"))
    {
        AssignTypeToLocalSymbol (contents);
        return false;
    }
    else if (GetLocalSymbolAddress (contents.symbol) != -1 &&
             IsInteger (contents.value))
    {
        AllocateLocalArray (contents);
        return false;
    }

    return false;
}

void EndMain ()
{
	SetKernelVariableValue("END_TIME", itos (HALclock));

    memoryCell contents;

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
    contents = ram.Pop ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));
    SetKernelVariableValue ("RETURN_VALUE", contents.value);
    SetKernelVariableValue ("RETURN_STATUS_FLAG", "true");

    return;
}

void Open (string symbol, string restartInstructionStatusFlag, string result)
{
    int pid;
    int address1;
    int address2;
    memoryCell contents;
    string mode;
    bool isLocalSymbol = true;

    address1 = GetLocalSymbolAddress (symbol);
    if (address1 == -1)
    {
        isLocalSymbol = false;
        address1 = GetGlobalSymbolAddress (symbol);
        if (address1 == -1)
        {
            cout << "HAL9000: " << symbol << " not defined" << endl;
            CoreDump ();
            exit (1);
        }
    }
    pid = GetKernelVariableIntegerValue ("PID");

    ram.SetP (address1);
    contents = ram.Read ();
    address2 = atoi (contents.value.c_str ());
    ram.SetP (address2);
    contents = ram.Read ();
    if (restartInstructionStatusFlag == "false")
    {
        if (contents.symbol == "input_file")
        {
            mode = "input";
        }
        else if (contents.symbol == "output_file")
        {
            mode = "output";
        }
        else
        {
            cout << "HAL9000: cannot open " << symbol << " as it is not a file" << endl;
            CoreDump ();
            exit (1);
        }
        if (symbol == "keyboard" || symbol == "display")
        {
            SendMessageToHALos ("SYSTEM_CALL", "OPEN", itos (pid), symbol, "", mode);
        }
        else
        {
            address1 = ram.GetP ();
            if (isLocalSymbol)
            {
                address1 --;
            }
            else
            {
                address1 ++;
            }
            ram.SetP (address1);
            contents = ram.Read ();
            SendMessageToHALos ("SYSTEM_CALL", "OPEN", itos (pid), contents.value, itos (address2), mode);
        }
    }
    else
    {
        if (result == "FILE_OPEN_FAILED")
        {
            cout << "HAL9000: unable to open " << symbol << endl;
            CoreDump ();
            exit (1);
        }
        else if (result == "FILE_ALREADY_OPEN")
        {
            cout << "HAL9000: " << symbol << " already open" << endl;
            CoreDump ();
            exit (1);
        }
        contents.value = "open";
        ram.ReWrite ("", contents.value);
    }

    return;    
}

void Read (string symbol, string restartInstructionStatusFlag, string buffer, string result)
{
    int pid;
    int address1;
    int address2;
    memoryCell contents;
    bool isLocalSymbol = true;

    address1 = GetLocalSymbolAddress (symbol);
    if (address1 == -1)
    {
        isLocalSymbol = false;
        address1 = GetGlobalSymbolAddress (symbol);
        if (address1 == -1)
        {
            cout << "HAL9000: " << symbol << " not defined" << endl;
            CoreDump ();
            exit (1);
        }
    }
    pid = GetKernelVariableIntegerValue ("PID");

    ram.SetP (address1);
    contents = ram.Read ();
    address2 = atoi (contents.value.c_str ());
    ram.SetP (address2);
    contents = ram.Read ();
    if (restartInstructionStatusFlag == "false")
    {
        if (contents.symbol != "input_file")
        {
            cout << "HAL9000: cannot read from " << symbol << " as it is not an input file" << endl;
            CoreDump ();
            exit (1);
        }
        if (contents.value != "open")
        {
            cout << "HAL9000: cannot read from " << symbol << " as it is not open" << endl;
            CoreDump ();
            exit (1);
        }
        if (symbol == "keyboard")
        {
            SendMessageToHALos ("SYSTEM_CALL", "READ", itos (pid), symbol, "", "");
        }
        else
        {
            address1 = ram.GetP ();
            if (isLocalSymbol)
            {
                address1 --;
            }
            else
            {
                address1 ++;
            }
            ram.SetP (address1);
            contents = ram.Read ();
            SendMessageToHALos ("SYSTEM_CALL", "READ", itos (pid), contents.value, itos (address2), "");
        }
    }
    else
    {
        if (result == "KEYBOARD_READ_OK" || result == "FILE_READ_OK")
        {
            ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
            ram.Push ("string", buffer);
            SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));
        }
        else if (result == "FILE_AT_END")
        {
            ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
            ram.Push ("eof", "");
            SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));
        }
        else // (result == "FILE_READ_FAILED")
        {
            cout << "HAL9000: unable to read from " << symbol << endl;
            CoreDump ();
            exit (1);
        }
    }

    return;    
}

void Write (string symbol, string restartInstructionStatusFlag, string result)
{
    int pid;
    int address1;
    int address2;
    memoryCell contents1;
    memoryCell contents2;
    bool isLocalSymbol = true;

    address1 = GetLocalSymbolAddress (symbol);
    if (address1 == -1)
    {
        isLocalSymbol = false;
        address1 = GetGlobalSymbolAddress (symbol);
        if (address1 == -1)
        {
            cout << "HAL9000: " << symbol << " not defined" << endl;
            CoreDump ();
            exit (1);
        }
    }
    pid = GetKernelVariableIntegerValue ("PID");
    
    ram.SetP (address1);
    contents1 = ram.Read ();
    address2 = atoi (contents1.value.c_str ());
    ram.SetP (address2);
    contents1 = ram.Read ();
    address1 = ram.GetP ();
    if (isLocalSymbol)
    {
        address1 --;
    }
    else
    {
        address1 ++;
    }
    if (restartInstructionStatusFlag == "false")
    {
        if (contents1.symbol != "output_file")
        {
            cout << "HAL9000: cannot write to " << symbol << " as it is not an output file" << endl;
            CoreDump ();
            exit (1);
        }
        if (contents1.value != "open")
        {
            cout << "HAL9000: cannot write to " << symbol << " as it is not open" << endl;
            CoreDump ();
            exit (1);
        }
        ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
        contents1 = ram.Pop ();
        SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));
        if (contents1.symbol == "float")
        {
            if (IsInteger (contents1.value))
            {
                contents1.value = contents1.value + ".0";
            }
        }
        if (symbol == "display")
        {
            SendMessageToHALos ("SYSTEM_CALL", "WRITE", itos (pid), symbol, "", contents1.value);
        }
        else
        {
            ram.SetP (address1);
            contents2 = ram.Read ();
            SendMessageToHALos ("SYSTEM_CALL", "WRITE", itos (pid), contents2.value, itos (address2), contents1.value);
        }
    }
    else
    {
        if (result == "FILE_WRITE_FAILED")
        {
            cout << "HAL9000: unable to write to " << symbol << endl;
            CoreDump ();
            exit (1);
        }
    }

    return;    
}

void Newline (string symbol, string restartInstructionStatusFlag, string result)
{
    int pid;
    int address1;
    int address2;
    memoryCell contents;
    bool isLocalSymbol = true;

    address1 = GetLocalSymbolAddress (symbol);
    if (address1 == -1)
    {
        isLocalSymbol = false;
        address1 = GetGlobalSymbolAddress (symbol);
        if (address1 == -1)
        {
            cout << "HAL9000: " << symbol << " not defined" << endl;
            CoreDump ();
            exit (1);
        }
    }
    pid = GetKernelVariableIntegerValue ("PID");

    ram.SetP (address1);
    contents = ram.Read ();
    address2 = atoi (contents.value.c_str ());
    ram.SetP (address2);
    contents = ram.Read ();
    if (restartInstructionStatusFlag == "false")
    {
        if (contents.symbol != "output_file")
        {
            cout << "HAL9000: cannot newline to " << symbol << " as it is not an output file" << endl;
            CoreDump ();
            exit (1);
        }
        if (contents.value != "open")
        {
            cout << "HAL9000: cannot newline to " << symbol << " as it is not open" << endl;
            CoreDump ();
            exit (1);
        }
        if (symbol == "display")
        {
            SendMessageToHALos ("SYSTEM_CALL", "NEWLINE", itos (pid), symbol, "", "");
        }
        else
        {
            address1 = ram.GetP ();
            if (isLocalSymbol)
            {
                address1 --;
            }
            else
            {
                address1 ++;
            }
            ram.SetP (address1);
            contents = ram.Read ();
            SendMessageToHALos ("SYSTEM_CALL", "NEWLINE", itos (pid), contents.value, itos (address2), "");
        }
    }
    else
    {
        if (result == "FILE_NEWLINE_FAILED")
        {
            cout << "HAL9000: unable to newline to " << symbol << endl;
            CoreDump ();
            exit (1);
        }
    }

    return;    
}

void Close (string symbol, string restartInstructionStatusFlag)
{
    int pid;
    int address1;
    int address2;
    memoryCell contents;
    bool isLocalSymbol = true;

    address1 = GetLocalSymbolAddress (symbol);
    if (address1 == -1)
    {
        isLocalSymbol = false;
        address1 = GetGlobalSymbolAddress (symbol);
        if (address1 == -1)
        {
            cout << "HAL9000: " << symbol << " not defined" << endl;
            CoreDump ();
            exit (1);
        }
    }
    pid = GetKernelVariableIntegerValue ("PID");

    ram.SetP (address1);
    contents = ram.Read ();
    address2 = atoi (contents.value.c_str ());
    ram.SetP (address2);
    contents = ram.Read ();
    if (restartInstructionStatusFlag == "false")
    {
        if (contents.symbol != "input_file" && contents.symbol != "output_file")
        {
            cout << "HAL9000: cannot close " << symbol << " as it is not a file" << endl;
            CoreDump ();
            exit (1);
        }
        if (symbol == "keyboard" || symbol == "display")
        {
            SendMessageToHALos ("SYSTEM_CALL", "CLOSE", itos (pid), symbol, "", "");
        }
        else
        {
            address1 = ram.GetP ();
            if (isLocalSymbol)
            {
                address1 --;
            }
            else
            {
                address1 ++;
            }
            ram.SetP (address1);
            contents = ram.Read ();
            SendMessageToHALos ("SYSTEM_CALL", "CLOSE", itos (pid), contents.value, itos (address2), "");
        }
    }
    else
    {
        contents.value = "closed";
        ram.ReWrite ("", contents.value);
    }

    return;    
}

void Comment ()
{
    return;
}

void Call (int currentProgramTextAddress, memoryCell contents)
{
    int functionCallStackStartAddress;
    int topFunctionCallStackAddress;
    int functionCallValuesStackStartAddress;
    int topFunctionCallValuesStackAddress;
    int segmentSize;
    int address;
    string functionName = contents.value;

    functionCallValuesStackStartAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_VALUES_STACK_START_ADDRESS", segmentSize);
    topFunctionCallValuesStackAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
    ram.SetP (topFunctionCallValuesStackAddress);
    if (ram.GetP () == functionCallValuesStackStartAddress - segmentSize)
    {
        cout << "HAL9000: function call values stack segmentation violation" << endl;
        return;
    }
    ram.Push ("return_address", itos (currentProgramTextAddress));
    topFunctionCallValuesStackAddress = ram.GetP ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (topFunctionCallValuesStackAddress));
    SetKernelVariableValue ("NEXT_ARGUMENT_ADDRESS", itos (topFunctionCallValuesStackAddress + 1));

    functionCallStackStartAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_STACK_START_ADDRESS", segmentSize);
    topFunctionCallStackAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_STACK_ADDRESS");
    ram.SetP (topFunctionCallStackAddress);
    if (ram.GetP () == functionCallStackStartAddress - segmentSize)
    {
        cout << "HAL9000: function call stack segmentation violation" << endl;
        CoreDump ();
        exit (1);
    }
    ram.Push ("call_" + functionName, itos (topFunctionCallValuesStackAddress));
    topFunctionCallStackAddress = ram.GetP ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_STACK_ADDRESS", itos (topFunctionCallStackAddress));

    address = GetGlobalSymbolAddress (functionName);
    if (address != -1)
    {
        ram.SetP (address);
        contents = ram.Read ();
        ram.SetP (atoi (contents.value.c_str ()));
        contents = ram.Read ();
        SetKernelVariableValue ("INSTRUCTION_POINTER", contents.value);
    }
    else
    {
        cout << "HAL9000: function " << functionName << " not defined" << endl;
        CoreDump ();
        exit (1);
    }

    return;
}

void Return (memoryCell contents1)
{
    int functionCallValuesStackStartAddress;
    int topFunctionCallValuesStackAddress;
    int segmentSize;
    memoryCell contents2;

    functionCallValuesStackStartAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_VALUES_STACK_START_ADDRESS", segmentSize);
    topFunctionCallValuesStackAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
    ram.SetP (topFunctionCallValuesStackAddress);
    if (ram.GetP () == functionCallValuesStackStartAddress - segmentSize)
    {
        cout << "HAL9000: function call values stack segmentation violation" << endl;
        CoreDump ();
        exit (1);
    }

    if (contents1.value.length () == 0)
    {
        ;
    }
    else
    {
        contents2 = DetermineMemoryCellContentsForReturnAndPutCommands (contents1);
    }

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_STACK_ADDRESS"));
    contents1 = ram.Read ();
    while (1)
    {
        if (contents1.symbol.substr (0, 5) != "call_")
        {
            contents1 = ram.Pop ();
            SetKernelVariableValue ("TOP_FUNCTION_CALL_STACK_ADDRESS", itos (ram.GetP ()));
            ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_STACK_ADDRESS"));
            contents1 = ram.Read ();
        }
        else
        {
            contents1 = ram.Pop ();
            SetKernelVariableValue ("TOP_FUNCTION_CALL_STACK_ADDRESS", itos (ram.GetP ()));
            break;
        }
    }

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
    contents1 = ram.Read ();
    while (1)
    {
        if (contents1.symbol != "return_address")
        {
            contents1 = ram.Pop ();
            SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));
            ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
            contents1 = ram.Read ();
        }
        else
        {
            contents1 = ram.Pop ();
            SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));
            break;
        }
    }
    SetKernelVariableValue ("INSTRUCTION_POINTER", contents1.value);

    if (contents2.symbol.length () > 0)
    {
        ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
        ram.Push (contents2.symbol, contents2.value);
        SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));
    }

    return;
}

void Put (memoryCell contents1)
{
    int functionCallValuesStackStartAddress;
    int topFunctionCallValuesStackAddress;
    int segmentSize;
    memoryCell contents2;

    functionCallValuesStackStartAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_VALUES_STACK_START_ADDRESS", segmentSize);
    topFunctionCallValuesStackAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
    ram.SetP (topFunctionCallValuesStackAddress);
    if (ram.GetP () == functionCallValuesStackStartAddress - segmentSize)
    {
        cout << "HAL9000: function call values stack segmentation violation" << endl;
        CoreDump ();
        exit (1);
    }

    // put :argument
    if (contents1.value == "argument")
    {
        ram.SetP (GetKernelVariableIntegerValue ("NEXT_ARGUMENT_ADDRESS"));
        contents1 = ram.Read ();
        contents2.symbol = GetDataType (contents1.symbol);
        contents2.value = contents1.value;
        SetKernelVariableValue ("NEXT_ARGUMENT_ADDRESS", itos (ram.GetP () + 1 ));
    }
    else
    {
        contents2 = DetermineMemoryCellContentsForReturnAndPutCommands (contents1);
    }

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
    ram.Push (contents2.symbol, contents2.value);
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));

    return;
}

memoryCell DetermineMemoryCellContentsForReturnAndPutCommands (memoryCell contents1)
{
    memoryCell contents2;
    int address;
    int symbolAddress;
    int indexAddress;
    bool isLocalSymbol = true;
    size_t foundArrayBracket;
    size_t foundArrayAddress;
    size_t foundFileAddress;
    string index;
    size_t indexStartPosition;
    size_t indexEndPosition;

    // put/return string
    if (contents1.value [0] == '\'')
    {
        contents2.symbol = "string";
        if (contents1.value.length () > 1)
        {
            contents2.value = contents1.value.substr (1, contents2.value.length () - 1);
        }
        else
        {
            contents2.value = "";
        }
    }
    // put/return :@a or put/return :@a<?> (? is literal integer constant, variable name, or constant name)
    else if (contents1.value [0] == '@')
    {
        contents1.value = contents1.value.substr (1, contents1.value.length () - 1);
        foundArrayBracket = contents1.value.find ("<");
        // return :@a
        if (foundArrayBracket == string::npos)
        {
            symbolAddress = GetLocalSymbolAddress (contents1.value);
            if (symbolAddress == -1)
            {
                symbolAddress = GetGlobalSymbolAddress (contents1.value);
                if (symbolAddress == -1)
                {
                    cout << "HAL9000: " << contents1.value << " not defined" << endl;
                    CoreDump ();
                    exit (1);
                }
            }
            ram.SetP (symbolAddress);
            contents1 = ram.Read ();
            address = atoi (contents1.value.c_str ());
            ram.SetP (atoi (contents1.value.c_str ()));
            contents1 = ram.Read ();
            contents2.symbol = contents1.symbol + "_address";
            contents2.value = itos (address);
        }
        // put/return :@a<?> (? is a literal integer constant, variable name, or constant name)
        else
        {
            indexStartPosition = foundArrayBracket + 1;
            indexEndPosition = contents1.value.find (">") - 1;
            index = contents1.value.substr (indexStartPosition, indexEndPosition - indexStartPosition + 1);
            contents1.value = contents1.value.substr (0, indexStartPosition - 1);
            symbolAddress = GetLocalSymbolAddress (contents1.value);
            if (symbolAddress == -1)
            {
                isLocalSymbol = false;
                symbolAddress = GetGlobalSymbolAddress (contents1.value);
                if (symbolAddress == -1)
                {
                    cout << "HAL9000: " << contents1.value << " not defined" << endl;
                    CoreDump ();
                    exit (1);
                }
            }
            ram.SetP (symbolAddress);
            contents1 = ram.Read ();
            address = atoi (contents1.value.c_str ());
            ram.SetP (atoi (contents1.value.c_str ()));
            contents1 = ram.Read ();
            foundArrayAddress = contents1.symbol.find ("array");
            contents2.symbol = contents1.symbol.substr (0, foundArrayAddress - 1) + "_address";
            // put/return :@a<9> (9 is any literal integer constant)
            if (IsInteger (index))
            {
                if (isLocalSymbol)
                {
                    address = address - atoi (index.c_str ());
                }
                else
                {
                    address = address + atoi (index.c_str ());
                }
                contents2.value = itos (address);
            }
            // put/return :@a<i> (i is a variable name or constant name)
            else
            {
                indexAddress = GetLocalSymbolAddress (index);
                if (indexAddress == -1)
                {
                    indexAddress = GetGlobalSymbolAddress (index);
                    if (indexAddress == -1)
                    {
                        cout << "HAL9000: " << index << " not defined" << endl;
                        CoreDump ();
                        exit (1);
                    }
                }
                ram.SetP (indexAddress);
                contents1 = ram.Read ();
                ram.SetP (atoi (contents1.value.c_str ()));
                contents1 = ram.Read ();
                if (isLocalSymbol)
                {
                    address = address - atoi (contents1.value.c_str ());
                }
                else
                {
                    address = address + atoi (contents1.value.c_str ());
                }
                contents2.value = itos (address);
            }
        }
    }
    else
    {
        foundArrayBracket = contents1.value.find ("<");
        // put/return :a<?> (? is a literal integer constant, variable name, or constant name)
        if (foundArrayBracket != string::npos)
        {
            indexStartPosition = foundArrayBracket + 1;
            indexEndPosition = contents1.value.find (">") - 1;
            index = contents1.value.substr (indexStartPosition, indexEndPosition - indexStartPosition + 1);
            contents1.value = contents1.value.substr (0, indexStartPosition - 1);
            symbolAddress = GetLocalSymbolAddress (contents1.value);
            if (symbolAddress == -1)
            {
                isLocalSymbol = false;
                symbolAddress = GetGlobalSymbolAddress (contents1.value);
                if (symbolAddress == -1)
                {
                    cout << "HAL9000: " << contents1.value << " not defined" << endl;
                    CoreDump ();
                    exit (1);
                }
            }
            ram.SetP (symbolAddress);
            contents1 = ram.Read ();
            address = atoi (contents1.value.c_str ());
            ram.SetP (atoi (contents1.value.c_str ()));
            contents1 = ram.Read ();
            contents2.symbol = GetDataType (contents1.symbol);
            // put/return :a<9> (9 is any literal integer constant)
            if (IsInteger (index))
            {
                if (isLocalSymbol)
                {
                    ram.SetP (address - atoi (index.c_str ()));
                }
                else
                {
                    ram.SetP (address + atoi (index.c_str ()));
                }
            }
            // put/return :a<i> (i is a variable name or constant name)
            else
            {
                indexAddress = GetLocalSymbolAddress (index);
                if (indexAddress == -1)
                {
                    indexAddress = GetGlobalSymbolAddress (index);
                    if (indexAddress == -1)
                    {
                        cout << "HAL9000: " << index << " not defined" << endl;
                        CoreDump ();
                        exit (1);
                    }
                }
                ram.SetP (indexAddress);
                contents1 = ram.Read ();
                ram.SetP (atoi (contents1.value.c_str ()));
                contents1 = ram.Read ();
                if (isLocalSymbol)
                {
                    ram.SetP (address - atoi (contents1.value.c_str ()));
                }
                else
                {
                    ram.SetP (address + atoi (contents1.value.c_str ()));
                }
            }
            contents1 = ram.Read ();
            contents2.value = contents1.value;
        }
        // put/return :a or put/return :9 or put/return :9.9 or put/return :s
        else
        {
            symbolAddress = GetLocalSymbolAddress (contents1.value);
            if (symbolAddress == -1)
            {
                isLocalSymbol = false;
                symbolAddress = GetGlobalSymbolAddress (contents1.value);
            }
            if (symbolAddress != -1)
            {
                ram.SetP (symbolAddress);
                contents1 = ram.Read ();
                ram.SetP (atoi (contents1.value.c_str ()));
                contents1 = ram.Read ();
                foundArrayAddress = contents1.symbol.find ("array");
                // put/return :a (where a is not an array name)
                if (foundArrayAddress == string::npos)
                {
                    foundFileAddress = contents1.symbol.find ("file");
                    if (foundFileAddress == string::npos)
                    {
                        contents2.symbol = GetDataType (contents1.symbol);
                        contents2.value = contents1.value;
                    }
                    // put/return :a (where a is a file)
                    else
                    {
                        contents2.symbol = GetDataType (contents1.symbol);
                        if (isLocalSymbol)
                        {
                            contents2.symbol = "local_" + contents2.symbol + "_address";
                        }
                        else
                        {
                            contents2.symbol = "global_" + contents2.symbol + "_address";
                        }
                        contents2.value = itos (ram.GetP ());
                    }
                }
                // put/return :a (where a is an array name)
                else
                {
                    foundArrayAddress = contents1.symbol.find ("0");
                    contents2.symbol = contents1.symbol.substr (0, foundArrayAddress - 1);
                    if (isLocalSymbol)
                    {
                        contents2.symbol = "local_" + contents2.symbol + "_address";
                    }
                    else
                    {
                        contents2.symbol = "global_" + contents2.symbol + "_address";
                    }
                    contents2.value = itos (ram.GetP ());
                }
            }
            // put/return :9 (9 is any integer)
            else if (IsInteger (contents1.value))
            {
                contents2.symbol = "integer";
                contents2.value = contents1.value;
            }
            // put/return :9.9 (9.9 is any float)
            else if (IsFloat (contents1.value))
            {
                contents2.symbol = "float";
                contents2.value = contents1.value;
            }
            // put/return :s (s is any string)
            else 
            {
                contents2.symbol = "string";
                contents2.value = contents1.value;
            }
        }
    }

    return contents2;
}

void Set (memoryCell contents)
{
    string symbol;
    string value;
    size_t foundArrayBracket;
    string index;
    size_t indexStartPosition;
    size_t indexEndPosition;
    int addressOffset = 0;
    int address;

    symbol = contents.value;

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
    contents = ram.Pop ();
    value = contents.value;
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));

    if (symbol [0] == '@')
    {
        symbol = symbol.substr (1, symbol.length () - 1);
    }
    foundArrayBracket = symbol.find ("<");
    if (foundArrayBracket != string::npos)
    {
        indexStartPosition = foundArrayBracket + 1;
        indexEndPosition = symbol.find (">") - 1;
        index = symbol.substr (indexStartPosition, indexEndPosition - indexStartPosition + 1);
        if (IsInteger (index))
        {
            symbol = symbol.substr (0, indexStartPosition - 1);
            addressOffset = atoi (index.c_str ());
        }
        else
        {
            address = GetLocalSymbolAddress (index);
            if (address != -1)
            {
                ram.SetP (address);
                contents = ram.Read ();
                symbol = symbol.substr (0, indexStartPosition - 1);
                ram.SetP (atoi (contents.value.c_str ()));
                contents = ram.Read ();
                addressOffset = atoi (contents.value.c_str ());
            }
            else
            {
                address = GetGlobalSymbolAddress (index);
                if (address != -1)
                {
                    ram.SetP (address);
                    contents = ram.Read ();
                    symbol = symbol.substr (0, indexStartPosition - 1);
                    ram.SetP (atoi (contents.value.c_str ()));
                    contents = ram.Read ();
                    addressOffset = atoi (contents.value.c_str ());
                }
                else
                {
                    cout << "HAL9000: " << index << " is an undeclared symbol" << endl;
                    CoreDump ();
                    exit (1);
                }
            }
        }
    } 
    
    address = GetLocalSymbolAddress (symbol);
    if (address != -1)
    {
        ram.SetP (address);
        contents = ram.Read ();
        ram.SetP (atoi (contents.value.c_str ()) - addressOffset);
        contents = ram.Read ();
        if (contents.symbol == "input_file" || contents.symbol == "output_file")
        {
            address = ram.GetP ();
            ram.SetP (address - 1);
            contents = ram.Read ();
        }
    }
    else
    {
        address = GetGlobalSymbolAddress (symbol);
        if (address != -1)
        {
            ram.SetP (address);
            contents = ram.Read ();
            ram.SetP (atoi (contents.value.c_str ()) + addressOffset);
            contents = ram.Read ();
            if (contents.symbol == "input_file" || contents.symbol == "output_file")
            {
                address = ram.GetP ();
                ram.SetP (address + 1);
                contents = ram.Read ();
            }
        }
        else
        {
            cout << "HAL9000: " << symbol << " is an undeclared symbol" << endl;
            CoreDump ();
            exit (1);
        }
    }

    ram.ReWrite ("", value);

    return;
}

void Compare (string value)
{
    if (value.length () == 0)
    {
        CompareValues ();
    }
    else if (value == "eof")
    {
        CompareEndOfFile ();
    }
    else // (value == "integer" || "float" || "string" || "emptystring")
    {
        CompareDataTypes (value);
    }

    return;
}

void CompareEndOfFile ()
{
    memoryCell contents;
    int comparisonStatusFlag;

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
    contents = ram.Pop ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));

    if (contents.symbol == "eof")
    {
        comparisonStatusFlag = 0;
    }
    else
    {
        comparisonStatusFlag = -2;
        ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
        ram.Push (contents.symbol, contents.value);
        SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));
    }

    SetKernelVariableValue ("COMPARISON_STATUS_FLAG", itos (comparisonStatusFlag));

    return;
}

void CompareDataTypes (string dataType)
{
    memoryCell contents;
    int comparisonStatusFlag;

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
    contents = ram.Pop ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));

    if (dataType == "integer")
    {
        if (IsInteger (contents.value))
        {
            comparisonStatusFlag = 0;
            contents.symbol = "integer";
        }
        else
        {
            comparisonStatusFlag = -2;
        }
    }
    else if (dataType == "float")
    {
        if (IsFloat (contents.value))
        {
            comparisonStatusFlag = 0;
            contents.symbol = "float";
        }
        else
        {
            comparisonStatusFlag = -2;
        }
    }
    else if (dataType == "string")
    {
        if (contents.value.length () > 0)
        {
            comparisonStatusFlag = 0;
        }
        else
        {
            comparisonStatusFlag = -2;
        }
    }
    else if (dataType == "emptystring")
    {
        if (contents.value.length () == 0)
        {
            comparisonStatusFlag = 0;
        }
        else
        {
            comparisonStatusFlag = -2;
        }
    }

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
    ram.Push (contents.symbol, contents.value);
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));

    SetKernelVariableValue ("COMPARISON_STATUS_FLAG", itos (comparisonStatusFlag));

    return;
}

void CompareValues ()
{
    memoryCell contents1;
    memoryCell contents2;
    int integerValue1;
    int integerValue2;
    double floatValue1;
    double floatValue2;
    string stringValue1;
    string stringValue2;
    int comparisonStatusFlag;

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
    contents2 = ram.Pop ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
    contents1 = ram.Pop ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));

    if (contents1.symbol == "integer")
    {
        integerValue1 = atoi (contents1.value.c_str ());
        if (contents2.symbol == "integer")
        {
            integerValue2 = atoi (contents2.value.c_str ());
            if (integerValue1 == integerValue2)
            {
                comparisonStatusFlag = 0;
            }
            else if (integerValue1 > integerValue2)
            {
                comparisonStatusFlag = 1;
            }
            else
            {
                comparisonStatusFlag = -1;
            }
        }
        else if (contents2.symbol == "float")
        {
            floatValue2 = atof (contents2.value.c_str ());
            if (integerValue1 == floatValue2)
            {
                comparisonStatusFlag = 0;
            }
            else if (integerValue1 > floatValue2)
            {
                comparisonStatusFlag = 1;
            }
            else
            {
                comparisonStatusFlag = -1;
            }
        }
        else
        {
            cout << "HAL9000: integer and string types mismatched in compare operation" << endl;
            CoreDump ();
            exit (1);
        }
    }
    else if (contents1.symbol == "float")
    {
        floatValue1 = atof (contents1.value.c_str ());
        if (contents2.symbol == "integer")
        {
            integerValue2 = atoi (contents2.value.c_str ());
            if (floatValue1 == integerValue2)
            {
                comparisonStatusFlag = 0;
            }
            else if (floatValue1 > integerValue2)
            {
                comparisonStatusFlag = 1;
            }
            else
            {
                comparisonStatusFlag = -1;
            }
        }
        else if (contents2.symbol == "float")
        {
            floatValue2 = atof (contents2.value.c_str ());
            if (floatValue1 == floatValue2)
            {
                comparisonStatusFlag = 0;
            }
            else if (floatValue1 > floatValue2)
            {
                comparisonStatusFlag = 1;
            }
            else
            {
                comparisonStatusFlag = -1;
            }
        }
        else
        {
            cout << "HAL9000: float and string types mismatched in compare operation" << endl;
            CoreDump ();
            exit (1);
        }
    }
    else // (contents1.symbol == "string")
    {
        stringValue1 = contents1.value;
        if (contents2.symbol == "integer")
        {
            cout << "HAL9000: string and integer types mismatched in compare operation" << endl;
            CoreDump ();
            exit (1);
        }
        else if (contents2.symbol == "float")
        {
            cout << "HAL9000: string and float types mismatched in compare operation" << endl;
            CoreDump ();
            exit (1);
        }
        else
        {
            stringValue2 = contents2.value;
            if (stringValue1 == stringValue2)
            {
                comparisonStatusFlag = 0;
            }
            else if (stringValue1 > stringValue2)
            {
                comparisonStatusFlag = 1;
            }
            else
            {
                comparisonStatusFlag = -1;
            }
        }
    }
    
    SetKernelVariableValue ("COMPARISON_STATUS_FLAG", itos (comparisonStatusFlag));

    return;
}

void Unput ()
{
    memoryCell contents;

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
    contents = ram.Pop ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));

    return;
}

void Jump (memoryCell contents)
{
    int segmentSize;
    int address;
    string labelName;
    int comparisonStatusFlag;

    labelName = contents.value;

    comparisonStatusFlag = GetKernelVariableIntegerValue ("COMPARISON_STATUS_FLAG");
    if ((contents.symbol == "jump") ||
        (contents.symbol == "jumpless" && comparisonStatusFlag == -1) ||
        (contents.symbol == "jumpequal" && comparisonStatusFlag == 0) ||
        (contents.symbol == "jumpgreater" && comparisonStatusFlag == 1))
    {
        address = GetGlobalSymbolAddress (labelName);
        if (address != -1)
        {
            ram.SetP (address);
            contents = ram.Read ();
            ram.SetP (atoi (contents.value.c_str ()));
            contents = ram.Read ();
            SetKernelVariableValue ("INSTRUCTION_POINTER", contents.value);
            SetKernelVariableValue ("COMPARISON_STATUS_FLAG", itos (-2));
        }
        else
        {
            cout << "HAL9000: label " << labelName << " does not exist" << endl;
            CoreDump ();
            exit (1);
        }
    }

    return;
}

void DoTheMath (string operation)
{
    int functionCallValuesStackStartAddress;
    int segmentSize;
    int topAddress;
    memoryCell contents1;
    memoryCell contents2;
    int integerOperand1;
    double floatOperand1;
    int integerOperand2;
    double floatOperand2;
    int integerResult;
    double floatResult;
    size_t foundAddress;

    functionCallValuesStackStartAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_VALUES_STACK_START_ADDRESS", segmentSize);
    topAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
    ram.SetP (topAddress);
    if (ram.GetP () > functionCallValuesStackStartAddress)
    {
        cout << "HAL9000: function call values stack segmentation violation" << endl;
        CoreDump ();
        exit (1);
    }
    contents2 = ram.Pop ();
    topAddress = ram.GetP ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (topAddress));
    foundAddress = contents2.symbol.find ("address");
    if (foundAddress != string::npos)
    {
        ram.SetP (atoi (contents2.value.c_str ()));
        contents2 = ram.Read ();
        contents2.symbol = GetDataType (contents2.symbol);
    }

    topAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
    ram.SetP (topAddress);
    if (ram.GetP () > functionCallValuesStackStartAddress)
    {
        cout << "HAL9000: function call values stack segmentation violation" << endl;
        CoreDump ();
        exit (1);
    }
    contents1 = ram.Pop ();
    topAddress = ram.GetP ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (topAddress));
    foundAddress = contents1.symbol.find ("address");
    if (foundAddress != string::npos)
    {
        ram.SetP (atoi (contents1.value.c_str ()));
        contents1 = ram.Read ();
        contents1.symbol = GetDataType (contents1.symbol);
    }

    if (contents1.symbol == "integer")
    {
        integerOperand1 = atoi (contents1.value.c_str ());
        if (contents2.symbol == "integer")
        {
            integerOperand2 = atoi (contents2.value.c_str ());
            if (operation == "add")
            {
                integerResult = integerOperand1 + integerOperand2;
            }
            else if (operation == "subtract")
            {
                integerResult = integerOperand1 - integerOperand2;
            }
            else if (operation == "multiply")
            {
                integerResult = integerOperand1 * integerOperand2;
            }
            else if (operation == "divide")
            {
                if (integerOperand2 == 0)
                {
                    cout << "HAL9000: division by zero" << endl;
                    CoreDump ();
                    exit (1);
                }
                integerResult = integerOperand1 / integerOperand2;
            }
            else // (operation == "modulo")
            {
                if (integerOperand2 == 0)
                {
                    cout << "HAL9000: modulo division by zero" << endl;
                    CoreDump ();
                    exit (1);
                }
                integerResult = integerOperand1 % integerOperand2;
            }
            topAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
            ram.SetP (topAddress);
            ram.Push ("integer", itos (integerResult));
            topAddress = ram.GetP ();
            SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (topAddress));
        }
        else if (contents2.symbol == "float")
        {
            if (operation == "modulo")
            {
                cout << "HAL9000: second operand in modulo division not an integer" << endl;
                CoreDump ();
                exit (1);
            }
            floatOperand2 = atof (contents2.value.c_str ());
            if (operation == "add")
            {
                floatResult = integerOperand1 + floatOperand2;
            }
            else if (operation == "subtract")
            {
                floatResult = integerOperand1 - floatOperand2;
            }
            else if (operation == "multiply")
            {
                floatResult = integerOperand1 * floatOperand2;
            }
            else // (operation == "divide")
            {
                if (floatOperand2 == 0.0)
                {
                    cout << "HAL9000: division by zero" << endl;
                    CoreDump ();
                    exit (1);
                }
                floatResult = integerOperand1 / floatOperand2;
            }
            topAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
            ram.SetP (topAddress);
            ram.Push ("integer", itos (floatResult));
            topAddress = ram.GetP ();
            SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (topAddress));
        }
        else
        {
            cout << "HAL9000: second operand in aarithmetic operation is non-numeric type" << endl;
            CoreDump ();
            exit (1);
        }
    }
    else if (contents1.symbol == "float")
    {
        if (operation == "modulo")
        {
            cout << "HAL9000: first operand in modulo division not an integer" << endl;
            CoreDump ();
            exit (1);
        }
        floatOperand1 = atof (contents1.value.c_str ());
        if (contents2.symbol == "float")
        {
            if (operation == "modulo")
            {
                cout << "HAL9000: second operand in modulo division not an integer" << endl;
                CoreDump ();
                exit (1);
            }
            floatOperand2 = atof (contents2.value.c_str ());
            if (operation == "add")
            {
                floatResult = floatOperand1 + floatOperand2;
            }
            else if (operation == "subtract")
            {
                floatResult = floatOperand1 - floatOperand2;
            }
            else if (operation == "multiply")
            {
                floatResult = floatOperand1 * floatOperand2;
            }
            else // (operation == "divide")
            {
                if (floatOperand2 == 0.0)
                {
                    cout << "HAL9000: division by zero" << endl;
                    CoreDump ();
                    exit (1);
                }
                floatResult = floatOperand1 / floatOperand2;
            }
            topAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
            ram.SetP (topAddress);
            ram.Push ("float", dtos (floatResult));
            topAddress = ram.GetP ();
            SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (topAddress));
        }
        else if (contents2.symbol == "integer")
        {
            integerOperand2 = atoi (contents2.value.c_str ());
            if (operation == "add")
            {
                floatResult = floatOperand1 + integerOperand2;
            }
            else if (operation == "subtract")
            {
                floatResult = floatOperand1 - integerOperand2;
            }
            else if (operation == "multiply")
            {
                floatResult = floatOperand1 * integerOperand2;
            }
            else // (operation == "divide")
            {
                if (integerOperand2 == 0)
                {
                    cout << "HAL9000: division by zero" << endl;
                    CoreDump ();
                    exit (1);
                }
                floatResult = floatOperand1 / integerOperand2;
            }
            topAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
            ram.SetP (topAddress);
            ram.Push ("float", dtos (floatResult));
            topAddress = ram.GetP ();
            SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (topAddress));
        }
        else
        {
            cout << "HAL9000: second operand in arithmetic operation is non-numeric type" << endl;
            CoreDump ();
            exit (1);
        }
    }
    else
    {
        cout << "HAL9000: first operand in arithmetic operation is non-numeric type" << endl;
        CoreDump ();
        exit (1);
    }

    return;
}

int GetLocalSymbolAddress (string symbol)
{
    memoryCell contents;

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_STACK_ADDRESS"));
    contents = ram.Read ();
    while (1)
    {
        if (contents.symbol == symbol)
        {
            return ram.GetP ();
        }
        else if (contents.symbol.substr (0, 5) == "call_")
        {
            return -1;
        }
        ram.IterateUp ();
        contents = ram.Read ();
    }
}

int GetGlobalSymbolAddress (string symbol)
{
    int globalSymbolsTableStartAddress;
    int lastGlobalSymbolsTableAddress;
    int segmentSize;
    memoryCell contents;

    globalSymbolsTableStartAddress = GetMemorySegmentBoundary ("GLOBAL_SYMBOLS_TABLE_START_ADDRESS", segmentSize);
    lastGlobalSymbolsTableAddress = GetKernelVariableIntegerValue ("LAST_GLOBAL_SYMBOLS_TABLE_ADDRESS");
    ram.SetP (globalSymbolsTableStartAddress);
    contents = ram.Read ();
    while (ram.GetP () <= lastGlobalSymbolsTableAddress)
    {
        if (contents.symbol == symbol)
        {
            return ram.GetP ();
        }
        ram.IterateUp ();
        contents = ram.Read ();
    }
        
    return -1;
}

void DeleteProcessImage (int startAddress)
{
    SetKernelVariableValue ("INSTRUCTION_POINTER", "null");
    SetKernelVariableValue ("PID", itos (-1));
    SetKernelVariableValue ("PROGRAM_NAME", "");
    SetKernelVariableValue ("START_ADDRESS", "null");
    SetKernelVariableValue ("END_ADDRESS", "null");
    SetKernelVariableValue ("RETURN_VALUE", "");
    SetKernelVariableValue ("RETURN_STATUS_FLAG", "false");
    SetKernelVariableValue ("COMPARISON_STATUS_FLAG", itos (-2));
    SetKernelVariableValue ("NEXT_ARGUMENT_ADDRESS", "null");
    SetKernelVariableValue ("QUANTUM_TIME_REMAINING", itos (QUANTUM_LENGTH));
    SetKernelVariableValue ("RESTART_INSTRUCTION_STATUS_FLAG", "false");
	SetKernelVariableValue("CREATION_TIME", itos(0));
	SetKernelVariableValue("RUNNING_TIME", itos(0));
	SetKernelVariableValue("END_TIME", itos(0));
    SetTableAndStackStartAddresses ();
    ram.Clear (startAddress);




    return;
}

void CoreSnapShot ()
{
    int pid;
    static int coreSnapShotCounter = 0;

    pid = GetKernelVariableIntegerValue ("PID");
    coreSnapShotCounter ++;
    ProcessImageToFile (pid, "coresnapshot_" + itos (coreSnapShotCounter));

    return;    
}

void CoreDump ()
{
    int pid;

    pid = GetKernelVariableIntegerValue ("PID");
    ProcessImageToFile (pid, "coredump");

    return;    
}

void ProcessImageToMemory (int pid)
{
    ifstream processImageFile;
    string processPid;
    int globalSymbolsTableStartAddress;
    int globalSymbolsTableEndAddress;
    int functionCallStackEndAddress;
    int functionCallStackStartAddress;
    int segmentSize;
    char uselessCharacter;
    int address;
    char fieldSeparator;
    string symbol;
    string value;
    size_t foundLowerCaseAddress;
    size_t foundUpperCaseAddress;
    size_t foundInstructionPointer;

    processPid = itos (pid) + "_backingstore";
    processImageFile.open (processPid.c_str ());
    if (!processImageFile)
    {
        cout << "HAL9000: unable to swap in process " << processPid << " image" << endl;
        CoreDump ();
        exit (1);
    }

    globalSymbolsTableStartAddress = GetMemorySegmentBoundary ("GLOBAL_SYMBOLS_TABLE_START_ADDRESS", segmentSize);
    globalSymbolsTableEndAddress = globalSymbolsTableStartAddress + segmentSize;
    functionCallStackEndAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_STACK_START_ADDRESS", segmentSize) + 1;
    functionCallStackStartAddress = functionCallStackEndAddress - segmentSize;

    ram.Clear (0);

    processImageFile >> uselessCharacter;
    while (processImageFile)
    {
        processImageFile >> uselessCharacter;
        processImageFile >> uselessCharacter;
        processImageFile >> address;
        ram.SetP (address);
        processImageFile >> fieldSeparator;
        processImageFile >> symbol;
        processImageFile.ignore (256, ':');
        foundLowerCaseAddress = symbol.find ("address");
        foundUpperCaseAddress = symbol.find ("ADDRESS");
        foundInstructionPointer = symbol.find ("INSTRUCTION_POINTER");
        if (foundLowerCaseAddress != string::npos ||
            foundUpperCaseAddress != string::npos ||
            foundInstructionPointer != string::npos)
        {
            processImageFile >> uselessCharacter;
            processImageFile >> uselessCharacter;
            processImageFile >> uselessCharacter;
        }
        else
        {
            if (symbol.length () > 0)
            {
                if (address >= globalSymbolsTableStartAddress && address < globalSymbolsTableEndAddress)
                {
                    processImageFile >> uselessCharacter;
                    processImageFile >> uselessCharacter;
                    processImageFile >> uselessCharacter;
                }
                else if (address >= functionCallStackStartAddress && address < functionCallStackEndAddress)
                {
                    processImageFile >> uselessCharacter;
                    processImageFile >> uselessCharacter;
                    processImageFile >> uselessCharacter;
                }
            }
        }
        getline (processImageFile, value);
        ram.Write (symbol, value);
        processImageFile >> uselessCharacter;
    }

    processImageFile.close ();

    return;
}

void ProcessImageToFile (int pid, string location)
{
    ofstream processImageFile;
    string processPid;
    memoryCell contents;
    int i;
    int globalSymbolsTableStartAddress;
    int globalSymbolsTableEndAddress;
    int functionCallStackEndAddress;
    int functionCallStackStartAddress;
    int segmentSize;

    processPid = itos (pid) + "_" + location;
    processImageFile.open (processPid.c_str ());
    if (!processImageFile)
    {
        cout << "HAL9000: unable to swap out process " << processPid << " image" << endl;
        CoreDump ();
        exit (1);
    }

    globalSymbolsTableStartAddress = GetMemorySegmentBoundary ("GLOBAL_SYMBOLS_TABLE_START_ADDRESS", segmentSize);
    globalSymbolsTableEndAddress = globalSymbolsTableStartAddress + segmentSize;
    functionCallStackEndAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_STACK_START_ADDRESS", segmentSize) + 1;
    functionCallStackStartAddress = functionCallStackEndAddress - segmentSize;

    for (i = 0; i < MEMORY_SIZE; i ++)
    {
        ram.SetP (i);
        contents = ram.Read ();
        if (contents.symbol.length () > 0)
        {
            processImageFile << "0d_";
            processImageFile << i << ": ";
            processImageFile << contents.symbol << " :";
            if (AddressField (contents.symbol, i,
                              globalSymbolsTableStartAddress,
                              globalSymbolsTableEndAddress,
                              functionCallStackStartAddress,
                              functionCallStackEndAddress))
            {
                processImageFile << "0d_";
            }
            processImageFile << contents.value << endl;
        }
    }
    processImageFile.close ();

    return;
}

bool AddressField (string symbol, int i, int globalSymbolsTableStartAddress, int globalSymbolsTableEndAddress,
                   int functionCallStackStartAddress, int functionCallStackEndAddress)
{
    size_t foundLowerCaseAddress;
    size_t foundUpperCaseAddress;
    size_t foundInstructionPointer;

    foundLowerCaseAddress = symbol.find ("address");
    foundUpperCaseAddress = symbol.find ("ADDRESS");
    foundInstructionPointer = symbol.find ("INSTRUCTION_POINTER");
    if (foundLowerCaseAddress != string::npos ||
        foundUpperCaseAddress != string::npos ||
        foundInstructionPointer != string::npos)
    {
        return true;
    }
    else
    {
        if (symbol.length () > 0)
        {
            if (i >= globalSymbolsTableStartAddress && i < globalSymbolsTableEndAddress)
            {
                return true;
            }
            else if (i >= functionCallStackStartAddress && i < functionCallStackEndAddress)
            {
                return true;
            }
        }
    }

    return false;
}

void AllocateLocalSymbol (memoryCell contents1)
{
    int functionCallValuesStackStartAddress;
    int topFunctionCallValuesStackAddress;
    int functionCallStackStartAddress;
    int topFunctionCallStackAddress;
    int segmentSize;
    memoryCell contents2;

    functionCallValuesStackStartAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_VALUES_STACK_START_ADDRESS", segmentSize);
    topFunctionCallValuesStackAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
    ram.SetP (topFunctionCallValuesStackAddress);
    if (ram.GetP () == functionCallValuesStackStartAddress - segmentSize)
    {
        cout << "HAL9000: function call values stack segmentation violation" << endl;
        CoreDump ();
        exit (1);
    }
    if (contents1.symbol == "constant")
    {
        ram.Push ("constant", "undefined_type");
    }
    else if (contents1.symbol == "variable")
    {
        ram.Push ("variable", "undefined_type");
    }
    else if (contents1.symbol == "file")
    {
        if (contents1.value == "keyboard")
        {
            ram.Push ("input_" + contents1.symbol, "closed");
        }
        else if (contents1.value == "display")
        {
            ram.Push ("output_" + contents1.symbol, "closed");
        }
        else
        {
            ram.Push (contents1.symbol, "closed");
        }
    }
    else // (contents1.symbol == "reference")
    {
        contents2 = ram.Pop ();
    }
    topFunctionCallValuesStackAddress = ram.GetP ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (topFunctionCallValuesStackAddress));
    
    functionCallStackStartAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_STACK_START_ADDRESS", segmentSize);
    topFunctionCallStackAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_STACK_ADDRESS");
    ram.SetP (topFunctionCallStackAddress);
    if (ram.GetP () == functionCallStackStartAddress - segmentSize)
    {
        cout << "HAL9000: function call stack segmentation violation" << endl;
        return;
    }
    if (contents1.symbol == "constant" ||
        contents1.symbol == "variable" ||
        contents1.symbol == "file")
    {
        ram.Push (contents1.value, itos (topFunctionCallValuesStackAddress));
    }
    else // (contents1.symbol == "reference")
    {
        ram.Push (contents1.value, contents2.value);
    }
    topFunctionCallStackAddress = ram.GetP ();
    SetKernelVariableValue ("TOP_FUNCTION_CALL_STACK_ADDRESS", itos (topFunctionCallStackAddress));

    if (contents1.symbol == "file")
    {
        if (contents1.value != "keyboard" && contents1.value != "display")
        {
            functionCallValuesStackStartAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_VALUES_STACK_START_ADDRESS", segmentSize);
            topFunctionCallValuesStackAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
            ram.SetP (topFunctionCallValuesStackAddress);
            if (ram.GetP () == functionCallValuesStackStartAddress - segmentSize)
            {
                cout << "HAL9000: function call values stack segmentation violation" << endl;
                CoreDump ();
                exit (1);
            }
            ram.Push ("file_name", "");
            topFunctionCallValuesStackAddress = ram.GetP ();
            SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (topFunctionCallValuesStackAddress));
        }
    }

    return;
}

void AssignTypeToLocalSymbol (memoryCell contents)
{
    string symbol;
    string type;

    symbol = contents.symbol;
    type = contents.value;

    ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_STACK_ADDRESS"));
    contents = ram.Read ();
    while (1)
    {
        if (contents.symbol.substr (0, 5) == "call_")
        {
            cout << "HAL9000: cannot set data type for undeclared local symbol " << symbol << endl;
            CoreDump ();
            exit (1);
        }
        if (contents.symbol == symbol)
        {
            ram.SetP (atoi (contents.value.c_str ()));
            contents = ram.Read ();
            if (contents.symbol == "constant" ||
                contents.symbol == "variable")
            {
                if (type == "integer")
                {
                    ram.ReWrite (contents.symbol + "_" + type, "0");
                }
                else if (type == "float")
                {
                    ram.ReWrite (contents.symbol + "_" + type, "0.0");
                }
                else if (type == "string")
                {
                    ram.ReWrite (contents.symbol + "_" + type, "");
                }
                else
                {
                    cout << "HAL9000: unrecognized data type " << type << " for local symbol " << symbol << endl;
                    CoreDump ();
                    exit (1);
                }
            }
            else if (contents.symbol == "file")
            {
                if (type == "input")
                {
                    ram.ReWrite (type + "_" + contents.symbol, "");
                }
                else if (type == "output")
                {
                    ram.ReWrite (type + "_" + contents.symbol, "");
                }
                else
                {
                    cout << "HAL9000: unrecognized data type " << type << " for global symbol " << symbol << endl;
                    exit (1);
                }
                ram.IterateDown ();
                contents = ram.Read ();
                ram.ReWrite (type + "_" + contents.symbol, "");
            }
            break;
        }
        ram.IterateUp ();
        contents = ram.Read ();
    }

    return;
}

void AllocateLocalArray (memoryCell contents)
{
    int functionCallValuesStackStartAddress;
    int topFunctionCallValuesStackAddress;
    int segmentSize;
    int noOfArrayElements = atoi (contents.value.c_str ());
    string symbol;
    int i;

    functionCallValuesStackStartAddress = GetMemorySegmentBoundary ("FUNCTION_CALL_VALUES_STACK_START_ADDRESS", segmentSize);
    topFunctionCallValuesStackAddress = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
    ram.SetP (topFunctionCallValuesStackAddress);
    if (ram.GetP () == functionCallValuesStackStartAddress - noOfArrayElements - segmentSize)
    {
        cout << "HAL9000: function call values stack segmentation violation" << endl;
        CoreDump ();
        exit (1);
    }
    contents = ram.Read ();
    symbol = contents.symbol;
    contents.symbol = contents.symbol + "_array_0";
    ram.ReWrite (contents.symbol, contents.value);
    
    for (i = 1; i < noOfArrayElements; i ++)
    {
        ram.SetP (GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS"));
        contents.symbol = symbol + "_array_" + itos (i);
        ram.Push (contents.symbol, contents.value);
        SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (ram.GetP ()));
    }
    
    return;
}

int GetKernelVariableIntegerValue (string kernelVariableDescription)
{
    int startAddress;
    int segmentSize;
    memoryCell contents;
    int kernelVariableValue;

    startAddress = GetMemorySegmentBoundary ("KERNEL_SPACE_START_ADDRESS", segmentSize);
    startAddress = startAddress + segmentSize;
    ram.SetP (startAddress);
    ram.IterateDown ();
    while (1)
    {
        contents = ram.Read ();
        if (contents.symbol == kernelVariableDescription)
        {
            break;
        }
        ram.IterateDown ();
    }
    kernelVariableValue = atoi (contents.value.c_str ());

    return (kernelVariableValue);
}

string GetKernelVariableStringValue (string kernelVariableDescription)
{
    int startAddress;
    int segmentSize;
    memoryCell contents;
    string kernelVariableValue;

    startAddress = GetMemorySegmentBoundary ("KERNEL_SPACE_START_ADDRESS", segmentSize);
    startAddress = startAddress + segmentSize;
    ram.SetP (startAddress);
    ram.IterateDown ();
    while (1)
    {
        contents = ram.Read ();
        if (contents.symbol == kernelVariableDescription)
        {
            break;
        }
        ram.IterateDown ();
    }
    kernelVariableValue = contents.value;

    return (kernelVariableValue);
}
    
void SetKernelVariableValue (string kernelVariableDescription, string value)
{
    int startAddress;
    int segmentSize;
    memoryCell contents;

    startAddress = GetMemorySegmentBoundary ("KERNEL_SPACE_START_ADDRESS", segmentSize);
    startAddress = startAddress + segmentSize;
    ram.SetP (startAddress);
    ram.IterateDown ();
    while (1)
    {
        contents = ram.Read ();
        if (contents.symbol == kernelVariableDescription)
        {
            break;
        }
        ram.IterateDown ();
    }
    ram.ReWrite ("", value);

    return;
}

void Shutdown ()
{
    cout << "HAL9000: powering down ..." << endl;
    usleep (SLEEP_DELAY);

    exit (0);
}

void Restart ()
{
    cout << "HAL9000: powering down ..." << endl;
    usleep (10 * SLEEP_DELAY);

    execle ("HALstart", "HALstart", (char *) NULL, environ);
    cout << "HALstart: HAL9000 failed to restart" << endl;
    CoreDump ();

    exit (1);
}

string GetDataType (string dataType)
{
    size_t foundDataType;

    foundDataType = dataType.find ("integer");
    if (foundDataType != string::npos)
    {
        return ("integer");
    }

    foundDataType = dataType.find ("float");
    if (foundDataType != string::npos)
    {
        return ("float");
    }

    foundDataType = dataType.find ("string");
    if (foundDataType != string::npos)
    {
        return ("string");
    }

    foundDataType = dataType.find ("input_file");
    if (foundDataType != string::npos)
    {
        return ("input_file");
    }

    foundDataType = dataType.find ("output_file");
    if (foundDataType != string::npos)
    {
        return ("output_file");
    }

    cout << "HAL9000: unrecognized data type " << dataType << endl;
    CoreDump ();
    exit (1);
}

bool IsInteger (string value)
{
    int i;

    for (i = 0; i < value.length (); i ++)
    {
        if (i == 0 && value [i] == '-')
        {
            continue;
        }
        else if (!isdigit (value [i]))
        {
            return false;
        }
    }

    return true;
}

bool IsFloat (string value)
{
    int i;
    bool decimalPointSeen = false;

    for (i = 0; i < value.length (); i ++)
    {
        if (i == 0 && value [i] == '-')
        {
            continue;
        }
        else if (value [i] == '.')
        {
            if (!decimalPointSeen)
            {
                decimalPointSeen = true;
                continue;
            }
            else
            {
                return false;
            }
        }
        else if (!isdigit (value [i]))
        {
            return false;
        }
    }

    return true;
}

string itos (int i)
{
    stringstream s;

    s << i;

    return s.str ();
}

string dtos (double d)
{
    stringstream s;

    s << d;

    return s.str ();
}

void SendMessageToHALos (string messageType, string parameter1, string parameter2, string parameter3,
                         string parameter4, string parameter5)
{
    static int seqNo = 0;
    static string fileNamePrefix = "HAL9000ToHALos_";
    string fileName;
    ofstream hal9000MessageFile;
    union sigval dummyValue;

    seqNo ++;
    fileName = fileNamePrefix + itos (seqNo);
    hal9000MessageFile.open (fileName.c_str ());
    if (!hal9000MessageFile)
    {
        cout << "HAL9000: unable to initialize return value buffer" << endl;
        exit (1);
    }
    hal9000MessageFile << messageType << endl;
    hal9000MessageFile << parameter1 << endl;
    hal9000MessageFile << parameter2 << endl;
    hal9000MessageFile << parameter3 << endl;
    hal9000MessageFile << parameter4 << endl;
    hal9000MessageFile << parameter5 << endl;
    
    hal9000MessageFile.close ();

    if (sigqueue (HALosPid, SIGRTMIN, dummyValue) == -1)
    {
        cout << "HAL9000: message ready signal not sent to HALos" << endl;
        exit (1);
    }

    if (seqNo == INT_MAX)
    {
        seqNo = 0;
    }

    return;
}

void Initialize ()
{
    cout << "HALbios: initializing ..." << endl;
    usleep (SLEEP_DELAY);

    cout << "HALbios: checking memory ..." << endl;
    InitializeMemory ();
    usleep (SLEEP_DELAY);

    cout << "HALbios: memory OK" << endl;
    usleep (SLEEP_DELAY);

    cout << "HALbios: loading HALos ..." << endl;
    if ((sigemptyset (&interruptMask) == -1) ||
        (sigaddset (&interruptMask, SIGRTMIN) == -1))
    {
        cout << "HALos: unable to initialize signal mask" << endl;
        CoreDump ();
        exit (1);
    }
    act.sa_sigaction = &SignalHandler;
    act.sa_mask = interruptMask;
    act.sa_flags = SA_SIGINFO;
    if ((sigemptyset (&act.sa_mask) == -1) ||
        (sigaction (SIGRTMIN, &act, NULL) == -1))
    {
        cout << "HALbios: problem loading HALos" << endl;
        CoreDump ();
        exit (1);
    }
    signal (SIGINT, SIG_IGN);
    usleep (SLEEP_DELAY);

    HALstartPid = getppid ();

    HALosPid = fork ();
    if (HALosPid < 0)
    {
        cout << "HALbios: HALos process creation failed" << endl;
        CoreDump ();
        exit (1);
    }
    else if (HALosPid == 0)
    {
        execle (HALos.c_str (), HALos.c_str (), (char *) NULL, environ);
        cout << "HALbios: HALos failed to load" << endl;
        CoreDump ();
        exit (1);
    }

	SetClockTicks();

    return;
}

void InitializeMemory ()
{
    GetMemorySegmentParameters ();
    SetMemorySegmentBoundaries ();
    SetKernelVariables ();

    SetKernelVariableValue ("QUANTUM_TIME_REMAINING", itos (QUANTUM_LENGTH));
    ProcessImageToFile (0, "template");

    return;
}

void GetMemorySegmentParameters ()
{
    ifstream memorySegmentsFile;
    string symbol;
    string value;
    int i;

    memorySegmentsFile.open ("HALmemoryVariables");
    if (!memorySegmentsFile)
    {
        cout << "HAL9000: unable to read memory segments file" << endl;
        CoreDump ();
        exit (1);
    }

    ram.ResetP ();
    memorySegmentsFile >> symbol;
    while (memorySegmentsFile)
    {
        ram.IterateUp ();
        memorySegmentsFile.ignore (256, ':');
        getline (memorySegmentsFile, value);
        ram.Write (symbol, value);
        memorySegmentsFile >> symbol;
    }
    memorySegmentsFile.close ();

    for (i = 0; i < MAX_BIOS_VARIABLES; i ++)
    {
        if (biosVariables [i].symbol.length () > 0)
        {
            ram.IterateUp ();
            ram.Write (biosVariables [i].symbol, biosVariables [i].value);
        }
    }

    return;
}

void SetMemorySegmentBoundaries ()
{
    int address;
    memoryCell contents;
    int i;

    address = 0;
    ram.ResetP ();
    ram.IterateUp ();
    ram.IterateUp ();

    for (i = 1; i <= 3; i ++)
    {
        contents = ram.Read ();
        address = address + atoi (contents.value.c_str ());
        ram.IterateUp ();
        ram.ReWrite ("", itos (address));
        ram.IterateUp ();
    }

    address = MEMORY_SIZE;

    ram.IterateUp ();
    for (i = 1; i <= 2; i ++)
    {
        ram.ReWrite ("", itos (address - 1));
        ram.IterateUp ();
        contents = ram.Read ();
        address = address - atoi (contents.value.c_str ());
        ram.IterateUp ();
    }

    return;
}

void SetKernelVariables ()
{
    ifstream kernelVariablesFile;
    int address;
    int segmentSize;
    string symbol;
    string value;
    int kernelVariableValue;

    kernelVariablesFile.open ("HALkernelVariables");
    if (!kernelVariablesFile)
    {
        cout << "HAL9000: unable to read kernel variables file" << endl;
        CoreDump ();
        exit (1);
    }

    address = GetMemorySegmentBoundary ("KERNEL_SPACE_START_ADDRESS", segmentSize);
    address = address + segmentSize;
    ram.SetP (address);
    kernelVariablesFile >> symbol;
    while (kernelVariablesFile)
    {
        ram.IterateDown ();
        kernelVariablesFile.ignore (256, ':');
        getline (kernelVariablesFile, value);
        ram.Write (symbol, value);
        kernelVariablesFile >> symbol;
    }
    kernelVariablesFile.close ();
    SetTableAndStackStartAddresses ();
    
    return;
}

void SetTableAndStackStartAddresses ()
{
    int address;
    int segmentSize;
    int kernelVariableValue;

    address = GetMemorySegmentBoundary ("GLOBAL_SYMBOLS_TABLE_START_ADDRESS", segmentSize);
    kernelVariableValue = GetKernelVariableIntegerValue ("LAST_GLOBAL_SYMBOLS_TABLE_ADDRESS");
    SetKernelVariableValue ("LAST_GLOBAL_SYMBOLS_TABLE_ADDRESS", itos (address - 1));

    address = GetMemorySegmentBoundary ("GLOBAL_VALUES_TABLE_START_ADDRESS", segmentSize);
    kernelVariableValue = GetKernelVariableIntegerValue ("LAST_GLOBAL_VALUES_TABLE_ADDRESS");
    SetKernelVariableValue ("LAST_GLOBAL_VALUES_TABLE_ADDRESS", itos (address - 1));

    address = GetMemorySegmentBoundary ("FUNCTION_CALL_STACK_START_ADDRESS", segmentSize);
    kernelVariableValue = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_STACK_ADDRESS");
    SetKernelVariableValue ("TOP_FUNCTION_CALL_STACK_ADDRESS", itos (address + 1));

    address = GetMemorySegmentBoundary ("FUNCTION_CALL_VALUES_STACK_START_ADDRESS", segmentSize);
    kernelVariableValue = GetKernelVariableIntegerValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS");
    SetKernelVariableValue ("TOP_FUNCTION_CALL_VALUES_STACK_ADDRESS", itos (address + 1));

    return;
}

int GetMemorySegmentBoundary (string segmentStartAddressDescription, int& segmentSize)
{
    int startAddress;
    memoryCell contents;

    ram.ResetP ();
    ram.IterateUp ();
    while (1)
    {
        contents = ram.Read ();
        if (contents.symbol == segmentStartAddressDescription)
        {
            break;
        }
        ram.IterateUp ();
    }
    startAddress = atoi (contents.value.c_str ());
    ram.IterateUp ();
    contents = ram.Read ();
    segmentSize = atoi (contents.value.c_str ());
    
    return startAddress;
}

static void SignalHandler (int signalNo, siginfo_t* info, void* context)
{
    if (signalNo == SIGRTMIN)
    {
        if (info -> si_pid == HALosPid)
        {
            somethingToExecute = 1;
        }
    }
}
