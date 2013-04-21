
#include <iostream>
#include "pin.H"
using namespace std;

unsigned int iCount = 0; //NEVER use globals

/* execution time routine */
void docount() {    
    ++iCount;
}

/* JIT time routine */
void Instruction(INS ins, void *v){
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);    
}

VOID Fini(INT32 code, VOID *v){
    cout<<"Instruction count :"<<iCount<<endl;
}


int main(int argc, char *argv[]){
    cout<<"My pin tool"<<endl;
    //let's start with the instruction count 
    PIN_Init(argc, argv); 
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();
    return 0;
}
