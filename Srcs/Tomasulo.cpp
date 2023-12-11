#include "Tomasulo.hpp"


Tomasulo::Tomasulo(vector<string>& Instructions, unordered_map<int,int>& Memory, unordered_map<string, int>& Labels_Addresses, int starting_address, int delay){
    
    this->Memory = Memory;
    this->Labels_Addresses = Labels_Addresses;
    pc = starting_address;
    delay_between_cycles = delay;
    this->literalInstructions = Instructions;
    store_stall = false;
    branch_flag = false;
    instructions_completed = 0;
    mispredictions = 0;
    branches = 0;

    // populate the vector of instructions
    for(int i = 0; i < Instructions.size(); ++i){
        string instruction_name;
        size_t index = Instructions[i].find(" ");                // assuming no spaces before the instruction type
        string inst_type = Instructions[i].substr(0, index);


        if(inst_type == "ADD" || inst_type == "NAND" || inst_type == "DIV"){
            this->Instructions.push_back(parse_R_Instructions(Instructions[i]));
        }
        else if(inst_type == "ADDI"){
            this->Instructions.push_back(parse_I_Instructions(Instructions[i]));
        }
        else if(inst_type == "LOAD" || inst_type == "STORE"){
            this->Instructions.push_back(parse_LOAD_STORE_Instructions(Instructions[i]));
        }
        else if(inst_type == "BNE"){
            this->Instructions.push_back(parse_Branch_Instructions(Instructions[i]));
        }
        else if(inst_type == "CALL"){
            this->Instructions.push_back(parse_CALL_Instruction(Instructions[i]));
        }
        else if(inst_type == "RET"){
            this->Instructions.push_back(parse_RET_Instruction(Instructions[i]));
        }
        else{
            cout << "invalid instruction while parsing instructions" << '\n';
            exit(1);
        }
    }


    // initialize the reservation units
    // mark src1_value and src2_value with -32767 to indicate that they are not used
    Reservation_Units["LOAD1"] = {false, "LOAD", -32767, -32767, "", "", -1, -1, -1, -1, -1};
    Reservation_Units["LOAD2"] = {false, "LOAD", -32767, -32767, "", "", -1, -1, -1, -1, -1};
    Reservation_Units["STORE"] = {false, "STORE", -32767, -32767, "", "", -1, -1, -1, -1, -1};
    Reservation_Units["CALL/RET"] = {false, "", -1, -1, "", "", -1, -1, -1, -1, -1};
    Reservation_Units["BNE"] = {false, "BNE", -32767, -32767, "", "", -1, -1, -1, -1, -1};
    Reservation_Units["ADD/ADDI1"] = {false, "", -32767, -32767, "", "", -1, -1, -1, -1, -1};
    Reservation_Units["ADD/ADDI2"] = {false, "", -32767, -32767, "", "", -1, -1, -1, -1, -1};
    Reservation_Units["ADD/ADDI3"] = {false, "", -32767, -32767, "", "", -1, -1, -1, -1, -1};
    Reservation_Units["DIV"] = {false, "DIV", -32767, -32767, "", "", -1, -1, -1, -1, -1};
    Reservation_Units["NAND"] = {false, "NAND", -32767, -32767, "", "", -1, -1, -1, -1, -1};

    // initialize the registers
    Registers = vector<int>(8, 1);

    // report the number of cycles taken for each instruction
    Cycles_Taken["LOAD"] = 3;
    Cycles_Taken["STORE"] = 3;
    Cycles_Taken["CALL"] = 1;
    Cycles_Taken["RET"] = 1;
    Cycles_Taken["BNE"] = 1;
    Cycles_Taken["ADD"] = 2;
    Cycles_Taken["ADDI"] = 2;
    Cycles_Taken["DIV"] = 10;
    Cycles_Taken["NAND"] = 1;

    // initialize the register to reservation unit table
    Register_to_ReservationUnit_Table = vector<string>(8, "");

    issue_stall = false;
    pc_branch = 10000;

}



void Tomasulo::runSimulation(){

    std::chrono::milliseconds timespan(delay_between_cycles);

    string curr_res_unit;
    do{
        // check if we can issue the current instruction
        curr_res_unit = (!issue_stall ? "" : curr_res_unit);  // if stall is true, then we need to issue the same instruction again, so we keep the same reservation unit.
                                                        // if stall is false, then we need to issue a new instruction, so we set the reservation unit to "" to search for a new one

        
        // get the corrosponding reservation unit to the current instruction
        string inst_name;
        if(pc < Instructions.size()){
            inst_name = Instructions[pc].name;

            for(auto& res_unit: Reservation_Units){
                if(res_unit.first.find(inst_name) != string::npos && !res_unit.second.busy){
                    curr_res_unit = res_unit.first;
                    res_unit.second.busy = true;
                    res_unit.second.rB = Instructions[pc].rB;   // current instruction's rB
                    res_unit.second.rC = Instructions[pc].rC;   // current instruction's rC
                    res_unit.second.op = inst_name;
                    res_unit.second.instruction_number = Instructions_Tracing.size() + 1;
                    res_unit.second.issue_time = cycle;
                    res_unit.second.pc = pc;
                    break;  // since we are implementing single-issue, we can break here
                }
            }

            issue_stall = (curr_res_unit == "");  // if stall is true, then we need to issue the same instruction again, so we keep the same reservation unit.
                                            // if stall is false, then we need to issue a new instruction, so we set the reservation unit to "" to search for a new one

            if(curr_res_unit != ""){
                // mark the current instruction as issued in the instructions tracing vector
                tracing t;
                t.inst_name = literalInstructions[pc];
                t.pc = pc;
                t.issue_cycle = cycle;
                t.execution_start_cycle = -1;
                t.execution_end_cycle = -1;
                t.write_result_cycle = -1;

                Instructions_Tracing.push_back(t);


                // check if src registers are ready, avoiding RAW hazards
                if(Instructions[pc].rB != -1){
                    if(Register_to_ReservationUnit_Table[Instructions[pc].rB] == ""){
                        // the register is not mapped to any reservation unit
                        // so we can read its value from the register file
                        Reservation_Units[curr_res_unit].src1_value = Registers[Instructions[pc].rB];
                    }
                    else{
                        // the register is mapped to a reservation unit
                        // so we can read its value from the reservation unit
                        Reservation_Units[curr_res_unit].src1_reservation_unit = Register_to_ReservationUnit_Table[Instructions[pc].rB];
                    }
                }

                if(Instructions[pc].rC != -1){
                    if(Register_to_ReservationUnit_Table[Instructions[pc].rC] == ""){
                        // the register is not mapped to any reservation unit
                        // so we can read its value from the register file
                        Reservation_Units[curr_res_unit].src2_value = Registers[Instructions[pc].rC];
                    }
                    else{
                        // the register is mapped to a reservation unit
                        // so we can read its value from the reservation unit
                        Reservation_Units[curr_res_unit].src2_reservation_unit = Register_to_ReservationUnit_Table[Instructions[pc].rC];
                    }
                }

                if(Instructions[pc].imm_offset != -10000){
                    // the instruction is not I type
                    // so we can read the immediate value from the instruction
                    Reservation_Units[curr_res_unit].address = Instructions[pc].imm_offset;
                }

                // map the destination register to the reservation unit in the register to reservation unit table
                if(Instructions[pc].rA != -1 && !branch_flag)
                    Register_to_ReservationUnit_Table[Instructions[pc].rA] = curr_res_unit;
                
            }   
        }

        // check if we can execute any instruction in the reservation stations
        execute(curr_res_unit);

        // check if we can write the result of any instruction to the register file
        write();
            
        if(inst_name == "BNE" || inst_name == "CALL" || inst_name == "RET"){
            if(inst_name == "BNE")
                branches++;
            branch_flag = true;
            pc_branch = pc;
        }

        // print some information about the current cycle
        cout << "*************************************\n";
        cout << "Cycle: " << cycle << '\n';
        cout << "PC: " << pc << '\n';
        cout << "Branches: " << branches << '\n';
        cout << "Mispredictions: " << mispredictions << '\n';
        cout << "Instructions Completed: " << instructions_completed << '\n';
        cout << "*************************************\n";
        cout << "Instructions Tracing: \n";
        cout << "Instruction Name\tPC\tIssue Cycle\tExecution Start Cycle\tExecution End Cycle\tWrite Result Cycle\n";
        for(auto inst: Instructions_Tracing){
            cout << inst.inst_name << "\t\t" << inst.pc << "\t\t" << inst.issue_cycle << "\t\t" << inst.execution_start_cycle << 
            "\t\t" << inst.execution_end_cycle << "\t\t\t" << inst.write_result_cycle << '\n';
        }
        cout << "*************************************\n";
        // print the Registers to Reservation Unit Table
        cout << "Registers to Reservation Unit Table: \n";
        for(int i = 0; i < Register_to_ReservationUnit_Table.size(); ++i){
            cout << "R" << i << ": " << Register_to_ReservationUnit_Table[i] << '\n';
        }

        this_thread::sleep_for(timespan);
        cycle++;

    }while(!isFinished());

    // print statistics about the program
    // display the vector of instructions tracing in a table

    cout << "*************************************\n";
    cout << "Instructions Tracing: \n";
    cout << "Instruction Name\tPC\tIssue Cycle\tExecution Start Cycle\tExecution End Cycle\tWrite Result Cycle\n";
    for(auto inst: Instructions_Tracing){
        cout << inst.inst_name << "\t\t" << inst.pc << '\t' << inst.issue_cycle << "\t\t" << inst.execution_start_cycle << 
        "\t\t" << inst.execution_end_cycle << "\t\t" << inst.write_result_cycle << '\n';
    }
    cout << "Total Cycles: " << cycle << '\n';
    cout << "IPC: " << (Instructions.size())/((cycle+1) * 1.0) << '\n';
    if(branches != 0)
        cout << "Mispredictions perentage: " << (mispredictions)/(branches * 1.0);
    else
        cout << "No Branches encountered";
    cout << '\n';


}

// we need the curr_res_unit to flush it if control flow changes
void Tomasulo::execute(string curr_res_unit){

    // first we loop over reservation units to change the status of source registers if they are ready
    for(auto& res_unit: Reservation_Units){
        if(res_unit.second.src1_reservation_unit != "" && (Register_to_ReservationUnit_Table[res_unit.second.rB].empty() || Register_to_ReservationUnit_Table[res_unit.second.rB] == res_unit.first)){
            res_unit.second.src1_value = Registers[Instructions[res_unit.second.pc].rB];
        }

        if(res_unit.second.src2_reservation_unit != "" && (Register_to_ReservationUnit_Table[res_unit.second.rC].empty() || Register_to_ReservationUnit_Table[res_unit.second.rC] == res_unit.first)){
            res_unit.second.src2_value = Registers[Instructions[res_unit.second.pc].rC];
        }
    }

    bool pc_changed = false;
    // check all reservation units to see if any of them is ready to execute
    for(auto& res_unit: Reservation_Units){
        // Conditions for execution:
        // 1- the reservation unit is busy
        // 2- the src registers are ready
        // 3- the instruction is not issued in the current cycle
        // 4- the instruction is not finished yet

        if(res_unit.second.busy && res_unit.second.src1_value != -32767 && (res_unit.second.src2_value != -32767 || res_unit.second.address != -1) && res_unit.second.issue_time != cycle && res_unit.second.finish_time == -1){
            if(!branch_flag || res_unit.second.op == "BNE" || res_unit.second.op == "CALL" || res_unit.second.op == "RET" || res_unit.second.pc < pc_branch){
                // the reservation unit is ready to execute
                // so we can execute it
                res_unit.second.finish_time = cycle + Cycles_Taken[res_unit.second.op] - 1;
                if(Instructions[res_unit.second.pc].name == "STORE")
                    res_unit.second.finish_time -= Cycles_Taken[res_unit.second.op] - 2; // store only takes 1 cycle to execute, and the other cycles are used to write to memory
                
                // report the cycle number in the instructions tracing vector
                Instructions_Tracing[res_unit.second.instruction_number - 1].execution_start_cycle = cycle;
                Instructions_Tracing[res_unit.second.instruction_number - 1].execution_end_cycle = res_unit.second.finish_time;

                bool changedControlFlow = false;
                // check for bne, call, and ret instructions
                if(res_unit.first == "BNE"){
                    branch_flag = false;
                    if(res_unit.second.src1_value != res_unit.second.src2_value){
                        mispredictions++;
                        // branch taken
                        changedControlFlow = true;
                        pc_changed = true;
                        pc = res_unit.second.address;
                        for(auto& ru: Reservation_Units){
                            if(ru.second.issue_time > res_unit.second.issue_time){
                                flushReservationStation(ru.first);
                            }
                        }
                    }
                }
                else if(res_unit.first == "CALL/RET"){
                    branch_flag = false;
                    changedControlFlow = true;
                    pc_changed = true;
                    if(res_unit.second.op == "RET"){
                        // return instruction
                        pc = Registers[1];  // return address is stored in R1
                    }
                    else{
                        // call instruction
                        pc = res_unit.second.address;   // the address of the label to jump to is stored in src1_value
                    }
                }

                // if the control flow changed, we need to flush the @curr_res_unit, the field of rA in the registers to reservations unit, and the last instruction in the instructions tracing vector
                if(changedControlFlow && curr_res_unit != ""){
                    Reservation_Units[curr_res_unit].busy = false;
                    Reservation_Units[curr_res_unit].src1_value = -32767;
                    Reservation_Units[curr_res_unit].src2_value = -32767;
                    Reservation_Units[curr_res_unit].src1_reservation_unit = "";
                    Reservation_Units[curr_res_unit].src2_reservation_unit = "";
                    Reservation_Units[curr_res_unit].finish_time = -1;
                    Reservation_Units[curr_res_unit].instruction_number = -1;

                    Register_to_ReservationUnit_Table[Instructions[pc].rA] = "";
                    // remove the second last instruction from the instructions tracing vector
                    Instructions_Tracing.pop_back();
                }

                break;
            }
        }
    }
    if(!pc_changed && !issue_stall)
        pc++;
}

void Tomasulo::write(){
    // check all reservation units to see if any of them is ready to write to the register file or memory
    // only one instruction can write to the register file or memory in each cycle

    if(store_stall){
        // we can't write to memory in this cycle, which is the second cycle of the store instruction write
        store_stall = false;
        return;
    }
    for(auto& res_unit: Reservation_Units){
        if(res_unit.second.busy && res_unit.second.finish_time != -1 && res_unit.second.finish_time + 1 <= cycle && !store_stall){
            
            bool donot_write = false;
            // check if there is a destination register mapped to this reservation unit in the register to reservation unit table
            // if not, then the instruction is a store instruction, so we can write its result to memory
            // if yes, then the instruction is not a store instruction, so we can write its result to the register file
            // The first condition avoids WAW hazards
            if(Register_to_ReservationUnit_Table[Instructions[res_unit.second.pc].rA] != res_unit.first && res_unit.second.op != "STORE")
                donot_write = true;
            
            if(res_unit.second.op == "STORE")
                store_stall = true;
            

            // the reservation unit is ready to write
            // so we can write its result to the register file
            if(!donot_write){
                switch(res_unit.second.op[0]){
                    case 'L':   // LOAD
                        Registers[Instructions[res_unit.second.pc].rA] = Memory[res_unit.second.address + res_unit.second.src1_value];
                        break;
                    case 'S':   // STORE
                        Memory[res_unit.second.address + res_unit.second.src2_value] = Registers[Instructions[res_unit.second.pc].rA];
                        break;
                    case 'C':   // CALL/RET - only call instruction writes to the register file
                        if(res_unit.second.src1_value != 1){
                            // call instruction
                            Registers[1] = pc + 1;   // the address of the next instruction is stored in R1
                        }
                        break;
                    case 'B':   // BNE
                        // do nothing
                        break;
                    case 'N':   // NAND
                        Registers[Instructions[res_unit.second.pc].rA] = ~(res_unit.second.src1_value & res_unit.second.src2_value);
                        break;
                    case 'D':   // DIV
                        Registers[Instructions[res_unit.second.pc].rA] = res_unit.second.src1_value / res_unit.second.src2_value;
                        break;
                    case 'A':   // ADD
                        if(res_unit.second.op == "ADDI")
                            Registers[Instructions[res_unit.second.pc].rA] = res_unit.second.src1_value + res_unit.second.address;
                        else
                            Registers[Instructions[res_unit.second.pc].rA] = res_unit.second.src1_value + res_unit.second.src2_value;
                        break;
                    default:
                        cout << "Invalid Instruction While Writing" << endl;
                        exit(1);
                }
            }
            // report the cycle number in the instructions tracing vector
            Instructions_Tracing[res_unit.second.instruction_number - 1].write_result_cycle = cycle;
            if(res_unit.second.op == "STORE")
                Instructions_Tracing[res_unit.second.instruction_number - 1].write_result_cycle++;

            // remove the instruction from the reservation unit
            res_unit.second.busy = false;
            res_unit.second.src1_value = -32767;
            res_unit.second.src2_value = -32767;
            res_unit.second.src1_reservation_unit = "";
            res_unit.second.src2_reservation_unit = "";
            res_unit.second.finish_time = -1;
            res_unit.second.instruction_number = -1;
            res_unit.second.rB = -1;
            res_unit.second.rC = -1;
            for(int i = 0; i < 8; ++i){
                if(Register_to_ReservationUnit_Table[i] == res_unit.first)
                    Register_to_ReservationUnit_Table[i] = "";
            }

            instructions_completed++;
            break;
        }
    }
}
Inst Tomasulo::parse_R_Instructions(string instruction){
    string name, rA, rB, rC;
    name = rA = rB = rC = "";

    int i = 0;
    while(instruction[i++] == ' '); // skip spaces
    i--;

    while(instruction[i] != ' '){
        name += instruction[i++];
    }

    while(instruction[i++] == ' ');
    i--;

    rA += instruction[i];
    rA += instruction[i+1];

    i+=2;
    while(instruction[i] == ' ' || instruction[i] == ','){
        i++;
    }

    rB += instruction[i];
    rB += instruction[i+1];

    i+=2;
    while(instruction[i] == ' ' || instruction[i] == ','){
        i++;
    }

    rC = instruction[i];
    rC += instruction[i+1];

    return {name, Register_to_Digit[rA], Register_to_Digit[rB], Register_to_Digit[rC], -33};    // range of imm is [-32, 31], so we use -33 to indicate that the instruction is not I type
}

// ADDI R1, R2, 10

Inst Tomasulo::parse_I_Instructions(string instruction){
    string name, rA, rB, imm_offset;
    rA = rB = imm_offset = "";
    bool imm_sign = false;


    int i = 0;
    while(instruction[i++] == ' '); // skip spaces
    i--;

    while(instruction[i] != ' '){
        name += instruction[i++];
    }
    
    while(instruction[i++] == ' ');
    i--;

    rA += instruction[i];
    rA += instruction[i+1];

    i+=2;
    while(instruction[i] == ' ' || instruction[i] == ','){
        i++;
    }

    rB += instruction[i];
    rB += instruction[i+1];
    
    i+=2;
    while(instruction[i] == ' ' || instruction[i] == ','){
        i++;
    }


    // negative number
    if(instruction[i] == '-'){
        imm_sign = true;
        i++;
    }

    while(i < instruction.size() && isdigit(instruction[i])){
        imm_offset += instruction[i++];
    }
    // -2^15 - 1 = -32767
    return {name, Register_to_Digit[rA], Register_to_Digit[rB], -1, stoi(imm_offset) * (imm_sign ? -1 : 1)}; 
}

Inst Tomasulo::parse_LOAD_STORE_Instructions(string instruction){
    string name, rA, rB, imm_offset;
    rA = rB = imm_offset = "";
    bool imm_sign = false;

    int i = 0;
    while(instruction[i++] == ' '); // skip spaces
    i--;

    while(instruction[i] != ' '){
        name += instruction[i++];
    }

    while(instruction[i++] == ' ');
    i--;

    rA += instruction[i];
    rA += instruction[i+1];

    i+=2;
    while(instruction[i] == ' ' || instruction[i] == ','){
        i++;
    }

    // negative number
    if(instruction[i] == '-'){
        imm_sign = true;
        i++;
    }

    while(instruction[i] != '(' && instruction[i] != ' '){
        imm_offset += instruction[i++];
    }

    while(instruction[i++] == ' ');


    while(instruction[i] != ')' && instruction[i] != ' '){
        rB += instruction[i++];
    }

    if(name == "LOAD")
        return {name, Register_to_Digit[rA], Register_to_Digit[rB], -1, stoi(imm_offset) * (imm_sign ? -1 : 1)};
    else
        return {name, -1, Register_to_Digit[rA], Register_to_Digit[rB], stoi(imm_offset) * (imm_sign ? -1 : 1)};
}

Inst Tomasulo::parse_Branch_Instructions(string instruction){
    string name, rA, rB, label;
    rA = rB = "";

    int i = 0;
    while(instruction[i++] == ' '); // skip spaces
    i--;

    while(instruction[i] != ' '){
        name += instruction[i++];
    }

    while(instruction[i++] == ' ');
    i--;

    rA += instruction[i];
    rA += instruction[i+1];

    i+=2;
    while(instruction[i] == ' ' || instruction[i] == ','){
        i++;
    }

    rB += instruction[i];
    rB += instruction[i+1];

    i+=2;
    while(instruction[i] == ' ' || instruction[i] == ','){
        i++;
    }

    while(i < instruction.size() && instruction[i] != ' '){
        label += instruction[i++];
    }

    return {name, -1, Register_to_Digit[rA], Register_to_Digit[rB], Labels_Addresses[label]};
}

Inst Tomasulo::parse_CALL_Instruction(string instruction){
    string name, label;

    int i = 0;
    while(instruction[i++] == ' '); // skip spaces
    i--;

    while(instruction[i] != ' '){
        name += instruction[i++];
    }

    while(instruction[i++] == ' ');
    i--;

    while(i < instruction.size() && instruction[i] != ' '){
        label += instruction[i++];
    }

    return {name, -1, -1, -1, Labels_Addresses[label]};

}

Inst Tomasulo::parse_RET_Instruction(string instruction){
    string name;

    int i = 0;
    while(instruction[i++] == ' '); // skip spaces
    i--;

    while(i < instruction.size() && instruction[i] != ' '){
        name += instruction[i++];
    }

    return {name, -1, 1, -1, -33};  // src1_value = 1 to make it easier to check if the return address is ready, aka Reg1 is ready
}


bool Tomasulo::isFinished(){
    // check if all reservation units are free, so we can terminate the program
    for(auto res_unit: Reservation_Units){
        if(res_unit.second.busy)
            return false;
    }
    return true;
}

void Tomasulo::flushReservationStation(string res_unit){
    // remove the instruction from the reservation unit
    Reservation_Units[res_unit].busy = false;
    Reservation_Units[res_unit].src1_value = -32767;
    Reservation_Units[res_unit].src2_value = -32767;
    Reservation_Units[res_unit].src1_reservation_unit = "";
    Reservation_Units[res_unit].src2_reservation_unit = "";
    Reservation_Units[res_unit].finish_time = -1;
    Reservation_Units[res_unit].instruction_number = -1;
    Reservation_Units[res_unit].rB = -1;
    Reservation_Units[res_unit].rC = -1;
    for(int i = 0; i < 8; ++i){
        if(Register_to_ReservationUnit_Table[i] == res_unit)
            Register_to_ReservationUnit_Table[i] = "";
    }

    instructions_completed++;
}