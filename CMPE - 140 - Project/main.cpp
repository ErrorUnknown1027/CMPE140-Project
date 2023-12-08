#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>

using namespace std;

int t[32];//global array of registers
long pc;

typedef struct instr{
    string instruction;//holds 32 bits of the instruction
    string address;//holds the address in a string form
}instr;

vector<instr> rom;//instruction memory

void instrInit(instr *i){
    i->address = "0";
    i->instruction = "0";
}

typedef struct node{
    string address;//binary address from the instruction
    long data;//data to be stored
}node;

void nodeInit(node *n){
    n->address = "00000000";//set for default check
    n->data = 0;
}

const int ELEMENTS = 512; //2^16 - 2^32 seems like a lot of space

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
    h->Table[index]->address = address;
    h->Table[index]->data = data;
}

long hashPull(string address, hashT* h){
    int index  = hashS(address) % ELEMENTS;
    while(h->Table[index]->address != address){
        index++;
        if(index > ELEMENTS){
            index = 0;
        }
    }
    return h->Table[index]->data;
}

hashT h;// global hash table

string bin2str(long bin){//converts any long into a 32 bit string
    string binary;
    int bits = 32;
    for(int i = bits -1; i >= 0; i--){
        int bit = (bin >> i) & 1;
        binary += (bit + '0');
    }
    return binary;
}

string signExtend(string bits){//sign extends any string of bits into a 32 bit string
    while(bits.length() < 32){
        bits = bits[0] + bits;
    }
    return bits;
}

void loadInstr(vector<instr> *rom, string fileName){
    ifstream in(fileName);
    if(!in){
        cerr << "Failed to open file" << endl;
        return;
    }
    string line, temp;
    instr* t = new instr;
    while(getline(in, line)){
        if(temp.length() < 32){
            temp = line + temp;
        }
        if(temp.length() == 32){
            t->address = bin2str(pc);
            t->instruction = temp;
            temp.clear();
            pc += 4;
            rom->push_back(*t);
        }
    }
}

void iType(string instruction){
    string funct3, rs1, rd, immed;
    //decode
    immed = instruction.substr(0,12);
    funct3 = instruction.substr(17,3);
    rs1 = instruction.substr(12,5);
    rd = instruction.substr(20,5);
    
    //convert to long
    long r1, r2, data;
    r1 = stol(rs1, nullptr, 2);
    r2 = stol(rd, nullptr, 2);
    data = stol(immed, nullptr, 2);
    if(immed[0] == '1'){//checking is signed
        data = data - pow(2, immed.length());
    }

    if(funct3 == "000"){//addi
        t[r2] = t[r1] + data;
        cout << "addi" << endl;
    }
    else if(funct3 == "010"){//slti
        t[r2] = (t[r1] < data) ? 1 : 0;
        cout << "slti" << endl;
    }
    else if(funct3 == "011"){//sltiu
        t[r2] = ((unsigned int)t[r1] < (unsigned int)data) ? 1 : 0;
        cout << "sltiu" << endl;
    }
    else if(funct3 == "100"){//xori
        t[r2] = t[r1] ^ data;
        cout << "xori" << endl;
    }
    else if(funct3 == "110"){//ori
        t[r2] = t[r1] | data;
        cout << "ori" << endl;
    }
    else if(funct3 == "111"){//andi
        t[r2] = t[r1] & data;
        cout << "andi" << endl;
    }
    else if(funct3 == "001"){//slli
        t[r2] = t[r1] << data;
        cout << "slli" << endl;
    }
    else if(funct3 == "101"){//srli and srai
        if(immed[1] == '1'){//srai
            t[r2] = t[r1] >> data;
            cout << "srai" << endl;
        }
        else{//srli
            t[r2] = (unsigned int)t[r1] >> data;
            cout << "srli" << endl;
        }
    }
    pc += 4;
}

void rType(string instruction){
    string funct3, rs1, rs2, rd, immed;
    //decode
    immed = instruction.substr(0,7);
    funct3 = instruction.substr(17,3);
    rs1 = instruction.substr(12,5);
    rs2 = instruction.substr(7,5);
    rd = instruction.substr(20,5);

    //convert to long
    int r1, r2, r3, data;
    r1 = stol(rs1, nullptr, 2);
    r2 = stol(rd, nullptr, 2);
    r3 = stol(rs2, nullptr, 2);
    data = stol(immed, nullptr, 2);

    if(immed[0] == '1'){//checking is signed
        data = data - pow(2, immed.length());
    }

    if(funct3 == "000"){//add and sub
        if(immed == "0000000"){//add
            t[r2] = t[r1] + t[r3];
            cout << "add" << endl;
        }
        else if(immed == "0100000"){//sub
            t[r2] = t[r1] - t[r3];
            cout << "sub" << endl;
        }
    }
    else if(funct3 == "001"){//sll
        t[r2] = t[r1] << t[r3];
        cout << "sll" << endl;
    }
    else if(funct3 == "010"){//slt
        t[r2] = (t[r1] < t[r3]) ? 1 : 0;
        cout << "slt" << endl;
    }
    else if(funct3 == "011"){//sltu
        t[r2] = ((unsigned long)t[r1] < (unsigned long)t[r3]) ? 1 : 0;
        cout << "sltu" << endl;
    }
    else if(funct3 == "100"){//xor
        t[r2] = t[r1] ^ t[r3];
        cout << "xor" << endl;
    }
    else if(funct3 == "101"){//srl and sra
        if(immed == "0000000"){//srl
            t[r2] = (unsigned long)t[r1] >> (unsigned long)t[r3];
            cout << "srl" << endl;
        }
        else if(immed == "0100000"){//sra
            t[r2] = t[r1] >> t[r3];
            cout << "sra" << endl;
        }
    }
    else if(funct3 == "110"){//or
        t[r2] = t[r1] | t[r3];
        cout << "or" << endl;
    }
    else if(funct3 == "111"){//and
        t[r2] = t[r1] & t[r3];
        cout << "and" << endl;
    }
    pc += 4;
}

void lType(string instruction){
    string funct3, rs1, rd, immed;
    //decode
    immed = instruction.substr(0,12);// 12 bits
    funct3 = instruction.substr(17,3);// 3 bits
    rs1 = instruction.substr(12,5);// 5 bits
    rd = instruction.substr(20,5);// 5 bits

    //convert to long
    long r1, r2, data1;
    r1 = stol(rs1, nullptr, 2);
    r2 = stol(rd, nullptr, 2);
    data1 = stol(immed, nullptr, 2);

    if(immed[0] == '1'){//check if the number is negative in binary
         data1 = data1 - pow(2, immed.length());//sub from the largest possible negative number
    }

    if(funct3 == "000"){//lb WE DONT CARE ABOUT THIS INSTRUCTION
        // string address;
        // string byte;
        // long data;
        // long add = t[r1];//get address
        // address = bin2str(add);//convert to a string
        // data = hashPull(address, &h);//pulls the data from that address
        // byte = bin2str(data);//convert to a string
        // t[r2] = stol(byte.substr(byte.length()-9, 8), nullptr, 2);//convert the last 8 bits to int
        cout << "lb" << endl;
    }
    else if(funct3 == "001"){//lh WE DONT CARE ABOUT THIS INSTRUCTION
        // string address;
        // string half;
        // long data;
        // long add = t[r1] + data1;//get address
        // address = bin2str(add);//convert to a string
        // data = hashPull(address, &h);//pulls the data from that address
        // half = bin2str(data);//convert to a string
        // t[r2] = stol(half.substr(half.length()-17, 16), nullptr, 2);//convert the last 8 bits to int
        cout << "lh" << endl;
    }
    else if(funct3 == "010"){//lw
        string address;
        string word;
        long data;
        long add = t[r1] + data1;//get address
        address = bin2str(add);//convert to a string
        data = hashPull(address, &h);//pulls the data from that address
        word = bin2str(data);//convert to a string
        t[r2] = data;//convert the last 8 bits to int
        cout << "lw" << endl;
    }
    else if(funct3 == "100"){//lbu WE DONT CARE ABOUT THIS INSTRUCTION
        // string address;
        // string byte;
        // long data;
        // long add = t[r1] + data1;//get address
        // address = bin2str(add);//convert to a string
        // data = hashPull(address, &h);//pulls the data from that address
        // byte = bin2str(data);//convert to a string
        // t[r2] = (unsigned long)stol(byte.substr(byte.length()-9, 8), nullptr, 2);//convert the last 8 bits to int
        cout << "lbu" << endl;
    }
    else if(funct3 == "101"){//lhu WE DONT CARE ABOUT THIS INSTRUCTION
        // string address;
        // string half;
        // long data;
        // long add = t[r1] + data1;//get address
        // address = bin2str(add);//convert to a string
        // data = hashPull(address, &h);//pulls the data from that address
        // half = bin2str(data);//convert to a string
        // t[r2] = (unsigned long)stol(half.substr(half.length()-17, 16), nullptr, 2);//convert the last 8 bits to int
        cout << "lhu" << endl;
    }
    pc += 4;
}

void sType(string instruction){
    string funct3, rs1, rs2, immed1, immed2;
    //decode
    immed1 = instruction.substr(0,7);// 7 bits
    immed2 = instruction.substr(20,5);// 5 bits
    funct3 = instruction.substr(17,3);// 3 bits
    rs1 = instruction.substr(12,5);// 5 bits
    rs2 = instruction.substr(7,5);// 5 bits

    //convert to long
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
    
    if(funct3 == "000"){//sb - 8 bits WE DONT CARE ABOUT THIS INSTRUCTION
        string address;
        string byte;
        long add = t[r1] + data2;//get address
        address = bin2str(add);//convert to a string
        long store = t[r2] + data1;
        byte = bin2str(store);
        hashInsert(address, stol(byte.substr(byte.length()-9, 8), nullptr, 2), &h);//puts the data to that address
        cout << "sb" << endl;
    }
    else if(funct3 == "001"){//sh - 16 bits WE DONT CARE ABOUT THIS INSTRUCTION
        string address;
        string byte;
        long add = t[r1] + data2;//get address
        address = bin2str(add);//convert to a string
        long store = t[r2] + data1;
        byte = bin2str(store);
        hashInsert(address, stol(byte.substr(byte.length()-17, 16), nullptr, 2), &h);//puts the data to that address
        cout << "sh" << endl;
    }
    else if(funct3 == "010"){//sw - 32 bits
        string address;
        string byte;
        long add = t[r1] + data2;//get address
        address = bin2str(add);//convert to a string
        long store = t[r2] + data1;
        byte = bin2str(store);
        hashInsert(address, store, &h);//puts the data to that address
        cout << "sw" << endl;
    }
    pc += 4;
}

void bType(string instruction){
    string funct3, rs1, rs2, rd, immed1, immed2, immed3, immed4;
    //decode
    immed1 = instruction.substr(19,1);//imm[12]
    immed2 = instruction.substr(21,6);//imm[10:5]
    immed3 = instruction.substr(27,5);//imm[4:1]
    immed4 = instruction.substr(20,1);//imm[11]
    funct3 = instruction.substr(17,3);// 3 bits
    rs1 = instruction.substr(12,5);// 5 bits
    rs2 = instruction.substr(7,5);// 5 bits

    //convert to long
    long r1, r2;
    r1 = stol(rs1, nullptr, 2);
    r2 = stol(rs2, nullptr, 2);

    if(funct3 == "000"){//beq
        cout << "beq" << endl;
        if(t[r1] == t[r2]){
            string temp = immed3 + immed4 + '0'; //'0' add a left shift
            long data = stol(temp, nullptr, 2);
            if(temp[0] == '1'){//checking is signed
                data = data - pow(2, temp.length());
            }
            pc = pc + data;
        }
        else{//else move on to the next instruction
            pc += 4;
        }
    }
    else if(funct3 == "001"){//bne
        cout << "bne" << endl;
        if(t[r1] != t[r2]){
            string temp = immed3 + immed4 + '0'; //'0' add a left shift
            long data = stol(temp, nullptr, 2);
            if(temp[0] == '1'){//checking is signed
                data = data - pow(2, temp.length());
            }
            pc = pc + data;
        }
        else{//else move on to the next instruction
            pc += 4;
        }
    }
    else if(funct3 == "100"){//blt
        cout << "blt" << endl;
        if(t[r1] < t[r2]){
            immed1 = signExtend(immed1 + "0"); //'0' add a left shift
            long data = stol(immed1, nullptr, 2);
            if(immed1[0] == '1'){//checking is signed
                data = data - pow(2, immed1.length());
            }
            pc = pc + data;
        }
        else{//else move on to the next instruction
            pc += 4;
        }
    }
    else if(funct3 == "101"){//bge
        cout << "bge" << endl;
        if(t[r1] >= t[r2]){
            string temp = immed3 + immed4 + '0'; //'0' add a left shift
            long data = stol(temp, nullptr, 2);
            if(temp[0] == '1'){//checking is signed
                data = data - pow(2, temp.length());
            }
            pc = pc + data;
        }
        else{//else move on to the next instruction
            pc += 4;
        }
    }
    else if(funct3 == "110"){//bltu
        cout << "bltu" << endl;
        if((unsigned long)t[r1] < (unsigned long)t[r2]){
            immed1 = signExtend(immed1 + "0"); //'0' add a left shift
            long data = stol(immed1, nullptr, 2);
            if(immed1[0] == '1'){//checking is signed
                data = data - pow(2, immed1.length());
            }
            pc = pc + data;
        }
        else{//else move on to the next instruction
            pc += 4;
        }
    }
    else if(funct3 == "111"){//bgeu
        cout << "bgeu" << endl;
        if((unsigned long)t[r1] >= (unsigned long)t[r2]){
            immed1 = signExtend(immed1 + "0"); //'0' add a left shift
            long data = stol(immed1, nullptr, 2);
            if(immed1[0] == '1'){//checking is signed
                data = data - pow(2, immed1.length());
            }
            pc = pc + data;
        }
        else{//else move on to the next instruction
            pc += 4;
        }
    }
}

void LUI(string instruction){
    string immed, rd;
    immed = instruction.substr(0, 20);// 7 bits
    string temp = signExtend(immed);
    rd = instruction.substr(20,5);// 5 bits
    long data = stol(immed, nullptr, 2);
    data = data<<12;//shifts over 12 bits (load upper immediate)
    if(immed[0] == '1'){//check if the number is negative in binary
        data = data - pow(2, immed.length());//sub from the largest possible negative number
    }
    long data_rd = stol(rd, nullptr, 2);//changes
    t[data_rd] = data;//put the data into the destination reg
    pc += 4;//program counter
}

void AUIPC(string instruction){
    string immed, rd;
    immed = instruction.substr(0, 20);// 7 bits
    string temp = signExtend(immed);
    long data = stol(temp, nullptr, 2);
    rd = instruction.substr(20,5);// 5 bits
    if(immed[0] == '1'){//check if the number is negative in binary
        data = data - pow(2, immed.length());//sub from the largest possible negative number
    }
    data = data << 12;//left shit 12 bits
    long data_rd = stol(rd, nullptr, 2);
    t[data_rd] = pc + data;//add data to pc , then store
    pc +=4;

}

void JALR(string instruction){
    string immed, rd, funct3, rs1;
    immed = instruction.substr(0,12);// 1 bits
    funct3 = instruction.substr(12,3);// 3 bit
    rs1 = instruction.substr(15, 5);//5 bits
    string temp = signExtend(immed);
    rd = instruction.substr(20,5);// 5 bits
    long data = stol(immed, nullptr, 2);
    long data_rd = stol(rd, nullptr, 2);
    long data_rs1 = stol(rs1, nullptr, 2);
    if(immed[0] == '1'){//check if the number is negative in binary
        data = data - pow(2, immed.length());//sub from the largest possible negative number
    }
    if(funct3 == "000"){
    t[data_rd] = pc + 4;
    pc = ((t[data_rs1] + data) & ~1); //jumps to the address the ~1 according to chat gpt is to ensure that the rightmost
                                      //bit is going to be always 0 so that it aligns with the target address to a word
    }

}

void JAL(string instruction);//function declaration

void decode(string instruction){
    //find opcode
    string opcode;
    opcode = instruction.substr(25,7);
    
    //determine the type of operation
    if(opcode == "0010011"){//i - type instructions
        cout << "i - type instruction" << endl;
        iType(instruction);
    }
    else if(opcode == "0110011"){//r - type instrucitons
        cout << "r - type instruction" << endl;
        rType(instruction);
    }
    else if(opcode == "0000011"){//l - type instructions
        cout << "l - type instruction" << endl;
        lType(instruction);
    }
    else if(opcode == "0100011"){//s - type instructions
        cout << "s - type instruction" << endl;
        sType(instruction);
    }
    else if(opcode == "1100011"){//branch - type instructions
        cout << "branch - type instruction" << endl;
        bType(instruction);
    }
    else if(opcode == "0110111"){//lui instruction
        cout << "lui instruction" << endl;
        LUI(instruction);
    }
    else if(opcode == "0010111"){//auipc instruction
        cout << "auipc instruction" << endl;
        AUIPC(instruction);
    }
    else if(opcode == "1101111"){//jal instruction
        cout << "jal instruction" << endl;
        JAL(instruction);
    }
    else if(opcode == "1100111"){//jalr instruction
        cout << "jalr instruction" << endl;
        JALR(instruction);
    }
}

void printCommands(){
    cout << "'r' runs the entire program in one go till it hits a breakpoint or exits" << endl;
    cout << "'s' runs the next instruction and then stops and waits for next command" << endl;
    cout << "'x0' to 'x31' return the contents of the register from the register file" << endl;
    cout << "Enter an address that starts with '0x' " << endl;
    cout << "'pc' returns the value fo the PC" << endl << endl; 
}

void printReg(){
    cout << "register t(0) : " << t[5] << endl;
    cout << "register t(1) : " << t[6] << endl;
    cout << "register t(2) : " << t[7] << endl;
    cout << "register t(3) : " << t[28] << endl;
    cout << "register t(4) : " << t[29] << endl;
    cout << "register t(5) : " << t[30] << endl;
    cout << "register t(6) : " << t[31] << endl << endl << endl;
}

int main() {
    string file = "line.dat";
    pc = 0;

    hashInit(&h);

    //m = -3 from dat
    string addressTemp = "00010000000000010000000000000000";
    hashInsert(addressTemp, -3,&h);

    //x = 7 from dat
    addressTemp = "00010000000000010000000000000100";
    hashInsert(addressTemp, 7,&h);

    //c = 25 from dat
    addressTemp = "00010000000000010000000000001000";
    hashInsert(addressTemp, 25,&h);

    //y = -1 from dat
    addressTemp = "00010000000000010000000000001100";
    hashInsert(addressTemp, -1,&h);

    loadInstr(&rom, file);

    for(int i = 0; i < 32; i++){//initialize the registers
        t[i] = 0; 
    }
    
    cout << "instructions loaded" << endl;

    string command;

    //reset pc for instructions
    pc = 0;

    int count = 1;
    while(1){
        printCommands();
        cin >> command;
        if(command == "r"){
            while(pc/4 < rom.size()){
            cout << "instruction : " << rom[pc/4].instruction << endl;
            cout << "instruction #" << pc/4 << "/" << rom.size() << endl;
            decode(rom[pc/4].instruction);
            t[0] = 0;
            printReg();
            }
            break;
        }
        if(command == "s"){
            cout << "instruction : " << rom[pc/4].instruction << endl;
            cout << "instruction # " << pc/4 << "/" << rom.size() << endl;
            decode(rom[pc/4].instruction);
            t[0] = 0;
            printReg();
        }
        if(pc/4 > rom.size()){
            break;
        }
    }
    return 0;
}

void JAL(string instruction){
    cout << instruction << endl;
    string immed1, immed2, immed3, immed4, fullImmed, rd;
    immed1 = instruction.substr(11,1);// 1bits signed bit
    immed2 = instruction.substr(1,10);// 10bits immed[10:1]
    immed3 = instruction.substr(20,1);// 1bits immed[11]
    immed4 = instruction.substr(12,8);// 7bits immed[19:12]
    cout << immed1 << endl;
    cout << immed2 << endl;
    cout << immed3 << endl;
    cout << immed4 << endl;
    fullImmed = immed1 + immed4 + immed2 + immed3;
    string temp = signExtend(fullImmed);//sign extend to 32 bits\
    //temp is the offset of the target address
    //rd is the return address
    rd = instruction.substr(20,5);// 5 bits for destination

    long data = stol(fullImmed, nullptr, 2);
    long data_rd = stol(rd, nullptr, 2);
    if(fullImmed[0] == '1'){//check if the number is negative in binary
        data = data - pow(2, fullImmed.length());//sub from the largest possible negative number
    }
    t[data_rd] = pc + 4;//this is to store the return address
    pc = pc + data; // we then jump to the target address
    cout << "data : " << data << endl;
    cout << temp << endl;
    cout << pc << endl; 
}