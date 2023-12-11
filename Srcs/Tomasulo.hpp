#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <stdlib.h>
#include <chrono>
#include <thread>

using namespace std;


struct ReservationUnit{
    bool busy;
    string op;
    int src1_value;
    int src2_value;
    string src1_reservation_unit;
    string src2_reservation_unit;
    int address;
    int finish_time;
    int issue_time;
    int instruction_number; // the index of the instruction in the instructions tracing vector
    int pc;
    int rB;
    int rC;
};

// map each register name(R0-R7) to its digit 
unordered_map<string, int> Register_to_Digit = {
    {"R0", 0},
    {"R1", 1},
    {"R2", 2},
    {"R3", 3},
    {"R4", 4},
    {"R5", 5},
    {"R6", 6},
    {"R7", 7}
};

struct Inst{
    string name;
    int rA, rB, rC, imm_offset;
};

struct tracing {
    string inst_name;
    int pc;
    int issue_cycle;
    int execution_start_cycle;
    int execution_end_cycle;
    int write_result_cycle;
};

class Tomasulo{
    int pc, cycle;
    int mispredictions, branches, instructions_completed;   // for reporting purposes
    int delay_between_cycles;

    bool store_stall;
    bool branch_flag;
    bool issue_stall;
    int pc_branch;
    
    unordered_map<int,int> Memory;
    unordered_map<string, int> Labels_Addresses;
    unordered_map<string,int> Cycles_Taken;
    unordered_map<string, ReservationUnit> Reservation_Units;
    

    vector<int> Registers;
    vector<Inst> Instructions;
    vector<string> literalInstructions; // insructions in string format
    vector<string> Register_to_ReservationUnit_Table;

    vector<tracing> Instructions_Tracing;
    
public:
    Tomasulo(vector<string>& Instructions, unordered_map<int,int>& Memory, unordered_map<string,int>& Labels_Addresses, int starting_address, int delay);     

    Inst parse_R_Instructions(string instruction);
    Inst parse_I_Instructions(string instruction);
    Inst parse_LOAD_STORE_Instructions(string instruction);
    Inst parse_Branch_Instructions(string instruction);
    Inst parse_CALL_Instruction(string instruction);
    Inst parse_RET_Instruction(string instruction);


    // void execute_R_Instructions(string inst_type);
    // void execute_I_Instructions(string inst_type);
    // void execute_LOAD_STORE_Instructions(string inst_type);
    // void execute_Branch_Instructions(string inst_type);

    void printInstruction(Inst inst);

    void execute(string curr_res_unit);
    void write();

    void runSimulation();

    void flushReservationStation(string res_unit);


    bool isFinished();

};