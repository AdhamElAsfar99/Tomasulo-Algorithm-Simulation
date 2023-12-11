#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "Tomasulo.cpp"

using namespace std;

vector<string> Instructions;
unordered_map<int,int> Memory;
unordered_map<string,int> Labels_Addresses;

// processing data and instructions files
void readFile(string filename);
void readDataFile(string filename);

void removeEmptyLines();
void getLabels_Addresses();
void seperateLabels();    // write a label on a different line than its assigned instruction, e.g. L1: \n ADD R1, R2, R3.


int main()
{
    cout << "This is a program to simulate the dynamic scheduling algorithm: Tomasulo's Algorithm\n";
    cout << "Please provide the filenames for the instructions and data (THEY SHOULD BE IN THE SAME DIRECTORY)\n";

    string instruction_fname, data_fname;
    cout << "Instructions filename: ";
    cin >> instruction_fname;
    cout << "Data filename: ";
    cin >> data_fname;


    int starting_address;
    cout << "Enter the starting address for your program: ";
    cin >> starting_address;

    if(starting_address < 0)
        starting_address = 0;
    

    int delay = 500;
    cout << "Our simulator prints the status of the program after each instruction and make sure to pause to allow "
    <<  "for the reader to grasp what happens so far. The default delay between cycles is 500 milliseconds, do you need to change it(Y/N)?";
    char ans;
    cin >> ans;

    if(ans == 'Y'){
        cout << "Enter the new dealy in milliseconds: ";
        cin >> delay;
    }



    readFile(instruction_fname);
    readDataFile(data_fname);

    // parse the instructions file
    removeEmptyLines();
    seperateLabels();
    getLabels_Addresses();
    
    Tomasulo t(Instructions, Memory, Labels_Addresses, starting_address, delay);
    t.runSimulation();

    return 0;
}

void readFile(string filename)
{
    ifstream in;
    in.open(filename);
    if(!in.is_open()){
        cerr << "Error: could not not open the file";
        exit(1);
    }

    string inst;
    while (!in.eof())
    {
        getline(in, inst);
        Instructions.push_back(inst);
    }
}

void readDataFile(string filename)
{
    ifstream in;
    in.open(filename);

    int a = -1, v = -32769;				//To check if the file is empty or the address is not logical
    while (!in.eof())
    {   
        in >> a >> v;
        if (v != -32769 && a != -1)
        {
            Memory[a] = v;
        }
    }
    in.close();
}

void removeEmptyLines()
{
	vector<string> temp = Instructions;
	Instructions.clear();
	for (int i = 0; i < temp.size(); i++)
	{
		if (temp[i] != "")
			Instructions.push_back(temp[i]);
	}
}

void seperateLabels()   
{
    vector<string> temp = Instructions;
    Instructions.clear();

    string s = "", inst;
    string label = "";
    bool label_flag, space_flag;
    for (int i = 0; i < temp.size(); i++)
    {
        if (i > 0)
        {
            if (s != "")
                Instructions.push_back(s);

        }
        s = "";
        label = "";

        inst = temp[i];

        label_flag = false; 
        space_flag = false;
        for (int j = 0; j < inst.size(); j++)
        {
            if (label_flag)
            {
                if (space_flag)
                {
                    s += inst[j];
                }
                else
                {
                    if (inst[j] == ' ')
                        continue;
                    else
                    {
                        s += inst[j];
                        space_flag = true;
                    }
                }
            }
            else
            {
                if (inst[j] == ':')
                {
                    Instructions.push_back(label + ':');
                    label_flag = true;
                    s = "";
                }
                else
                {
                    label += inst[j];
                    s += inst[j];
                }
            }
        }
    }
    if (s != "")
        Instructions.push_back(s);
    
}

void getLabels_Addresses()
{
    string label, inst;
    int num_labels = 0;
    vector<string> temp = Instructions;
    Instructions.clear();

    for (int i = 0; i < temp.size(); i++)
    {
        label = ""; 
        inst = temp[i];

        // there is a label on this line
        size_t colon_pos = inst.find(':');
        if (colon_pos != string::npos)
        {
            label = inst.substr(0, colon_pos);
            Labels_Addresses[label] = i - num_labels; // subtract the current address from the total labels encountered so far
            num_labels++;
        }
        else
        {
            Instructions.push_back(inst);
        }
    }
}