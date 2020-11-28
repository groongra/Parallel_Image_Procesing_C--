#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <dirent.h>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define COPY "copy"
#define GAUSS "gauss"
#define SOBEL "sobel"
#define BPP24PX 24
#define COMPRESS 0
#define PLANES 1
#define BMP_FILE_HEADER 14 /* headerSize*/
#define BMP_INFO_HEADER 40 /* headerSize*/

typedef struct bmpFileHeader
{
    uint16_t type;   /* 'B' 'M' Bytes */
    uint32_t size;   /* FileSize */
    uint32_t resv;   /* Reserved */
    uint32_t offset; /* Offset*/

} bmpFileHeader;

typedef struct bmpInfoHeader
{
    uint32_t headersize; /* Header size */
    uint32_t width;      /* Img width */
    uint32_t height;     /* Img height */
    uint16_t planes;     /* Color planes (1) */
    uint16_t bpp;        /* Bits per pixel */
    uint32_t compress;   /* Compresion */
    uint32_t imgsize;    /* Img size */
    uint32_t bpmx;       /* Bits x resolution per meter*/
    uint32_t bpmy;       /* Bits y resolution per meter*/
    uint32_t colors;     /* Color palette*/
    uint32_t imxtcolors; /* Relevant colors (0 all)*/
} bmpInfoHeader;

unsigned char *LoadBMP(FILE *f, bmpInfoHeader *bInfoHeader, bmpFileHeader *bFileHeader)
{
    unsigned char *imgdata; /* Img data */
    if (f == NULL)
    {
        fclose(f);
        return NULL;
    }

    fread(bFileHeader, BMP_FILE_HEADER, 1, f); //Read file's header

    if (bFileHeader->type != 0x4D42) /* Check correct format */
    {
        fclose(f);
        return NULL;
    }
    fread(bInfoHeader, BMP_INFO_HEADER, 1, f); //Read bmp's header

    imgdata = (unsigned char *)malloc(bInfoHeader->imgsize); //Allocate memory (imgsize)

    //fseek(f, bFileHeader->offset, SEEK_SET); //Set filedescriptor to the beginning of the image data (bmp file header offset)

    /*while (!feof(f)){
     fread(imgdata, 1, 1, f);
  }*/
    int offset = 0;

    fseek(f, 10, SEEK_SET);
    fread(&offset, sizeof(int), 1, f); // Read image data, in other words, imgsize bytes*/
    std::cout << "OFFSET " << offset << "\n";
    fseek(f, 54, 0);
    fread(imgdata, bInfoHeader->imgsize, 1, f); // Read image data, in other words, imgsize bytes*/

    std::cout << "suma:" << BMP_INFO_HEADER + BMP_FILE_HEADER + bInfoHeader->imgsize;
    fclose(f);
    return imgdata;
}

int checkBMP(bmpInfoHeader *bInfoHeader)
{
    std::string msg, error_msg;
    int comparison = 0;
    comparison = (bInfoHeader->planes == PLANES);
    if (!comparison)
    {
        error_msg = "\tIncorrect number of planes\n";
        msg.append(error_msg);
    }
    comparison = (bInfoHeader->compress == COMPRESS);
    if (!comparison)
    {
        error_msg = "\tIncorrect compression value\n";
        msg.append(error_msg);
    }
    comparison = (bInfoHeader->bpp == BPP24PX);
    if (!comparison)
    {
        error_msg = "\tIncorrect bits per pixel\n";
        msg.append(error_msg);
    }
    std::cout << msg;
    return (comparison);
}

int fcopy(bmpFileHeader *bmpFile, bmpInfoHeader *bmpInfo, const void *bmp_img, size_t imgSize, char *copyPath)
{
    FILE *fdDest = fopen(copyPath, "w");
    if (fdDest != NULL)
    {

        if (fwrite(bmpFile, BMP_FILE_HEADER, 1, fdDest) != 1)
            return -1;
        if (fwrite(bmpInfo, BMP_INFO_HEADER, 1, fdDest) != 1)
            return -1;
        if (fwrite(bmp_img, imgSize, 1, fdDest) != 1)
            return -1;
    }
    std::cout << "siee";
    return 0;
}

int fgauss()
{
    return (0);
}

int fsobel()
{
    return (0);
}

void displayBMPInfo(bmpInfoHeader *info)
{
    printf("\t| Header size %u\n", info->headersize);
    printf("\t| Width: %d\n", info->width);
    printf("\t| Height: %d\n", info->height);
    printf("\t| Planes (1): %d\n", info->planes);
    printf("\t| Bits per pixel: %d\n", info->bpp);
    printf("\t| Compresion: %d\n", info->compress);
    printf("\t| Image size: %u\n", info->imgsize);
    printf("\t| Horizontal resolution: %u\n", info->bpmx);
    printf("\t| Vertical resolution: %u\n", info->bpmy);
    printf("\t| Color palette: %d\n", info->colors);
    printf("\t| Relevant colors (0 all): %d\n", info->imxtcolors);
}

void displayBMPFile(bmpFileHeader *info)
{
    printf("\t| type %c\n", info->type);
    printf("\t| size: %i\n", info->size);
    printf("\t| reserved: %d\n", info->resv);
    printf("\t| offset: %i\n", info->offset);
}

void runtimeError(int errorCode, std::string elem)
{
    switch (errorCode)
    {
    case -1:
        std::cout << "Undefined error" << std::endl;
        break;
    case -2:
        std::cout << "Wrong format:" << std::endl;
        break;
    case -3:
        std::cout << ("Unexpected operation: " + elem + "\n");
        break;
    case -4:
        std::cout << ("Cannot open directory [" + elem + "]\n");
        break;
    case -5:
        std::cout << ("Output directory [" + elem + "] does not exist\n");
        break;
    }
    std::ostringstream oss;
    oss << "\timage-seq operation in_path out_path\n\toperation:" << COPY << ", " << GAUSS << ", " << SOBEL;
    std::cout << oss.str() << std::endl;
    exit(errorCode);
}

////////////////////MAIN////////////////////

int main(int argc, char **argv)
{
    int copy, gauss, sobel;
    if (argc != 4)
        runtimeError(-2, "");
    else
    {
        copy = (strcmp(argv[1], COPY) == 0);
        gauss = (strcmp(argv[1], GAUSS) == 0);
        sobel = (strcmp(argv[1], SOBEL) == 0);

        if (copy + gauss + sobel != 1)
            runtimeError(-3, argv[1]);
    }

    DIR *dir_in, *dir_out;
    struct dirent *ent_dir_in;
    struct stat statbuff;
    char *copyPath;
    char *copyPath2;
    char *extension;
    unsigned int copyPathSize;

    if ((dir_in = opendir(argv[2])) == NULL) /* could not open input directory */
    {
        runtimeError(-4, argv[2]);
    }
    else if ((dir_out = opendir(argv[3])) == NULL) /* could not open output directory */
    {
        runtimeError(-5, argv[3]);
    }
    while ((ent_dir_in = readdir(dir_in)) != NULL)
    {
        std::string bmp_file = ent_dir_in->d_name;

        if (!S_ISDIR(statbuff.st_mode)) //Check it is a Dir file
        {

            extension = strchr(ent_dir_in->d_name, '.');
            if (extension && (strcmp(extension, ".bmp") == 0)) //Check BMP extension
            {

                std::cout << bmp_file << std::endl;
                unsigned char *bmp_img;
                bmpFileHeader bmp_file_header;
                bmpInfoHeader bmp_img_info;

                copyPath2 = (char *)malloc(copyPathSize * sizeof(char));
                strcpy(copyPath2, argv[2]);
                strcat(copyPath2, "/");
                strcat(copyPath2, ent_dir_in->d_name);

                FILE *source = fopen(copyPath2, "r"); //Open source file

                bmp_img = LoadBMP(source, &bmp_img_info, &bmp_file_header); //Obtain BMP header and file structure

                if (bmp_img == NULL)
                    std::cout << ("> Can't open [" + bmp_file + "]\n"); //NO access

                if (checkBMP(&bmp_img_info))
                { //Apply tranformation
                    displayBMPFile(&bmp_file_header);
                    std::cout << "\n";
                    displayBMPInfo(&bmp_img_info);

                    if (copy)
                    {
                        //Copy
                        copyPathSize = strlen(argv[3]) + strlen("/") + strlen(ent_dir_in->d_name);
                        copyPath = (char *)malloc(copyPathSize * sizeof(char));
                        strcpy(copyPath, argv[3]);
                        strcat(copyPath, "/");
                        strcat(copyPath, ent_dir_in->d_name);

                        std::cout << "Copying " << ent_dir_in->d_name << " in " << copyPath << "\n";

                        if (fcopy(&bmp_file_header, &bmp_img_info, bmp_img, bmp_img_info.imgsize, copyPath) < 0)
                            std::cout << "Failed do copy " << ent_dir_in->d_name << " in " << copyPath << "\n";
                    }
                    else if (gauss)
                    {
                        fgauss();
                    }
                    else if (sobel)
                    {
                        fsobel();
                    }
                    free(bmp_img);
                    free(copyPath2);
                    free(copyPath);
                }

                //close(source);
            }
        }
    }
    closedir(dir_in);
    closedir(dir_out);
    exit(0);
}