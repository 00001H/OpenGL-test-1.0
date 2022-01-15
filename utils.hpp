#ifndef _utils_
#define _utils_
#include<iostream>
#include<fstream>
using namespace std;
string loadStringFile(const char* fname){
    ifstream file;
    file.open(fname,ios::in);
    string st;
    char ch;
    if(!file){
        cerr << "Error: file " << fname << " not avaliable";
        return NULL;
    }
    while(!file.eof()){
        file >> noskipws >> ch;
        st += ch;
    }
    file.close();
    return st;
}
#endif
