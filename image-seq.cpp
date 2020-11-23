#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <dirent.h>
#include <sstream>

#define COPY "copy"
#define GAUSS "gauss"
#define SOBEL "sobel"

void runtimeError(int errorCode, std::string elem) {
    switch(errorCode){
        case -1: std::cout << "Undefined error" <<std::endl;
        break;
        case -2: std::cout << "Wrong format:" << std::endl;
        break;
        case -3: std::cout << ("Unexpected operation: "+elem+"\n");
        break;
        case -4: std::cout << ("Cannot open directory ["+elem+"]\n");
        break;
        case -5: std::cout << ("Output directory ["+elem+"] does not exist\n");
        break;
    }
    std::ostringstream oss;
    oss << "\timage-seq operation in_path out_path\n\toperation:" << COPY << ", " << GAUSS <<", " << SOBEL;
    std::cout << oss.str() << std::endl;
    exit(errorCode);
}

int main(int argc, char **argv){

    if(argc!=4) runtimeError(-2,"");
    else {
        
        if(strcmp(argv[1],COPY)!=0 && strcmp(argv[1],GAUSS)!=0 && strcmp(argv[1],SOBEL)!=0) runtimeError(-3,argv[1]);
    }
 
    DIR *dir_in, *dir_out;
    struct dirent *ent;
    if ((dir_in = opendir (argv[2])) == NULL) {  /* could not open input directory */
        runtimeError(-4,argv[2]);
    } else if((dir_out = opendir (argv[3])) == NULL) {  /* could not open output directory */
        runtimeError(-5,argv[3]);
    }
    while ((ent = readdir (dir_in)) != NULL) {
        std::string s= ent->d_name;
        std::cout << s << std::endl;
    }
    closedir (dir_in); 
    closedir (dir_out); 

    exit(0);
}