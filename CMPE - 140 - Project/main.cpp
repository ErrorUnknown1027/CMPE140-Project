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

hashT h;

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
    //in.open(fileName);//open file
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
    //cout << "opcode in decimal : " << b2int(instruction.substr(25,7)) << endl;

    // cout << "funct3 : " << funct3 << endl;
    // cout << "rs1 : " << rs1 << endl;
    // cout << "rd : " << rd << endl;
    // cout << "immed : " << immed << endl;

    int r1, r2, data;
    r1 = stoi(rs1, nullptr, 2);
    r2 = stoi(rd, nullptr, 2);
    data = stoi(immed, nullptr, 2);
    if(immed[0] == '1'){//checking is signed
        data = data - pow(2, immed.length());
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
    immed = instruction.substr(0,7);
    funct3 = instruction.substr(17,3);
    rs1 = instruction.substr(12,5);
    rs2 = instruction.substr(7,5);
    rd = instruction.substr(20,5);

    int r1, r2, r3, data;
    r1 = stoi(rs1, nullptr, 2);
    r2 = stoi(rd, nullptr, 2);
    r3 = stoi(rs2, nullptr, 2);
    data = stoi(immed, nullptr, 2);

    if(immed[0] == '1'){//checking is signed
        data = data - pow(2, immed.length());
    }

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
        else if(immed == "0100000"){
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

    //convert to long
    long r1, r2;
    r1 = stol(rs1, nullptr, 2);
    r2 = stol(rs2, nullptr, 2);

    if(funct3 == "000"){//beq
        if(t[r1] == t[r2]){
            immed1 = signExtend(immed1 + "0");
            long data = stol(immed1, nullptr, 2);
            if(immed1[0] == '1'){//checking is signed
                data = data - pow(2, immed1.length());
            }
            pc = pc + data;
        }
    }
    else if(funct3 == "001"){//bne
        if(t[r1] != t[r2]){
            immed1 = signExtend(immed1 + "0");
            long data = stol(immed1, nullptr, 2);
            if(immed1[0] == '1'){//checking is signed
                data = data - pow(2, immed1.length());
            }
            pc = pc + data;
        }
    }
    else if(funct3 == "100"){//blt
        if(t[r1] < t[r2]){
            immed1 = signExtend(immed1 + "0");
            long data = stol(immed1, nullptr, 2);
            if(immed1[0] == '1'){//checking is signed
                data = data - pow(2, immed1.length());
            }
            pc = pc + data;
        }

    }
    else if(funct3 == "101"){//bge
        if(t[r1] >= t[r2]){
            immed1 = signExtend(immed1 + "0");
            long data = stol(immed1, nullptr, 2);
            if(immed1[0] == '1'){//checking is signed
                data = data - pow(2, immed1.length());
            }
            pc = pc + data;
        }
    }
    else if(funct3 == "110"){//bltu
        if((unsigned long)t[r1] < (unsigned long)t[r2]){
            immed1 = signExtend(immed1 + "0");
            long data = stol(immed1, nullptr, 2);
            if(immed1[0] == '1'){//checking is signed
                data = data - pow(2, immed1.length());
            }
            pc = pc + data;
        }
    }
    else if(funct3 == "111"){//bgeu
        if((unsigned long)t[r1] >= (unsigned long)t[r2]){
            immed1 = signExtend(immed1 + "0");
            long data = stol(immed1, nullptr, 2);
            if(immed1[0] == '1'){//checking is signed
                data = data - pow(2, immed1.length());
            }
            pc = pc + data;
        }
    }
}

/* LUI
Extract the bits imm[31:12] to form a 20-bit immediate value.
Left-shift the 20-bit immediate value by 12 bits.
Store the left-shifted immediate value in the destination register rd.
 */
void LUI(string instruction){
    string immed, rd;
    immed = instruction.substr(0, 20);// 7 bits
    string temp = signExtend(immed);
    rd = instruction.substr(7,5);// 5 bits
    long data = stol(immed, nullptr, 2);
    data = data<<12;//shifts over 12 bits (load upper immediate)
    if(immed[0] == '1'){//check if the number is negative in binary
        data = data - pow(2, immed.length());//sub from the largest possible negative number
    }
    long data_rd = stol(rd, nullptr, 2);//changes
    t[data_rd] = data;//put the data into the destination reg
    pc += 4;//program counter

}

/*AUIPC
Extract the bits imm[31:12] to form a 20-bit immediate value.
Left-shift the 20-bit immediate value by 12 bits.
Add the left-shifted immediate value to the current PC (program counter).
Store the result in the destination register rd.
 */
void AUIPC(string instruction){
    string immed, rd;
    immed = instruction.substr(0, 20);// 7 bits
    string temp = signExtend(immed);
    long data = stol(temp, nullptr, 2);
    rd = instruction.substr(7,5);// 5 bits
    if(immed[0] == '1'){//check if the number is negative in binary
        data = data - pow(2, immed.length());//sub from the largest possible negative number
    }
    data = data << 12;//left shit 12 bits
    long data_rd = stol(rd, nullptr, 2);
    t[data_rd] = pc + data;//add data to pc , then store
    pc +=4;

}


/* for JAL - chatgpt
Concatenate the bits imm[20], imm[10:1], imm[11], and imm[19:12] to form a 20-bit immediate value.
Sign-extend the 20-bit immediate value to 32 bits.
Add the sign-extended immediate value to the current PC (program counter) to compute the target address.
Store the address of the next instruction (PC + 4) in the destination register rd.
Update the PC to jump to the target address.
 */
void JAL(string instruction){
    cout << instruction << endl;
    string immed1, immed2, immed3, immed4, fullImmed, rd;
    immed1 = instruction.substr(0,1);// 1bits immed[20]
    immed2 = instruction.substr(1,10);// 9bits immed[10:1]
    immed3 = instruction.substr(11,1);// 1bits immed[11]
    immed4 = instruction.substr(12,8);// 7bits immed[19:12]
    fullImmed = immed1 + immed2 + immed3 + immed4;
    string temp = signExtend(fullImmed);
    rd = instruction.substr(7,5);// 5 bits for destination
    long data = stol(fullImmed, nullptr, 2);
    long data_rd = stol(rd, nullptr, 2);
    if(fullImmed[0] == '1'){//check if the number is negative in binary
        data = data - pow(2, fullImmed.length());//sub from the largest possible negative number
    }
    t[data_rd] = pc + 4;//this is to store the return address
    pc += data; // we then jump to the target address



}


/* for jalr (from chatgpt)
Concatenate the bits imm[11:0] and rs1 to form a 12-bit immediate value.
Sign-extend the 12-bit immediate value to 32 bits.
Add the sign-extended immediate value to the value in register rs1 to compute the target address.
Store the address of the next instruction (PC + 4) in the destination register rd.
Update the PC to jump to the target address.
*/

void JALR(string instruction){
    string immed, rd, funct3, rs1;
    immed = instruction.substr(0,12);// 1 bits
    funct3 = instruction.substr(12,3);// 3 bit
    rs1 = instruction.substr(15, 5);//5 bits
    string temp = signExtend(immed);
    rd = instruction.substr(7,5);// 5 bits
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
    else if(opcode == " "){//branch - type instructions
        bType(instruction);
    }
    else if(opcode == "0110111"){//lui instruction
        LUI(instruction);
    }
    else if(opcode == "0010111"){//auipc instruction
        AUIPC(instruction);
    }
    else if(opcode == "110111"){//jal instruction
        JAL(instruction);
    }
    else if(opcode == "1100111"){//jalr instruction
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
    cout << "register t(5) : " << t[30] << endl << endl << endl;
}

int main() {
    string file = "ldst.dat";
    vector<instr> rom;

    pc = 0;

    hashInit(&h);

    loadInstr(&rom, file);

    for(int i = 0; i < 32; i++){//initialize the registers
        t[i] = 0; 
    }
    
    cout << "instructions loaded" << endl;

    // for(int i = 0; i < rom.size(); i++){
    //     cout << "Instruction : " << rom[i].instruction << endl;
    //     cout << "At address : " << rom[i].address << "(" << stol(rom[i].address, nullptr, 2) << ")" << endl << endl;
    // }

    string command;

    //reset pc for instructions
    pc = 0;
    int count = 1;
    while(1){
        printCommands();
        cin >> command;

        if(command == "r"){
            for(int i = 0; i < rom.size(); i++){
            cout << "instruction : " << rom[count -1].instruction << endl;
            cout << "instruction #" << i+1 << "/" << rom.size() << endl;
            decode(rom[i].instruction);
            pc += 4;
            printReg();
            }
            break;
        }
        if(command == "s"){
            cout << "instruction : " << rom[count -1].instruction << endl;
            cout << "instruction # " << count << "/" << rom.size() << endl;
            decode(rom[count - 1].instruction);
            printReg();
            count++;
            pc += 4;
        }
        if(count > rom.size()){
            break;
        }
    }
    
    return 0;
}