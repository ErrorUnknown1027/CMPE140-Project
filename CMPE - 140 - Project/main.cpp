#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>

using namespace std;

long t[32];//global array of registers

typedef struct node{
    string address;//binary address from the instruction
    long data;//data to be stored
}node;

void nodeInit(node *n){
    n->address = "00000000";//set for default check
    n->data = 0;
}

const int ELEMENTS = 256; //2^16 - 2^32 seems like a lot of space

int hashS(string address){
    int index = stoi(address, nullptr, 2);
    return index % ELEMENTS;
}

typedef struct hashTable{// this will act as memory
    node* Table[ELEMENTS];
}hashT;

void hashInit(hashT* h){
    for(int i = 0; i < ELEMENTS; i++){
        h->Table[i] = new node;//allocate memory
        nodeInit(h->Table[i]);
    }
}

void hashInsert(string address, long data, hashT* h){
    int index = hashS(address) % ELEMENTS;
    
    while(h->Table[index]->address != "00000000"){//while this is already an entry at that location
        index++;
        if(index > ELEMENTS){
            index = 0;
        }
    }

    cout << "index : " << index << endl;
    cout << "address : " << address << endl;

    h->Table[index]->address = address;
    h->Table[index]->data = data;
}

long hashPull(string address, hashT* h){
    int index  = hashS(address) % ELEMENTS;
    cout << "index : " << index << endl;
    cout << "in hash pull" << endl;
    cout << "looking for address : " << address << endl;
    while(h->Table[index]->address != address){
        cout << "in while loop" << endl;
        index++;
        if(index > ELEMENTS){
            index = 0;
            //goto label;
        }
    }
    cout << "out of while loop" << endl;
    return h->Table[index]->data;
}

string bin2str(int bin){
    string binary; 
    int bits = 32;
    for(int i = bits -1; i >= 0; i--){
        int bit = (bin >> i) & 1;
        binary += (bit + '0');
    }

    return binary;
}

hashT h;

void Instructions(vector<string> *instr, string fileName){
    ifstream in(fileName);//open chosen file
    if(!in){//open check
        cerr << "Failed to open file" << endl;
        return;
    }

    string line, temp;
    int bits = 0;

    while(getline(in, line)){
        if(bits < 32){//if less than 4 line pulls
            bits += line.length();
            temp = line + temp;//add new 8 bits to the full instruction
        }
        if(temp.length() == 32){//at 4 pulls
            bits = 0;//reset counter
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

    long r1, r2, data;
    r1 = stol(rs1, nullptr, 2);// convert to int
    r2 = stol(rd, nullptr, 2);//convert to int
    data = stol(immed, nullptr, 2);//convert to int
    
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
        t[r2] = ((unsigned long)t[r1] < (unsigned long)data) ? 1 : 0;
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
            t[r2] = (unsigned long)t[r1] >> data;
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

    long r1, r2, r3, data;
    r1 = stol(rs1, nullptr, 2);// convert to int
    r2 = stol(rd, nullptr, 2);// convert to int
    r3 = stol(rs2, nullptr, 2);// convert to int
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
        t[r2] = ((unsigned long)t[r1] < (unsigned long)t[r3]) ? 1 : 0;
    }
    else if(funct3 == "100"){//xor
        t[r2] = t[r1] ^ t[r3];
    }
    else if(funct3 == "101"){//srl and sra
        if(immed == "0000000"){//srl
            t[r2] = (unsigned long)t[r1] >> (unsigned long)t[r3];
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

    long r1, r2, data1;
    r1 = stol(rs1, nullptr, 2);
    r2 = stol(rd, nullptr, 2);
    data1 = stol(immed, nullptr, 2);

    if(immed[0] == '1'){//check if the number is negative in binary
         data1 = data1 - pow(2, immed.length());//sub from the largest possible negative number
    }

    cout << data1 << endl;
    //long temp = data >> t[r1];
    cout << (t[r1] << data1) << endl;

    //00010000000000010000000000001100
    //10000000000010000000000001100000
    //00000010000000000010000000000001
    if(funct3 == "000"){//lb
        cout << "checking lb" << endl;
        string address;
        string byte;
        long data;
        long add = t[r1];//get address
        cout << "after shift" << endl;
        address = bin2str(add);//convert to a string
        cout << address << endl;
        data = hashPull(address, &h);//pulls the data from that address
        byte = bin2str(data);//convert to a string
        cout << byte << endl;
        t[r2] = stol(byte.substr(byte.length()-9, 8), nullptr, 2);//convert the last 8 bits to int
        cout << (t[r2] >> data1) << endl;
    }
    else if(funct3 == "001"){//lh
        cout << "checking lh" << endl;
        string address;
        string half;
        long data;
        long add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        data = hashPull(address, &h);//pulls the data from that address
        half = bin2str(data);//convert to a string
        t[r2] = stol(half.substr(half.length()-17, 16), nullptr, 2);//convert the last 8 bits to int
    }
    else if(funct3 == "010"){//lw
        cout << "in lw" << endl;
        string address;
        string word;
        long data;
        long add = t[r1] + data1;//get address
        cout << "got address" << endl;
        address = bin2str(add);//convert to a string
        cout << "got address : " << address << "lenght in bits : " << address.length() << endl; 
        data = hashPull(address, &h);//pulls the data from that address
        cout << "after pull" << endl;
        word = bin2str(data);//convert to a string
        cout << "checking lw" << endl;
        t[r2] = data;//convert the last 8 bits to int
        //cout << "put in data" << endl;
    }
    else if(funct3 == "100"){//lbu
        cout << "checking lbu" << endl;
        string address;
        string byte;
        long data;
        long add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        data = hashPull(address, &h);//pulls the data from that address
        byte = bin2str(data);//convert to a string
        t[r2] = (unsigned long)stol(byte.substr(byte.length()-9, 8), nullptr, 2);//convert the last 8 bits to int
        
    }
    else if(funct3 == "101"){//lhu
        cout << "checking lhu" << endl;
        string address;
        string half;
        long data;
        long add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        data = hashPull(address, &h);//pulls the data from that address
        half = bin2str(data);//convert to a string
        t[r2] = (unsigned long)stol(half.substr(half.length()-17, 16), nullptr, 2);//convert the last 8 bits to int
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

    long r1, r2, data1, data2;
    r1 = stol(rs1, nullptr, 2);
    r2 = stol(rs2, nullptr, 2);
    data1 = stol(immed1, nullptr, 2);
    data2 = stol(immed2, nullptr, 2);

    if(immed1[0] == '1'){//check if the number is negative in binary
        data1 = data1 - pow(2, immed1.length());//sub from the largest possible negative number
    }
    if(immed2[0] == '1'){//check if the number is negative in binary
        data2 = data2 - pow(2, immed2.length());//sub from the largest possible negative number
    }

    cout << "ADDRESS OFFSET : " << data2 << endl;
    
    if(funct3 == "000"){//sb - 8 bits
        string address;
        string byte;
        long add = t[r1] + data2;//get address
        address = bin2str(add);//convert to a string
        long store = t[r2] + data1;
        byte = bin2str(store);
        cout << "checking sb" << endl;
        hashInsert(address, stol(byte.substr(byte.length()-9, 8), nullptr, 2), &h);//puts the data to that address
    }
    else if(funct3 == "001"){//sh - 16 bits
        string address;
        string byte;
        long add = t[r1] + data2;//get address
        address = bin2str(add);//convert to a string
        long store = t[r2] + data1;
        byte = bin2str(store);
        cout << "check for sh" << endl;
        hashInsert(address, stol(byte.substr(byte.length()-17, 16), nullptr, 2), &h);//puts the data to that address
    }
    else if(funct3 == "010"){//sw - 32 bits
        string address;
        string byte;
        long add = t[r1] + data2;//get address
        address = bin2str(add);//convert to a string
        cout << "SW ADDRESS : " << address << endl;
        long store = t[r2] + data1;
        byte = bin2str(store);
        cout << "checking sw" << endl;
        hashInsert(address, store, &h);//puts the data to that address
    }
    
}

void bType(string instruction){
    string funct3, rs1, rs2, rd, immed1, immed2;
    //decode
    immed1 = instruction.substr(0,7);// 7 bits
    immed2 = instruction.substr(20,5);// 5 bits
    funct3 = instruction.substr(17,3);// 3 bits
    rs1 = instruction.substr(12,5);// 5 bits
    rs2 = instruction.substr(7,5);// 5 bits
    rd = instruction.substr(20, 5);// 5 bits

    if(funct3 == "000"){//beq
        if()
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

void printReg(){
    cout << "register t(0) : " << t[5] << endl;
    cout << "register t(1) : " << t[6] << endl;
    cout << "register t(2) : " << t[7] << endl;
    cout << "register t(3) : " << t[28] << endl;
    cout << "register t(4) : " << t[29] << endl;
    cout << "register t(5) : " << t[30] << endl << endl << endl;
}

int main() {
    string file = "ldst.dat";
    vector<string> instr;
    string command;
    //cout << "input a file name" << endl;
    //cin >> file;
    //cout << file << endl;

    hashInit(&h);

    Instructions(&instr, file);
    cout << "Instructions loaded" << endl;
    cout << "input command" << endl;
    cin >> command;

    for(int i = 0; i < 32; i++){//initialize the registers
        t[i] = 0; 
    }

    while(1){
        if(command == "r"){
            for(int i = 0; i < instr.size(); i++){
            cout << "instruction #" << i+1 << "/" << instr.size() << endl;
            decode(instr[i]);

            printReg();
            }

            break;
        }
        if(command )
    }

    cout << "finished instructions" << endl;

    return 0;
}