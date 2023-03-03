#include <iostream>
#include <string>
#include <fstream> 
#include <vector> 
#include <algorithm>
#include <bitset>
#include <cstring>
#include <map>  

using namespace std;  
//function declaration
void compressed(string ToCompressFilename); 
void decompressed(string ToDecompressFilename); 
//main method
int main (int argc, char** argv){
    //1 to execute compression
    if (!strcmp(argv[1],"1")){
        compressed("original.txt");
    }
    //2 to execute decompression
    else if (!strcmp(argv[1],"2")){
        decompressed("compressed.txt");
    }
}
//function definition
//////////////////////COMPRESSION FUNCTION////////////////////////////
void compressed(string ToCompressFilename){  
    //CREATING THE DICTIONARY
    vector<string> vecOriginal; //to store original instructions
    ifstream ReadOriginal(ToCompressFilename);
    string text;
    while (getline (ReadOriginal, text)) 
        {
        vecOriginal.push_back(text);  
        }
    ReadOriginal.close();

    vector<string> checked; //to store checked instructions
    vector<int> countvec; //to store frequencies
    vector<int> countvecCopy; 
    for(vector<string>::iterator i=vecOriginal.begin();i!=vecOriginal.end();++i){
        int count = 0;
        if (!(find(checked.begin(), checked.end(), *i) != checked.end())) {

            for(vector<string>::iterator j=vecOriginal.begin();j!=vecOriginal.end();++j) {
                if(*i == *j)
                    {count++;}  
            }
            checked.push_back(*i); 
            countvec.push_back(count); 
            countvecCopy.push_back(count); 
        }
    }
    sort(countvec.begin(), countvec.end(), greater<int>()); //arrange the frequencies in ascending order
    vector<int> subcountvec = {countvec.begin() , countvec.begin() +8}; //consider most frequent 8 elements
    vector<string> dict; 
    for (auto x : subcountvec){
        int afterIndx = find(countvecCopy.begin(), countvecCopy.end(), x)-countvecCopy.begin(); 
        string textSort = checked.at(afterIndx);
    
        checked.erase(checked.begin()+afterIndx); 
        countvecCopy.erase(countvecCopy.begin()+afterIndx); 
    
        dict.push_back(textSort); 
    }

    //CREATING COMPRESSED INSTRUCTIONS
    vector<string> vecComp; //store compressed instructions
    string prevIns="";

    int consCount = 0;  //consecutive count
    bool CHECK; 
    bool RLE_found=false;
    for (auto ins : vecOriginal){
        string compressed="";
        CHECK=true;
        //RLE CHECK
        if (ins == prevIns){
            RLE_found = true;
            consCount = consCount + 1;
            CHECK=false;
        }
        //DIRECT MATCHING CHECK
        else if ((find(dict.begin(), dict.end(), ins) != dict.end()) && CHECK) {
            if (RLE_found && ins != prevIns){
                string compressed = "000"+ bitset<2>(consCount-1).to_string();
                vecComp.push_back(compressed);
                consCount = 0;
                RLE_found = false; 
            }
            int dictIndx = find(dict.begin(), dict.end(), ins)-dict.begin();
            string binary = bitset<3>(dictIndx).to_string(); //convert to binary
            string compressed = "101"+binary;
            vecComp.push_back(compressed); 
            CHECK=false;
        }
        //MISMATCH CHECK
        else if (CHECK){
            if (RLE_found && ins != prevIns){
                string compressed = "000"+bitset<2>(consCount-1).to_string();
                vecComp.push_back(compressed);
                consCount = 0;
                RLE_found = false; 
            }
            vector<vector<int>> misLocDic; //store mismatch locations of all instructions in dictionary
            for (auto dins : dict){  
                vector<int> misIns; //store mismatch locations of an instruction
                for (int i = 0; i < 32; i++) {
                    if (ins[i] != dins[i]){
                        misIns.push_back(i); 
                    }
                }
                misLocDic.push_back(misIns); 
            }
            bool notdone = true; 
            for (int j = 0; j < 8; j++){
                //1-BIT MISMATCH
                if (misLocDic[j].size()==1){
                    string binLoc = bitset<5>(misLocDic[j][0]).to_string(); //binary of mislocation
                    string binDicLoc = bitset<3>(j).to_string(); //binary of dictionary index
                    string compressed = "010"+binLoc+binDicLoc;
                    vecComp.push_back(compressed); 
                    notdone = false; 
                    CHECK=false;
                    break;
                }
            }
            if (notdone){
                for (int j = 0; j < 8; j++){
                    //2-BIT CONSECUTIVE MISMATCH
                    if ((misLocDic[j].size()==2) && (misLocDic[j][1]-misLocDic[j][0]==1)){
                        string binLoc = bitset<5>(misLocDic[j][0]).to_string(); //binary of mislocation
                        string binDicLoc = bitset<3>(j).to_string(); //binary of dictionary index
                        string compressed = "011"+binLoc+binDicLoc;
                        vecComp.push_back(compressed); 
                        notdone = false; 
                        bool CHECK=false;
                        break;
                    } 
                }
            } 
            if (notdone){
                for (int j = 0; j < 8; j++){
                    //4-BITMASK
                    if ((misLocDic[j].size()<=4) && (misLocDic[j].back()-misLocDic[j].front()<=3)){
                        int prevIndx = -1;
                        string bitmask = "";
                        for (auto misIndx : misLocDic[j]){
                            if (prevIndx==-1){
                                bitmask = "1";
                            }
                            else {
                                bitmask = bitmask + string(misIndx-prevIndx-1,'0') + "1";
                            }
                            prevIndx = misIndx;
                        }
                        int diff = 4 - bitmask.size();  //to check bitmask is 4 digit length
                        if (diff!=0){
                            if ((misLocDic[j][misLocDic[j].size()-1]+diff)<32){
                                bitmask = bitmask + string(diff,'0');
                            }
                            else {
                                bitmask = string(diff,'0') + bitmask;
                            }
                        }
                        string binLoc = bitset<5>(misLocDic[j][0]).to_string(); //binary of mislocation
                        string binDicLoc = bitset<3>(j).to_string(); //binary of dictionary index
                        string compressed = "001"+binLoc+bitmask+binDicLoc;
                        vecComp.push_back(compressed); 
                        notdone = false; 
                        CHECK=false;
                        break;
                    }   
                }
            }
            if (notdone){
                for (int j = 0; j < 8; j++){
                    //2-BIT MISMATCH ANYWHERE
                    if (misLocDic[j].size()==2){
                        string binLoc1 = bitset<5>(misLocDic[j][0]).to_string(); //binary of mislocation1
                        string binLoc2 = bitset<5>(misLocDic[j][1]).to_string(); //binary of mislocation2
                        string binDicLoc = bitset<3>(j).to_string(); //binary of dictionary index
                        string compressed = "100"+binLoc1+binLoc2+binDicLoc;
                        vecComp.push_back(compressed); 
                        notdone = false; 
                        CHECK=false;
                        break;
                    }   
                }
            } 
            if (notdone){
                string compressed = "110"+ins;
                vecComp.push_back(compressed); 
            }      
        }
        prevIns = ins; 
    }
    //WRITE TO TEXT FILE
    string concatComp = "";
    string CompToWrite = "";
    string DictToWrite = "";
    for (auto c : vecComp){
        concatComp = concatComp + c;
    }
    int padding = 32-concatComp.size()%32;
    concatComp = concatComp + string(padding,'1');
    for (int i = 0; i<concatComp.length(); i+=32){
        CompToWrite = CompToWrite + concatComp.substr(i, 32) + "\n";
    }
    //dictionary
    for (auto d : dict){
        DictToWrite = DictToWrite + d + "\n";
    }
    DictToWrite.pop_back();  //remove last enter string
    //final text to write 
    string compText = CompToWrite+ "xxxx\n" + DictToWrite;
    //write to cout.txt
    ofstream compFile("cout.txt");
    compFile << compText;
    compFile.close();
}

//////////////////////DECOMPRESSION FUNCTION////////////////////////////
void decompressed(string ToDecompressFilename){
    //EXTRACTING THE COMPRESSED INSTRUCTIONS & DICTIONARY
    vector<string> vecCompressed; //store compressed instructions seperately
    ifstream ReadCompressed(ToDecompressFilename);
    string compText;    //all text in the file
    vector<string> dict; //store dictionary
    string allComp = "";    //to concatenate compressed instructions 
    int itr = 0;
    bool notDict = true;
    while (getline (ReadCompressed, compText)) {
            if (compText != "xxxx"){    //seperating dictionary part with compressed instructions
                if (notDict){
                    allComp += compText;
                } else {
                    dict.push_back(compText);  
                }
            } else {
                notDict = false;
            } 
        }
    ReadCompressed.close();

    //CREATING A MAP WITH DICTIONARY INDICES & NO.Of BITS
    map <string, int> ref;   
    ref["000"] = 3+2;  
    ref["001"] = 3+12;  
    ref["010"] = 3+8;  
    ref["011"] = 3+8;  
    ref["100"] = 3+13;  
    ref["101"] = 3+3; 
    ref["110"] = 3+32; 

    //checking
    int itrID = 0; 
    string cmpIns = ""; //compressed instruction
    string cmpID;   //compressed instruction id
    string decompText; //store decompressing instructions
    string decmpIns; //decompressed instruction
    string prevIns; //previous instruction
    int dictID; //dictionary id
    int loc1;   //location 1
    int loc2;   //location 2
    while (itrID+2 < allComp.size()){   //while end
        string index = allComp.substr(itrID, 3);
        if (index == "111"){  //end (ignore paddings)
            break; }
        cmpIns = allComp.substr(itrID, ref[index]);
        itrID += ref[index]; 
        cmpID = cmpIns.substr(0, 3);  //id of current compressed instruction
        //check each
        if (cmpID == "110"){
            decmpIns = cmpIns.substr(3,32);
        } else if (cmpID == "101"){
            dictID = stoi(cmpIns.substr(3,3), nullptr, 2);
            decmpIns = dict[dictID];
        } else if (cmpID == "100"){
            loc1 = stoi(cmpIns.substr(3,5), nullptr, 2);
            loc2 = stoi(cmpIns.substr(8,5), nullptr, 2);
            dictID = stoi(cmpIns.substr(13,3), nullptr, 2);
            decmpIns = dict[dictID];
            decmpIns[loc1] = char(!(decmpIns[loc1]-48)+48);  //as character type
            decmpIns[loc2] = char(!(decmpIns[loc2]-48)+48);
        } else if (cmpID == "011"){
            loc1 = stoi(cmpIns.substr(3,5), nullptr, 2);
            dictID = stoi(cmpIns.substr(8,3), nullptr, 2);
            decmpIns = dict[dictID];
            decmpIns[loc1] = char(!(decmpIns[loc1]-48)+48);
            decmpIns[loc1+1] = char(!(decmpIns[loc1+1]-48)+48);
        } else if (cmpID == "010"){
            loc1 = stoi(cmpIns.substr(3,5), nullptr, 2);
            dictID = stoi(cmpIns.substr(8,3), nullptr, 2);
            decmpIns = dict[dictID];
            decmpIns[loc1] = char(!(decmpIns[loc1]-48)+48);
        } else if (cmpID == "001"){
            loc1 = stoi(cmpIns.substr(3,5), nullptr, 2);
            string bitmask = cmpIns.substr(8,4);
            dictID = stoi(cmpIns.substr(12,3), nullptr, 2);
            decmpIns = dict[dictID];
            for (int i = 0; i < bitmask.size(); i++){
                if (bitmask[i]=='1'){
                    decmpIns[loc1+i] = char(!(decmpIns[loc1+i]-48)+48);
                }
            }
        }else if (cmpID == "000"){
            int occr = stoi(cmpIns.substr(3,2), nullptr, 2);
            decmpIns = prevIns;
            for (int i = 0; i < occr; i++){
                decompText += decmpIns + "\n";
            }
        }
        prevIns = decmpIns;
        decompText += decmpIns + "\n";  
    }
    decompText.pop_back();  //remove last enter string

    //writing to dout.txt
    ofstream decompFile("dout.txt");
    decompFile << decompText;
    decompFile.close();
}