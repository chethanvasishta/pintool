
#include <iostream>
#include "pin.H"
#include "routine.h"
using namespace std;

#define MALLOC "malloc"
#define FREE "free"

unsigned int iCount = 0; //NEVER use globals

#include <fstream>      // std::ifstream, std::ofstream
#include <time.h>

time_t t1, t2;

/* execution time routine */
void docount() {    
    ++iCount;
}

VOID Arg1Before(CHAR * name, ADDRINT size)
{
    cout << name << "(" << size << ")" << endl;
}

VOID MallocAfter(ADDRINT ret)
{
    cout << "  returns " << ret << endl;
}

/* JIT time routine */
void Instruction(INS ins, void *v){
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);    
}

VOID Fini(INT32 code, VOID *v){
    std::ofstream outfile ("timeforadd.txt", std::ofstream::binary);
    outfile<<(t2 - t1)+10;
    outfile.close();
}

void startTimer(){
    time(&t1);
}

void endTimer(){
    time(&t2);
}

void Image(IMG img, void *v){
    // Instrument the malloc() and free() functions.  Print the input argument
    // of each malloc() or free(), and the return value of malloc().
    //
    //  Find the malloc() function.
    RTN mallocRtn = RTN_FindByName(img, MALLOC);
    if (RTN_Valid(mallocRtn))
    {
        RTN_Open(mallocRtn);

        // Instrument malloc() to print the input argument value and the return value.
        RTN_InsertCall(mallocRtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before,
                       IARG_ADDRINT, MALLOC,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_END);
        RTN_InsertCall(mallocRtn, IPOINT_AFTER, (AFUNPTR)MallocAfter,
                       IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

        RTN_Close(mallocRtn);
    }

    // Find the free() function.
    RTN freeRtn = RTN_FindByName(img, FREE);
    if (RTN_Valid(freeRtn))
    {
        RTN_Open(freeRtn);
        // Instrument free() to print the input argument value.
        RTN_InsertCall(freeRtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before,
                       IARG_ADDRINT, FREE,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_END);
        RTN_Close(freeRtn);
    }

    // Find the add() function.
    RTN addRtn = RTN_FindByName(img, "add");
    if (RTN_Valid(addRtn))
    {
        RTN_Open(addRtn);
        // Instrument free() to print the input argument value.
        RTN_InsertCall(addRtn, IPOINT_BEFORE, (AFUNPTR)startTimer,
                       IARG_END);
        RTN_InsertCall(addRtn, IPOINT_AFTER, (AFUNPTR)endTimer,
                       IARG_END);
        RTN_Close(addRtn);
    }
}

VOID BeforeCall(CHAR * name, ADDRINT size)
{
    cout << name << "(" << size << ")" << endl;
}

VOID AfterCall(ADDRINT ret)
{
    cout << "  returns " << ret << endl;
}


void ProfileRoutine(RTN routine, void *v){
    RTN_Open(routine);

    // Instrument malloc() to print the input argument value and the return value.
    RTN_InsertCall(routine, IPOINT_BEFORE, (AFUNPTR)BeforeCall,
            IARG_ADDRINT, "x",
            IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
            IARG_END);
    RTN_InsertCall(routine, IPOINT_AFTER, (AFUNPTR)AfterCall,
            IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

    RTN_Close(routine);
}

VOID RoutineFinish(INT32 code, VOID *v){
    std::ofstream outfile ("timeforadd.txt", std::ofstream::binary);
    outfile<<(t2 - t1)+10;
    outfile.close();
}

int main(int argc, char *argv[]){
    cout<<"My pin tool"<<endl;
    //let's start with the instruction count 
    PIN_Init(argc, argv); 
    //INS_AddInstrumentFunction(Instruction, 0);
    PIN_InitSymbols();
    
    //IMG_AddInstrumentFunction(Image,0);
    RTN_AddInstrumentFunction(ProfileRoutine, 0);

    PIN_AddFiniFunction(RoutineFinish, 0);
    PIN_StartProgram();
    return 0;
}
