#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

using namespace std;

// Global Variables
unordered_map<int,int> Memory;

// processing data and instructions files
void readFile(string filename);
void readDataFile(string filename);
void removeEmptyLines();
vector<string> separateBranch(vector<string> instructions);
map<string, int> getBranchLocations(vector<string>& instructions);




int main()
{
    vector<string> Instructions;
    map<string, int>  Branche_Labels;







    return 0;
}

void readFile(string filename)
{
    vector<string> instructions;
    ifstream in;
    in.open(filename);
    if(!in.is_open()){
        cerr << "Could not open the file";
        exit(1);
    }

    string inst;
    while (!in.eof())
    {
        getline(in, inst);
        instructions.push_back(inst);
    }
}

void readDataFile(string filename)
{
    ifstream in;
    in.open(filename);

    int a = -1, v = -32769;				//To check if the file is empty or the address is not logical
    while (!in.eof())
    {   
        if (v != -32769 && a != -1)
        {
            in >> a >> v;
            Memory[a] = v;
        }
    }
    in.close();
}

void removeEmptyLines(vector<string> instructions)
{
	vector<string> temp = instructions;
	instructions.clear();
	for (int i = 0; i < temp.size(); i++)
	{
		if (temp[i] != "")
			instructions.push_back(temp[i]);
	}
}

vector<string> separateBranch(vector<string> instructions)   
{
    vector<string> temp = instructions;
    instructions.clear();
    string s = "", inst;
    string label = "";
    bool label_flag, space_flag;
    for (int i = 0; i < temp.size(); i++)
    {
        if (i > 0)
        {
            if (s != "")
                instructions.push_back(s);

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
                    instructions.push_back(label + ':');
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
        instructions.push_back(s);
    return instructions;
}

map<string, int> getBranchLocations(vector<string>& instructions)
{
    string label, inst;
    int num_branches = 0;
    map<string, int>  branches;
    vector<string> temp = instructions;
    instructions.clear();

    for (int i = 0; i < temp.size(); i++)
    {
        label = ""; 
        inst = temp[i];

        if (inst.find(':') != string::npos)
        {
            label = inst.substr(0, inst.find(':'));
            branches[label] = i - num_branches;
            num_branches += 1;
        }
        else
        {
            instructions.push_back(inst);
        }
    }
    return branches;
}