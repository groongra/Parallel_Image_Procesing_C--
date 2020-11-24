#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <dirent.h>
#include <sstream>

#define COPY "copy"
#define GAUSS "gauss"
#define SOBEL "sobel"
#define BPP24PX 24
#define COMPRESS 0
#define PLANES 1

typedef struct bmpFileHeader
{
  /* 2 bytes de identificación */
  uint32_t size;        /* Tamaño del archivo */
  uint16_t resv1;       /* Reservado */
  uint16_t resv2;       /* Reservado */
  uint32_t offset;      /* Offset hasta hasta los datos de imagen */
} bmpFileHeader;

typedef struct bmpInfoHeader
{
  uint32_t headersize;  /* Tamaño de la cabecera */
  uint32_t width;       /* Ancho */
  uint32_t height;      /* Alto */
  uint16_t planes;      /* Planos de color (Siempre 1) */
  uint16_t bpp;         /* bits por pixel */
  uint32_t compress;    /* compresión */
  uint32_t imgsize;     /* tamaño de los datos de imagen */
  uint32_t bpmx;        /* Resolución X en bits por metro */
  uint32_t bpmy;        /* Resolución Y en bits por metro */
  uint32_t colors;      /* colors used en la paleta */
  uint32_t imxtcolors;  /* Colores importantes. 0 si son todos */
} bmpInfoHeader;

unsigned char *LoadBMP(char *filename, bmpInfoHeader *bInfoHeader)
{
    FILE *f;
    bmpFileHeader header;     /* cabecera */
    unsigned char *imgdata;   /* datos de imagen */
    uint16_t type;        /* 2 bytes identificativos */

    f=fopen (filename, "r");
    if (!f)
      return NULL;        /* Si no podemos leer, no hay imagen*/

    /* Leemos los dos primeros bytes */
    fread(&type, sizeof(uint16_t), 1, f);
    if (type !=0x4D42)        /* Comprobamos el formato */
      {
        fclose(f);
        return NULL;
      }

    /* Leemos la cabecera de fichero completa */
    fread(&header, sizeof(bmpFileHeader), 1, f);

    /* Leemos la cabecera de información completa */
    fread(bInfoHeader, sizeof(bmpInfoHeader), 1, f);

    /* Reservamos memoria para la imagen, ¿cuánta?
        Tanto como indique imgsize */
    imgdata=(unsigned char*)malloc(bInfoHeader->imgsize);

    /* Nos situamos en el sitio donde empiezan los datos de imagen,
      nos lo indica el offset de la cabecera de fichero*/
    fseek(f, header.offset, SEEK_SET);

    /* Leemos los datos de imagen, tantos bytes como imgsize */
    fread(imgdata, bInfoHeader->imgsize,1, f);

    /* Cerramos */
    fclose(f);

    /* Devolvemos la imagen */
    return imgdata;
}

int checkBMP(bmpInfoHeader *bInfoHeader ){
    std::string msg;
    std::string error_msg; 
    if(bInfoHeader->planes!=PLANES){
        error_msg = "\tIncorrect number of planes\n";
        msg.append(error_msg);
    }
    if(bInfoHeader->compress!=COMPRESS){
        error_msg = "\tIncorrect compression value\n";
        msg.append(error_msg);
    }
    if(bInfoHeader->bpp!=BPP24PX){
        error_msg = "\tIncorrect bits per pixel\n";
        msg.append(error_msg);
    }
    std::cout << msg;
    return(-1);

}

int fcopy(){
  return(0);
}

int fgauss(){
  return(0);
}

int fsobel(){
  return(0);


}

void DisplayInfo(bmpInfoHeader *info)
{
  printf("Tamaño de la cabecera: %u\n", info->headersize);
  printf("Anchura: %d\n", info->width);
  printf("Altura: %d\n", info->height);
  printf("Planos (1): %d\n", info->planes);
  printf("Bits por pixel: %d\n", info->bpp);
  printf("Compresión: %d\n", info->compress);
  printf("Tamaño de datos de imagen: %u\n", info->imgsize);
  printf("Resolucón horizontal: %u\n", info->bpmx);
  printf("Resolucón vertical: %u\n", info->bpmy);
  printf("Colores en paleta: %d\n", info->colors);
  printf("Colores importantes: %d\n", info->imxtcolors);
}

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

////////////////////MAIN////////////////////


int main(int argc, char **argv){
    
    int copy, gauss, sobel;
    if(argc!=4) runtimeError(-2,"");
    else {
        copy = strcmp(argv[1],COPY);
        gauss = strcmp(argv[1],GAUSS);
        sobel = strcmp(argv[1],SOBEL);

        if(copy!=0 && gauss!=0 && sobel!=0) runtimeError(-3,argv[1]);
    }
 
    DIR *dir_in, *dir_out;
    struct dirent *ent;
    if ((dir_in = opendir (argv[2])) == NULL) {  /* could not open input directory */
        runtimeError(-4,argv[2]);
    } else if((dir_out = opendir (argv[3])) == NULL) {  /* could not open output directory */
        runtimeError(-5,argv[3]);
    }
    while ((ent = readdir (dir_in)) != NULL) {
        std::string bmp_file = ent->d_name;
        char* extension = strchr(ent->d_name, '.');

        if(extension && (strcmp(extension, ".bmp")==0))  {

            std::cout << std::endl << bmp_file << std::endl;
            unsigned char* bmp_img;
            bmpInfoHeader bmp_img_info;
            bmp_img = LoadBMP(ent->d_name, &bmp_img_info);
            
            if(bmp_img == NULL) std:: cout << ("> Can't open ["+bmp_file+"]\n");
            if(checkBMP(&bmp_img_info)==0){
              //Apply tranformation
              if(copy){
                fcopy();
              } else if (gauss){
                fgauss();
              } else if (sobel){
                fsobel();
              }
            }

        } 
        
  
      
    }
    closedir (dir_in); 
    closedir (dir_out); 



    exit(0);
}