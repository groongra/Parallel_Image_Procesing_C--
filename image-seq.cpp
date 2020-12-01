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
#include <math.h>
#include <chrono>

#define DEBUG 0
#define COPY "copy"
#define GAUSS "gauss"
#define SOBEL "sobel"
#define BPP24PX 24
#define COMPRESS 0
#define PLANES 1
#define BMP_FILE_HEADER 14 /* headerSize*/
#define BMP_INFO_HEADER 40 /* headerSize*/
#define TIME_UNIT std::chrono::duration<float, std::micro>

typedef struct timeMetrics
{
    TIME_UNIT readingTime;
    TIME_UNIT operationTime;
    TIME_UNIT writeTime;
    TIME_UNIT totalTime = readingTime + operationTime + writeTime;

} timeMetrics;

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

typedef struct bmp
{
    bmpFileHeader fileHeader;
    bmpInfoHeader infoHeader;
    unsigned char *image;
} bmp;

int gauss[5][5]{
    {1, 4, 7, 4, 1},
    {4, 16, 26, 16, 4},
    {7, 26, 41, 26, 7},
    {4, 16, 26, 16, 4},
    {1, 4, 7, 4, 1}};
int gaussWeight = 273;

int sobelX[3][3]{
    {1, 2, 1},
    {0, 0, 0},
    {-1, -2, -1}};
int sobelY[3][3]{
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}};
int sobelWeight = 8;

char *arrangePath(char *destination_path, char *destination_name);
void displayBMP(bmp *bmp);
void displayBMPInfo(bmpInfoHeader *info);
void displayBMPFile(bmpFileHeader *info);
void displayTime(timeMetrics time, char *operation);
void runtimeError(int errorCode, std::string elem);
int checkBMPHeader(bmpInfoHeader *bInfoHeader);
void readBMP(FILE *f, bmp *bmp);
int writeBMP(bmp *bmp, char *copyPath);
int sobelMask(unsigned char *arr, int col, int row, int k, uint32_t width, uint32_t height);
int gaussMask(unsigned char *arr, int col, int row, int k, uint32_t width, uint32_t height);
void applyFilter(unsigned char *arr, unsigned char *result, uint32_t width, uint32_t height, const char *blurOperation);

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
    char *dest_path, *source_path, *extension;

    if ((dir_in = opendir(argv[2])) == NULL) /* could not open input directory */
    {
        runtimeError(-4, argv[2]);
    }
    else if ((dir_out = opendir(argv[3])) == NULL) /* could not open output directory */
    {
        runtimeError(-5, argv[3]);
    }
    std::cout << "Input path: " << argv[2] << "\n";
    std::cout << "Output path: " << argv[3] << "\n";

    while ((ent_dir_in = readdir(dir_in)) != NULL)
    {
        if (!S_ISDIR(statbuff.st_mode)) //Check it is a Dir file
        {

            extension = strchr(ent_dir_in->d_name, '.');
            if (extension && (strcmp(extension, ".bmp") == 0)) //Check BMP extension
            {

                if (DEBUG)
                    std::cout << "\n"
                              << ent_dir_in->d_name << std::endl;

                bmp bmp;
                timeMetrics time;

                source_path = arrangePath(argv[2], ent_dir_in->d_name);

                FILE *source = fopen(source_path, "r"); //Open source file

                auto startTime = std::chrono::high_resolution_clock::now();
                readBMP(source, &bmp); //Obtain BMP header and file structure
                auto endTime = std::chrono::high_resolution_clock::now();
                time.readingTime = endTime - startTime;

                if (bmp.image == NULL)
                    if (DEBUG)
                        std::cout
                            << "> Can't open \"" << source_path << "\"\n"; //NO access

                if (checkBMPHeader(&bmp.infoHeader))
                {

                    if (DEBUG)
                        displayBMP(&bmp);

                    //Apply tranformation if present
                    unsigned char *result;
                    if (copy)
                    {
                        result = bmp.image;
                        time.operationTime = TIME_UNIT::zero();
                    }
                    else
                    {
                        int bytesPerPixel = ((int)bmp.infoHeader.bpp) / 8;
                        int unpaddedRowSize = (bmp.infoHeader.width) * (bytesPerPixel);
                        int totalSize = unpaddedRowSize * (bmp.infoHeader.height);
                        result = (unsigned char *)malloc(totalSize);

                        if (gauss)
                        {
                            startTime = std::chrono::high_resolution_clock::now();
                            applyFilter(bmp.image, result, bmp.infoHeader.width, bmp.infoHeader.height, GAUSS);
                            
                            endTime = std::chrono::high_resolution_clock::now();
                        }
                        else //sobel
                        {
                            startTime = std::chrono::high_resolution_clock::now();
                            applyFilter(bmp.image, result, bmp.infoHeader.width, bmp.infoHeader.height, SOBEL);
                            endTime = std::chrono::high_resolution_clock::now();
                        }
                        time.operationTime = endTime - startTime;
                    }
                    bmp.image = result;
                    //Copy file
                    dest_path = arrangePath(argv[3], ent_dir_in->d_name);
                    if (DEBUG)
                        std::cout << "Copying " << dest_path << "\n";
                    startTime = std::chrono::high_resolution_clock::now();
                    if (writeBMP(&bmp, dest_path) < 0)

                        if (DEBUG)
                            std::cout << "Failed to copy " << ent_dir_in->d_name << " in " << dest_path << "\n";
                    endTime = std::chrono::high_resolution_clock::now();
                    time.writeTime = endTime - startTime;

                    std::cout << "File:  \"" << source_path << "\"(time: " << time.totalTime.count() << ")\n";

                    displayTime(time, argv[1]);
                    //lsdisplayBMP(&bmp);

                    free(bmp.image);
                    free(source_path);
                    free(dest_path);
                }
                //fclose(source);
            }
        }
    }

    closedir(dir_in);
    closedir(dir_out);
    exit(0);
}

void readBMP(FILE *f, bmp *bmp)
{
    if (f == NULL)
    {
        fclose(f);
        bmp->image = NULL;
    }
    //fread(bFileHeader, BMP_FILE_HEADER, 1, f); //Read file's header

    //fseek(f, 0, SEEK_SET);
    fread(&bmp->fileHeader.type, sizeof(uint16_t), 1, f); // Read image data, in other words, imgsize bytes
    //std::cout << "type " << (char)bFileHeader->type << "\n";

    //fseek(f, 2, SEEK_SET);
    fread(&bmp->fileHeader.size, sizeof(uint32_t), 1, f); // Read image data, in other words, imgsize bytes
    //std::cout << "size " << bFileHeader->size << "\n";

    //fseek(f, 6, SEEK_SET);
    fread(&bmp->fileHeader.resv, sizeof(uint32_t), 1, f); // Read image data, in other words, imgsize bytes
    //std::cout << "resv " << bFileHeader->resv << "\n";

    //fseek(f, 10, SEEK_SET);
    fread(&bmp->fileHeader.offset, sizeof(uint32_t), 1, f); // Read image data, in other words, imgsize bytes
    //std::cout << "OFFSET " << bFileHeader->offset << "\n";

    if (bmp->fileHeader.type != 0x4D42) /* Check correct format */
    {
        fclose(f);
        bmp->image = NULL;
    }
    //fseek(f, BMP_FILE_HEADER, SEEK_SET);
    fread(&bmp->infoHeader, BMP_INFO_HEADER, 1, f); //Read bmp's header
    //char *imgdata;                                  /* Img data */
    //imgdata = (char *)malloc(bInfoHeader->imgsize); //Allocate memory (imgsize)

    //fseek(f, bFileHeader->offset, SEEK_SET);    //Set filedescriptor to the beginning of the image data (bmp file header offset)
    //fread(imgdata, bInfoHeader->imgsize, 1, f); // Read image data, in other words, imgsize bytes*/

    int bytesPerPixel = ((int)bmp->infoHeader.bpp) / 8;
    int unpaddedRowSize = (bmp->infoHeader.width) * (bytesPerPixel);
    int padding = (4 - (unpaddedRowSize % 4));
    if (padding == 4)
        padding = 0;
    int paddedRowSize = unpaddedRowSize + padding;
    if (DEBUG)
        std::cout << "\t| (padded|unpadded) ::" << paddedRowSize << " | " << unpaddedRowSize << "-> padding:" << padding << "\n";
    int totalSize = unpaddedRowSize * (bmp->infoHeader.height);

    bmp->image = (unsigned char *)malloc(totalSize);
    unsigned char *currentRowPointer = bmp->image + ((bmp->infoHeader.height - 1) * unpaddedRowSize);
    for (uint32_t i = 0; i < bmp->infoHeader.height; i++)
    {
        fseek(f, bmp->fileHeader.offset + (i * paddedRowSize), SEEK_SET);
        fread(currentRowPointer, 1, unpaddedRowSize, f);
        currentRowPointer -= unpaddedRowSize;
    }
    fclose(f);
}

int checkBMPHeader(bmpInfoHeader *bInfoHeader)
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

int writeBMP(bmp *bmp, char *copyPath)
{
    FILE *fdDest = fopen(copyPath, "wb");

    if (fdDest != NULL)
    {
        if (fwrite(&bmp->fileHeader, sizeof(u_int16_t), 1, fdDest) != 1)
            return -1; //  2B
        if (fwrite(&bmp->fileHeader.size, sizeof(u_int32_t), 3, fdDest) != 3)
            return -1; //  12B
        if (fwrite(&bmp->infoHeader.headersize, sizeof(u_int32_t), 3, fdDest) != 3)
            return -1; //  12B
        if (fwrite(&bmp->infoHeader.planes, sizeof(u_int16_t), 2, fdDest) != 2)
            return -1; //  4B
        if (fwrite(&bmp->infoHeader.compress, sizeof(u_int32_t), 6, fdDest) != 6)
            return -1; //  24B
        fseek(fdDest, bmp->fileHeader.offset, SEEK_SET);

        /*if (fwrite(bmp_img, imgSize, 1, fdDest) != 1)
            return
             -1; //  24B
        */

        int bytesPerPixel = ((int)bmp->infoHeader.bpp) / 8;
        int unpaddedRowSize = (bmp->infoHeader.width) * (bytesPerPixel);
        int padding = (4 - (unpaddedRowSize % 4));
        if (padding == 4)
            padding = 0;
        int paddedRowSize = unpaddedRowSize + padding;
        if (DEBUG)
            std::cout << "\t| (padded|unpadded) ::" << paddedRowSize << " | " << unpaddedRowSize << "-> padding:" << padding << "\n";

        for (uint32_t i = 0; i < bmp->infoHeader.height; i++)
        {
            int pixelOffset = ((bmp->infoHeader.height - i) - 1) * unpaddedRowSize;
            fwrite(&bmp->image[pixelOffset], 1, paddedRowSize, fdDest);
        }
        fclose(fdDest);
    }
    return 0;
}

char *arrangePath(char *destination_path, char *destination_name)
{
    unsigned int copyPathSize = strlen(destination_path) + strlen("/") + strlen(destination_name);
    char *path = (char *)malloc(copyPathSize * sizeof(char));
    strcpy(path, destination_path);
    if (path[(strlen(path) - 1)] != '/')
        strcat(path, "/");
    strcat(path, destination_name);
    return path;
}

int sobelMask(unsigned char *arr, int col, int row, int k, uint32_t width, uint32_t height)
{
    int sumSobelX = 0, sumSobelY = 0;
    uint32_t colorX, colorY;
    for (int j = -1; j <= 1; j++)
    {
        for (int i = -1; i <= 1; i++)
        {
            if ((row + j) >= 0 && (row + j) < (int)height && (col + i) >= 0 && (col + i) < (int)width)
            {
                colorX = arr[(row + j) * 3 * (int)width + (col + i) * 3 + k];
                sumSobelX += colorX * sobelX[i + 1][j + 1];
                colorY = arr[(row + j) * 3 * (int)width + (col + i) * 3 + k];
                sumSobelY += colorY * sobelY[i + 1][j + 1];
            }
        }
    }
    return (abs(sumSobelX) / sobelWeight) + (abs(sumSobelY) / sobelWeight);
}

int gaussMask(unsigned char *arr, int col, int row, int k, uint32_t width, uint32_t height)
{
    int sum = 0;

    for (int j = -2; j <= 2; j++)
    {
        for (int i = -2; i <= 2; i++)
        {
            if ((row + j) >= 0 && (row + j) < (int)height && (col + i) >= 0 && (col + i) < (int)width)
            {
                uint32_t color = arr[(row + j) * 3 * (int)width + (col + i) * 3 + k];
                sum += color * gauss[i + 2][j + 2];
            }
        }
    }
    return sum / gaussWeight;
}

void applyFilter(unsigned char *arr, unsigned char *result, uint32_t width, uint32_t height, const char *blurOperation)
{
    std::cout << &arr <<"\t" <<&result <<"\n" ;

    for (uint32_t row = 0; row < height; row++) //Rows
    {
        for (uint32_t col = 0; col < width; col++) //Cols
        {
            for (uint32_t k = 0; k < 3; k++) //RGB Colors
            {
                result[3 * row * width + 3 * col + k] = gaussMask(arr, col, row, k, width, height);
            }
        }
    }
    if (strcmp(blurOperation, SOBEL) == 0)
    {
        std::cout << &arr <<"\t" <<&result <<"\n" ;
        for (uint32_t row = 0; row < height; row++) //Rows
        {
            for (uint32_t col = 0; col < width; col++) //Cols
            {
                for (uint32_t k = 0; k < 3; k++) //RGB Colors
                {
                    arr[3 * row * width + 3 * col + k] = sobelMask(result, col, row, k, width, height);
                }
            }
        }
        result=arr;
        std::cout << &arr <<"\t" <<&result <<"\n" ;
    }
}

void displayBMP(bmp *bmp)
{
    displayBMPFile(&bmp->fileHeader);
    displayBMPInfo(&bmp->infoHeader);
}

void displayBMPInfo(bmpInfoHeader *info)
{
    printf("\t| Header size: %u\n", info->headersize);
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
    printf("\t| type: %u\n", info->type);
    printf("\t| size: %i\n", info->size);
    printf("\t| reserved: %d\n", info->resv);
    printf("\t| offset: %i\n", info->offset);
}

void displayTime(timeMetrics time, char *operation)
{
    std::cout << "\tLoad time: " << time.readingTime.count() << "\n";
    if (strcmp(operation, COPY) != 0)
        std::cout << "\t" << operation << " time: " << time.operationTime.count() << "\n";
    std::cout << "\tStore time: " << time.writeTime.count() << "\n";
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