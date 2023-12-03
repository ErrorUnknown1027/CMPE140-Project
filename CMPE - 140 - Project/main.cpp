#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>

using namespace std;

int t[32];//global array of registers

int b2int(string binary){
    int total = 0;
    int power = binary.length() - 1;
    for(char digit : binary){
        if(digit == '1'){
            total += pow(2,power);
        }
        power--;
    }
    //cout << total << endl;
    return total;
}

void Instructions(vector<string> *instr, string fileName){
    ifstream in(fileName);
    //in.open(fileName);//open file
    if(!in){
        cerr << "Failed to open file" << endl;
        return;
    }

    string line, temp;
    int count = 0;

    while(getline(in, line)){
        if(count < 4){
            count++;
            temp = line + temp;
        }
        if(count == 4){
            count = 0;
            instr->push_back(temp);
            //cout << temp << " bit count : " << temp.length() << endl;
            temp.clear();
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
        data = data - 4096;
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

void decode(string instruction){
    //find opcode
    string opcode;
    opcode = instruction.substr(25,7);
    
    //determine the type of operation
    if(opcode == "0010011"){//i - type instructions
        //cout << "I type instruction" << endl;
        iType(instruction);
    }
}

int main() {
    string file = "addi_hazards.dat";
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