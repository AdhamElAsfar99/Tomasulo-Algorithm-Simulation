#include "Tomasulo.hpp"


Tomasulo::Tomasulo(){

}



void Tomasulo::parseInstruction(string instruction){
   string instruction_name;
   size_t index = instruction.find(" "); // assuming no spaces before the instruction type
   string inst_type = instruction.substr(0, index);

   if(inst_type == "ADD" || inst_type == "NAND" || inst_type == "DIV"){
        parse_R_Instructions(instruction.substr(index));
   }

}



void Tomasulo::execute(){
    int address = pc/2;


    for(int i = 0; i < Instructions.size(); ++i){
        rA = rB = rC = imm_offset = "";

        string instruction_name;
        size_t index = Instructions[i].find(" ");                // assuming no spaces before the instruction type
        string inst_type = Instructions[i].substr(0, index);
        if(inst_type == "ADD" || inst_type == "NAND" || inst_type == "DIV"){
            parse_R_Instructions(Instructions[i].substr(index));
            execute_R_Instructions(inst_type);
        }
        else if(inst_type == "ADDI"){
            parse_I_Instructions(Instructions[i].substr(index));
            execute_I_Instructions(inst_type);
        }
        else if(inst_type == "LOAD" || inst_type == "STORE"){
            parse_LOAD_STORE_Instructions(Instructions[i].substr(index));
            execute_LOAD_STORE_Instructions(inst_type);
        }
        else if(inst_type == "BNE"){
            parse_Branch_Instructions(Instructions[i].substr(index));
            execute_Branch_Instructions(inst_type);
        }
        else if(inst_type == "CALL"){
            // TO BE IMPLEMENTED
        }
        else if(inst_type == "RET"){
            // TO BE IMPLEMENTED
        }
        else{
            cout << "Invalid Instruction" << endl;
            exit(1);
        }

    }
}

void Tomasulo::parse_R_Instructions(string instruction){
    int i = 0;
    while(instruction[i++] == ' ' || ',');

    rA = instruction[i] + instruction[i+1];

    i+=2;
    while(instruction[i++] == ' ' || ',');

    rB = instruction[i] + instruction[i+1];
    
    i+=2;
    while(instruction[i++] == ' ' || ',');

    rC = instruction[i] + instruction[i+1];
}

void Tomasulo::parse_I_Instructions(string instruction){
    int i = 0;
    while(instruction[i++] == ' ' || ',');

    rA = instruction[i] + instruction[i+1];

    i+=2;
    while(instruction[i++] == ' ' || ',');

    rB = instruction[i] + instruction[i+1];
    
    i+=2;
    while(instruction[i++] == ' ' || ',');

    // negative number
    if(instruction[i] == '-'){
        imm_sign = true;
        i++;
    }

    while(i < instruction.size() && isdigit(instruction[i])){
        imm_offset += instruction[i++];
    }
}

void Tomasulo::parse_LOAD_STORE_Instructions(string instruction){
    int i = 0;
    while(instruction[i++] == ' ' || ',');

    rA = instruction[i] + instruction[i+1];

    i+=2;
    while(instruction[i++] == ' ' || ',');

    // negative number
    if(instruction[i] == '-'){
        imm_sign = true;
        i++;
    }

    while(instruction[i] != '(' && instruction[i] != ' ' || ','){
        imm_offset += instruction[i++];
    }

    while(instruction[i] == ' '){
        i++;
    }

    i++;
    while(instruction[i++] != ')'){
        rB += instruction[i];
    }
}

void Tomasulo::parse_Branch_Instructions(string instruction){
    int i = 0;
    while(instruction[i++] == ' ' || ',');

    rA = instruction[i] + instruction[i+1];

    i+=2;
    while(instruction[i++] == ' ' || ',');

    rB = instruction[i] + instruction[i+1];
    
    i+=2;
    while(instruction[i++] == ' ' || ',');

    // negative number
    if(instruction[i] == '-'){
        imm_sign = true;
        i++;
    }

    while(i < instruction.size() && isdigit(instruction[i])){
        imm_offset += instruction[i++];
    }
}

void Tomasulo::execute_R_Instructions(string inst_type)
{
    int temp = INT_MAX;
    for (int i = 0; i < Reservation_Units.size(); i++)
    {
        if (cycle <= Reservation_Units[i].finish_time && Reservation_Units[i].name.find(inst_type) != string::npos)
        {
            temp = min(temp, Reservation_Units[i].finish_time + 1);
        }
        
    }
    cycle = temp;

    
    
}