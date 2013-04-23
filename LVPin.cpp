
#include <iostream>
#include "pin.H"
#include "routine.h"
#include <fstream>      // std::ifstream, std::ofstream
#include <time.h>
#include <map>

using namespace std;

#define MALLOC "malloc"
#define FREE "free"

unsigned int iCount = 0; //NEVER use globals

time_t t1, t2;

class RoutineTimer{

public:
    void setStartTime(char* name, time_t start) { 
        m_timeMap.insert(pair<char*, time_t>(name, start));
    }

    void setEndTime(char *name, time_t end) {
        time_t start = m_timeMap[name];
        time_t diff = end - start;
        //if(m_cumulativeMap.find(name) != )
            m_cumulativeMap[name] += diff;            
        //else
         //   m_cumulativeMap.insert(pair<char*, time_t>(name, diff));
    }

    map<char*, time_t>& getCumulativeMap() { return m_cumulativeMap; }

private:
    map<char*, time_t> m_timeMap;
    map<char*, time_t> m_cumulativeMap;
};

RoutineTimer myTimer;

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
    time_t t;
    time(&t);
    myTimer.setStartTime(name, t);
}

VOID AfterCall(char* name, ADDRINT ret)
{
    time_t t;
    time(&t);
    myTimer.setEndTime(name, t);
}


void ProfileRoutine(RTN routine, void *v){
    RTN_Open(routine);
    const char *name = RTN_Name(routine).c_str();

    // Instrument malloc() to print the input argument value and the return value.
    RTN_InsertCall(routine, IPOINT_BEFORE, (AFUNPTR)BeforeCall,
            IARG_ADDRINT, name,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
            IARG_END);
    RTN_InsertCall(routine, IPOINT_AFTER, (AFUNPTR)AfterCall,
            IARG_ADDRINT, name,
            IARG_FUNCRET_EXITPOINT_VALUE, 
            IARG_END);

    RTN_Close(routine);
}

VOID RoutineFinish(INT32 code, VOID *v){
    std::ofstream outfile ("timer.txt", std::ofstream::binary);
    map<char*, time_t>& cMap = myTimer.getCumulativeMap();
    map<char*, time_t>::iterator iter = cMap.begin();
    for(; iter != cMap.end(); ++iter)
        outfile << iter->first << " " << iter->second << endl ;
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
