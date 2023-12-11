#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <stdlib.h>
using namespace std;

struct ReservationUnit{
    string name;
    bool busy;
    string op;
    int src1_value;
    int src2_value;
    string src1_reservation_unit;
    string src2_reservation_unit;
    int address;
    int finish_time;
};

class Tomasulo{
    int pc;
    string rA, rB, rC, imm_offset;
    bool imm_sign;

    int cycle;
    
    unordered_map<int,int> Memory;
    unordered_map<string, int> Labels_Addresses;
    unordered_map<string,int> Cycles_Taken;
    
    vector<int> Registers;
    vector<string> Instructions;
    vector<ReservationUnit> Reservation_Units;
    vector<string> Register_to_ReservationUnit_Table;


    vector<vector<int>> Instructions_Tracing;
    
public:
    // take the instructions and memory files as well as the starting address
    Tomasulo(); 

    void parse();
    

    void parseInstruction(string instruction);
    void parse_R_Instructions(string instruction);
    void parse_I_Instructions(string instruction);
    void parse_LOAD_STORE_Instructions(string instruction);
    void parse_Branch_Instructions(string instruction);


    void execute_R_Instructions(string inst_type);
    void execute_I_Instructions(string inst_type);
    void execute_LOAD_STORE_Instructions(string inst_type);
    void execute_Branch_Instructions(string inst_type);

    void execute();




};