#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>

using namespace std;

int t[32];//global array of registers

typedef struct node{
    string address;//binary address from the instruction
    int data;//data to be stored
}node;

void nodeInit(node *n){
    n->address = "00000000";//set for default check
    n->data = 0;
}

const int ELEMENTS = 65536; //2^16 - 2^32 seems like a lot of space

int hashS(string address){
    int index = stoi(address, nullptr, 2);
    return index % ELEMENTS;
}

typedef struct hashTable{// this will act as memory
    node* Table[ELEMENTS];
}hashT;

void hashInit(hashT* h){
    for(int i = 0; i < ELEMENTS; i++){
        nodeInit(h->Table[i]);
    }
}

void hashInsert(string address, int data, hashT* h){
    int index = hashS(address) % ELEMENTS;
    
    while(h->Table[index]->address != "00000000"){//while this is already an entry at that location
        index++;
    }
    h->Table[index]->address = address;
    h->Table[index]->data = data;
}

int hashPull(string address, hashT* h){
    int index  = hashS(address) % ELEMENTS;
    while(h->Table[index]->address != address){
        index++;
    }
    return h->Table[index]->data;
}

hashT h;//global hashTable

string bin2str(int bin){
    string binary; 
    int bits = 32;
    for(int i = bits -1; i >= 0; i--){
        int bit = (bin >> i) & 1;
        binary += (bit + '0');
    }

    return binary;
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

    int r1, r2, data1;
    r1 = stoi(rs1, nullptr, 2);
    r2 = stoi(rd, nullptr, 2);
    data1 = stoi(immed, nullptr, 2);

    if(immed[0] == '1'){//check if the number is negative in binary
        data1 = data1 - pow(2, immed.length());//sub from the largest possible negative number
    }

    if(funct3 == "000"){//lb
        string address;
        string byte;
        int data;
        int add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        data = hashPull(address, &h);//pulls the data from that address
        byte = bin2str(data);//convert to a string
        t[r2] = stoi(byte.substr(byte.length()-8, 8), nullptr, 2);//convert the last 8 bits to int
    }
    else if(funct3 == "001"){//lh
        string address;
        string half;
        int data;
        int add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        data = hashPull(address, &h);//pulls the data from that address
        half = bin2str(data);//convert to a string
        t[r2] = stoi(half.substr(half.length()-16, 16), nullptr, 2);//convert the last 8 bits to int
    }
    else if(funct3 == "010"){//lw
        string address;
        string word;
        int data;
        int add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        data = hashPull(address, &h);//pulls the data from that address
        word = bin2str(data);//convert to a string
        t[r2] = stoi(word, nullptr, 2);//convert the last 8 bits to int
    }
    else if(funct3 == "100"){//lbu
        string address;
        string byte;
        int data;
        int add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        data = hashPull(address, &h);//pulls the data from that address
        byte = bin2str(data);//convert to a string
        t[r2] = (unsigned int)stoi(byte.substr(byte.length()-8, 8), nullptr, 2);//convert the last 8 bits to int
        
    }
    else if(funct3 == "101"){//lhu
        string address;
        string half;
        int data;
        int add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        data = hashPull(address, &h);//pulls the data from that address
        half = bin2str(data);//convert to a string
        t[r2] = (unsigned int)stoi(half.substr(half.length()-16, 16), nullptr, 2);//convert the last 8 bits to int
    }
}

void sType(string instruction){
    string funct3, rs1, rs2, immed1, immed2;
    //decode
    immed1 = instruction.substr(0,7);// 7 bits
    immed2 = instruction.substr(20,5);// 5 bits
    funct3 = instruction.substr(17,3);// 3 bits
    rs1 = instruction.substr(12,5);// 5 bits
    rs2 = instruction.substr(7,5);// 5 bits

    int r1, r2, data1, data2;
    r1 = stoi(rs1, nullptr, 2);
    r2 = stoi(rs2, nullptr, 2);
    data1 = stoi(immed1, nullptr, 2);
    data2 = stoi(immed2, nullptr, 2);

    if(immed1[0] == '1'){//check if the number is negative in binary
        data1 = data1 - pow(2, immed1.length());//sub from the largest possible negative number
    }
    if(immed2[0] == '1'){//check if the number is negative in binary
        data2 = data2 - pow(2, immed2.length());//sub from the largest possible negative number
    }

    if(funct3 == "000"){//sb - 8 bits
        string address;
        string byte;
        int add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        int store = t[r2] + data2;
        byte = bin2str(store);
        hashInsert(address, stoi(byte.substr(byte.length()-8, 8), nullptr, 2), &h);//puts the data to that address
    }
    else if(funct3 == "001"){//sh - 16 bits
        string address;
        string byte;
        int add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        int store = t[r2] + data2;
        byte = bin2str(store);
        hashInsert(address, stoi(byte.substr(byte.length()-16, 16), nullptr, 2), &h);//puts the data to that address
    }
    else if(funct3 == "010"){//sw - 32 bits
        string address;
        string byte;
        int add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        int store = t[r2] + data2;
        byte = bin2str(store);
        hashInsert(address, stoi(byte, nullptr, 2), &h);//puts the data to that address
    }
    
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
    string file = "ldst.dat";
    vector<string> instr;
    //cout << "input a file name" << endl;
    //cin >> file;
    //cout << file << endl;
    Instructions(&instr, file);
    for(int i = 0; i < 32; i++){//initialize the registers
        t[i] = 0; 
    }

    for(int i = 0; i < instr.size(); i++){
        decode(instr[i]);

        for(int j = 0; j < 32; j++){
            cout << "register (" << j << ") : " << t[j] << endl;
        }
        cout << endl << endl;
    }

    return 0;
}