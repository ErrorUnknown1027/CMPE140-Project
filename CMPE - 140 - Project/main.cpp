#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>

using namespace std;

int t[32];//global array of registers

typedef struct node{
    string address;
    int data;
    node* next;
}node;

void nodeInit(){
    
}

const int ELEMENTS = 65536; //2^16 - 2^32 seems like a lot of space

int hash(string address){
    int index = stoi(address, nullptr, 2);
    return index % ELEMENTS;
}

typedef struct hashTable{// this will act as memory
    node* Table[ELEMENTS];
}hashTable;

void hashInit(){
    for(int i = 0; i < ELEMENTS; i++){

    }
}

void hashInsert(string Address, int data){

}

void Instructions(vector<string> *instr, string fileName){
    ifstream in(fileName);//open chosen file
    if(!in){//open check
        cerr << "Failed to open file" << endl;
        return;
    }

    string line, temp;
    int count = 0;//counter

    while(getline(in, line)){
        if(count < 4){//if less than 4 line pulls
            count++;//increment
            temp = line + temp;//add new 8 bits to the full instruction
        }
        if(count == 4){//at 4 pulls
            count = 0;//reset counter
            instr->push_back(temp);//add full instruction to the front of the vector
            temp.clear();//clear the string
        }
    }
    return;
}

void iType(string instruction){
    string funct3, rs1, rd, immed;
    //decode
    immed = instruction.substr(0,12);//12 bits
    funct3 = instruction.substr(17,3);// 3 bits
    rs1 = instruction.substr(12,5);//5 bits
    rd = instruction.substr(20,5);// 5 bits

    int r1, r2, data;
    r1 = stoi(rs1, nullptr, 2);// convert to int
    r2 = stoi(rd, nullptr, 2);//convert to int
    data = stoi(immed, nullptr, 2);//convert to int
    if(immed[0] == '1'){//check if the number is negative in binary
        data = data - pow(2, immed.length());//sub from the largest possible negative number
    }

    if(funct3 == "000"){//addi
        t[r2] = t[r1] + data;
    }
    else if(funct3 == "010"){//slti
        t[r2] = (t[r1] < data) ? 1 : 0;
    }
    else if(funct3 == "011"){//sltiu
        t[r2] = ((unsigned int)t[r1] < (unsigned int)data) ? 1 : 0;
    }
    else if(funct3 == "100"){//xori
        t[r2] = t[r1] ^ data;
    }
    else if(funct3 == "110"){//ori
        t[r2] = t[r1] | data;
    }
    else if(funct3 == "111"){//andi
        t[r2] = t[r1] & data;
    }
    else if(funct3 == "001"){//slli
        t[r2] = t[r1] << data;
    }
    else if(funct3 == "101"){//srli and srai
        if(immed[1] == '1'){//srai
            t[r2] = t[r1] >> data;
        }
        else{//srli
            t[r2] = (unsigned int)t[r1] >> data;
        }
    }
    
}

void rType(string instruction){
    string funct3, rs1, rs2, rd, immed;
    //decode
    immed = instruction.substr(0,7);// 7 bits
    funct3 = instruction.substr(17,3);// 3 bits
    rs1 = instruction.substr(12,5);// 5 bits
    rs2 = instruction.substr(7,5);// 5 bits
    rd = instruction.substr(20,5);// 5 bits

    int r1, r2, r3, data;
    r1 = stoi(rs1, nullptr, 2);// convert to int
    r2 = stoi(rd, nullptr, 2);// convert to int
    r3 = stoi(rs2, nullptr, 2);// convert to int
    //we dont convert the immed to an int since we aren't using it for any calcualtions

    if(funct3 == "000"){//add and sub
        if(immed == "0000000"){//add
            t[r2] = t[r1] + t[r3];
        }
        else if(immed == "0100000"){
            t[r2] = t[r1] - t[r3];
        }
    }
    else if(funct3 == "001"){//sll
        t[r2] = t[r1] << t[r3];
    }
    else if(funct3 == "010"){//slt
        t[r2] = (t[r1] < t[r3]) ? 1 : 0;
    }
    else if(funct3 == "011"){//sltu
        t[r2] = ((unsigned int)t[r1] < (unsigned int)t[r3]) ? 1 : 0;
    }
    else if(funct3 == "100"){//xor
        t[r2] = t[r1] ^ t[r3];
    }
    else if(funct3 == "101"){//srl and sra
        if(immed == "0000000"){//srl
            t[r2] = (unsigned int)t[r1] >> (unsigned int)t[r3];
        }
        else if(immed == "0100000"){//sra
            t[r2] = t[r1] >> t[r3];
        }
    }
    else if(funct3 == "110"){//or
        t[r2] = t[r1] | t[r3];
    }
    else if(funct3 == "111"){//and
        t[r2] = t[r1] & t[r3];
    }
}

void lType(string instruction){
    string funct3, rs1, rd, immed;
    //decode
    immed = instruction.substr(0,12);// 12 bits
    funct3 = instruction.substr(17,3);// 3 bits
    rs1 = instruction.substr(12,5);// 5 bits
    rd = instruction.substr(20,5);// 5 bits
}

void sType(string instruction){
    string funct3, rs1, rs2, immed, immed2;
    //decode
    immed = instruction.substr(0,7);// 7 bits
    immed2 = instruction.substr(20,5);// 5 bits
    funct3 = instruction.substr(17,3);// 3 bits
    rs1 = instruction.substr(12,5);// 5 bits
    rs2 = instruction.substr(7,5);// 5 bits
}

void decode(string instruction){
    //find opcode
    string opcode;
    opcode = instruction.substr(25,7);
    
    //determine the type of operation
    if(opcode == "0010011"){//i - type instructions
        //cout << "I type instruction" << endl;
        iType(instruction);
    }
    else if(opcode == "0110011"){//r - type instrucitons
        rType(instruction);
    }
    else if(opcode == "0000011"){//l - type instructions
        lType(instruction);
    }
    else if(opcode == "0100011"){//s - type instructions
        sType(instruction);
    }
}

int main() {
    string file = "r_type.dat";
    vector<string> instr;
    //cout << "input a file name" << endl;
    //cin >> file;
    //cout << file << endl;
    Instructions(&instr, file);
    for(int i = 0; i < 32; i++){//initialize the registers
        t[i] = 0; 
    }
    
    // for(int i = 0; i < instr.size(); i++){
    //     cout << instr[i] << " bits count : " << instr[i].length() << endl;
    // }

    //cout << endl << endl;

    for(int i = 0; i < instr.size(); i++){
        decode(instr[i]);

        for(int j = 0; j < 32; j++){
            cout << "register (" << j << ") : " << t[j] << endl;
        }
        cout << endl << endl;
    }

    return 0;
}