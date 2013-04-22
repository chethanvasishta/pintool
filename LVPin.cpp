
#include <iostream>
#include "pin.H"
using namespace std;

#define MALLOC "malloc"
#define FREE "free"

unsigned int iCount = 0; //NEVER use globals

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
    cout<<"Instruction count :"<<iCount<<endl;
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

}


int main(int argc, char *argv[]){
    cout<<"My pin tool"<<endl;
    //let's start with the instruction count 
    PIN_Init(argc, argv); 
    //INS_AddInstrumentFunction(Instruction, 0);
    
    IMG_AddInstrumentFunction(Image,0);

    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();
    return 0;
}
