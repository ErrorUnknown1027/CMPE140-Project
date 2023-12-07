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
    string file = "r_type.dat";
    vector<instr> rom;
    pc = 0;
    loadInstr(&rom, file);
    for(int i = 0; i < 32; i++){//initialize the registers
        t[i] = 0; 
    }

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