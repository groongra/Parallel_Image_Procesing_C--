#include "error.h"
#include "string"

void printError(int errorCode, string elem) {
    switch(errorCode){
        case -1: cout << "Undefined error" <<endl;
        break;
        case -2: cout << "Wrong format:" <<endl;
        break;
        // case -3: std::string s("Unexpected operation:"+elem+"\n");
        // break;
        // case -4: std::string s("Cannot open directory ["+elem+"]\n");
        // break;
        // case -5: std::string s("Output directory ["+elem+"] does not exist\n");
        // break;
    }
    cout << "\timage-seq operation in_path out_path\n\toperation: copy, gauss, sobel" <<enl;
}