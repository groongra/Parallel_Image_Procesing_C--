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
#include <omp.h>

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
    TIME_UNIT gaussTime;
    TIME_UNIT sobelTime;
    TIME_UNIT writeTime;

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
    int headersize;      /* Header size */
    int width;           /* Img width */
    int height;          /* Img height */
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

typedef struct defaultBmpFileHeader
{
    char B = 'B';         /* 'B' 'M' Bytes */
    char M = 'M';         /* 'B' 'M' Bytes */
                          /* FileSize */
    uint32_t resv = 0;    /* Reserved */
    uint32_t offset = 54; /* Offset*/

} defaultBmpFileHeader;

typedef struct defaultBmpInfoHeader
{
    int headersize = 40;     /* Header size */
                             /* Img width */
                             /* Img height */
    uint16_t planes = 1;     /* Color planes (1) */
    uint16_t bpp = 24;       /* Bits per pixel */
    uint32_t compress = 0;   /* Compresion */
                             /* Img size */
    uint32_t bpmx = 2835;    /* Bits x resolution per meter*/
    uint32_t bpmy = 2835;    /* Bits y resolution per meter*/
    uint32_t colors = 0;     /* Color palette*/
    uint32_t imxtcolors = 0; /* Relevant colors (0 all)*/

} defaultBmpInfoHeader;

defaultBmpFileHeader defaultFile;
defaultBmpInfoHeader defaultHeader;

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
int readBMP(FILE *f, bmp *bmp);
int writeBMP(bmp *bmp, char *copyPath);
unsigned char *applyFilter(unsigned char *arr, unsigned char *result, int width, int height, const char *blurOperation, timeMetrics *time);

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
                if (readBMP(source, &bmp) == 0) //Obtain BMP header and file structure
                {
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
                            time.gaussTime = TIME_UNIT::zero();
                            time.sobelTime = TIME_UNIT::zero();
                        }
                        else
                        {
                            int bytesPerPixel = ((int)bmp.infoHeader.bpp) / 8;
                            int unpaddedRowSize = (bmp.infoHeader.width) * (bytesPerPixel);
                            int totalSize = unpaddedRowSize * (bmp.infoHeader.height);
                            result = (unsigned char *)malloc(totalSize);

                            if (gauss)
                            {

                                bmp.image = applyFilter(bmp.image, result, bmp.infoHeader.width, bmp.infoHeader.height, GAUSS, &time);
                            }
                            else //sobel
                            {
                                bmp.image = applyFilter(bmp.image, result, bmp.infoHeader.width, bmp.infoHeader.height, SOBEL, &time);
                            }
                        }

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

                        float totalTime = time.readingTime.count() + time.gaussTime.count() + time.sobelTime.count() + time.writeTime.count();
                        std::cout << "File:  \"" << source_path << "\"(time: " << totalTime << ")\n";
                        displayTime(time, argv[1]);

                        free(bmp.image);
                        free(source_path);
                        free(dest_path);
                    }
                }
                //fclose(source);
            }
        }
    }

    closedir(dir_in);
    closedir(dir_out);
    exit(0);
}

int readBMP(FILE *f, bmp *bmp)
{
    if (f == NULL)
    {
        fclose(f);
        bmp->image = NULL;
    }
    if (fread(&bmp->fileHeader.type, sizeof(uint16_t), 1, f) != 1) // Read image data, in other words, imgsize bytes
        return -1;
    if (fread(&bmp->fileHeader.size, sizeof(uint32_t), 1, f) != 1) // Read image data, in other words, imgsize bytes
        return -1;
    if (fread(&bmp->fileHeader.resv, sizeof(uint32_t), 1, f) != 1) // Read image data, in other words, imgsize bytes
        return -1;
    if (fread(&bmp->fileHeader.offset, sizeof(uint32_t), 1, f) != 1) // Read image data, in other words, imgsize bytes
        return -1;

    if (bmp->fileHeader.type != 0x4D42) /* Check correct format */
    {
        fclose(f);
        bmp->image = NULL;
    }
    if (fread(&bmp->infoHeader, sizeof(int), 3, f) != 3) //Read bmp's header
        return -1;
    if (fread(&bmp->infoHeader.planes, sizeof(uint16_t), 2, f) != 2) //Read bmp's header
        return -1;
    if (fread(&bmp->infoHeader.compress, sizeof(uint32_t), 6, f) != 6) //Read bmp's header
        return -1;

    int bytesPerPixel = ((int)bmp->infoHeader.bpp) / 8;
    int unpaddedRowSize = (bmp->infoHeader.width) * (bytesPerPixel);
    size_t size_t_cast_unpaddedRowSize = (size_t)unpaddedRowSize;
    int padding = (4 - (unpaddedRowSize % 4));
    if (padding == 4)
        padding = 0;
    int paddedRowSize = unpaddedRowSize + padding;
    if (DEBUG)
        std::cout << "\t| (padded|unpadded) ::" << paddedRowSize << " | " << unpaddedRowSize << "-> padding:" << padding << "\n";
    int totalSize = unpaddedRowSize * (bmp->infoHeader.height);

    bmp->image = (unsigned char *)malloc(totalSize);
    unsigned char *currentRowPointer = bmp->image + ((bmp->infoHeader.height - 1) * unpaddedRowSize);
    for (int i = 0; i < bmp->infoHeader.height; i++)
    {
        fseek(f, bmp->fileHeader.offset + (i * paddedRowSize), SEEK_SET);
        if (fread(currentRowPointer, 1, unpaddedRowSize, f) != size_t_cast_unpaddedRowSize)
            return -1;
        currentRowPointer -= unpaddedRowSize;
    }
    fclose(f);
    return 0;
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
        if (fwrite(&defaultFile.B, sizeof(char), 2, fdDest) != 2)
            return -1; //  2B
        bmp->fileHeader.size = bmp->infoHeader.imgsize + BMP_FILE_HEADER + BMP_INFO_HEADER;
        if (fwrite(&bmp->fileHeader.size, sizeof(u_int32_t), 1, fdDest) != 1)
            return -1; //  4B
        if (fwrite(&defaultFile.resv, sizeof(u_int32_t), 2, fdDest) != 2)
            return -1; //  8B
        if (fwrite(&defaultHeader.headersize, sizeof(u_int32_t), 1, fdDest) != 1)
            return -1; //  4B
        if (fwrite(&bmp->infoHeader.width, sizeof(int), 2, fdDest) != 2)
            return -1; //  8B
        if (fwrite(&defaultHeader.planes, sizeof(u_int16_t), 2, fdDest) != 2)
            return -1; //  8B
        if (fwrite(&defaultHeader.compress, sizeof(u_int32_t), 1, fdDest) != 1)
            return -1; //  4B
        if (fwrite(&bmp->infoHeader.imgsize, sizeof(u_int32_t), 1, fdDest) != 1)
            return -1; //  4B
        if (fwrite(&defaultHeader.bpmx, sizeof(u_int32_t), 4, fdDest) != 4)
            return -1; //  12B
        fseek(fdDest, defaultFile.offset, SEEK_SET);

        int bytesPerPixel = ((int)bmp->infoHeader.bpp) / 8;
        int unpaddedRowSize = (bmp->infoHeader.width) * (bytesPerPixel);
        int padding = (4 - (unpaddedRowSize % 4));
        if (padding == 4)
            padding = 0;
        int paddedRowSize = unpaddedRowSize + padding;
        if (DEBUG)
            std::cout << "\t| (padded|unpadded) ::" << paddedRowSize << " | " << unpaddedRowSize << "-> padding:" << padding << "\n";

        for (int i = 0; i < bmp->infoHeader.height; i++)
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

unsigned char *applyFilter(unsigned char *arr, unsigned char *result, int width, int height, const char *blurOperation, timeMetrics *time)
{
    int row, col, k, color, j, i, sum;
    auto startTime = std::chrono::high_resolution_clock::now();
#pragma omp parallel for private(row, col, k, j, i, color)
    for (row = 0; row < height; row++) //Rows
    {
        for (col = 0; col < width; col++) //Cols
        {
            for (k = 0; k < 3; k++) //RGB Colors
            {
                sum = 0; //gaussMask

                for (j = -2; j <= 2; j++)
                {
                    for (i = -2; i <= 2; i++)
                    {
                        if ((row + j) >= 0 && (row + j) < (int)height && (col + i) >= 0 && (col + i) < (int)width)
                        {
                            color = arr[(row + j) * 3 * (int)width + (col + i) * 3 + k];
                            sum += color * gauss[i + 2][j + 2];
                        }
                    }
                }
                result[3 * row * width + 3 * col + k] = sum / gaussWeight;
            }
        }
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    time->gaussTime = endTime - startTime;
    if (strcmp(blurOperation, SOBEL) == 0)
    {
        int sumSobelX, sumSobelY, colorX, colorY;
        auto startTime = std::chrono::high_resolution_clock::now();
#pragma omp parallel for private(row, col, k, j, i, colorX, colorY, sumSobelX, sumSobelY)
        for (row = 0; row < height; row++) //Rows
        {
            for (col = 0; col < width; col++) //Cols
            {
                for (k = 0; k < 3; k++) //RGB Colors
                {
                    sumSobelX = 0; //sobelMask
                    sumSobelY = 0;

                    for (j = -1; j <= 1; j++)
                    {
                        for (i = -1; i <= 1; i++)
                        {
                            if ((row + j) >= 0 && (row + j) < (int)height && (col + i) >= 0 && (col + i) < (int)width)
                            {
                                colorX = result[(row + j) * 3 * (int)width + (col + i) * 3 + k];
                                sumSobelX += colorX * sobelX[i + 1][j + 1];
                                colorY = result[(row + j) * 3 * (int)width + (col + i) * 3 + k];
                                sumSobelY += colorY * sobelY[i + 1][j + 1];
                            }
                        }
                    }
                    arr[3 * row * width + 3 * col + k] = (abs(sumSobelX) + abs(sumSobelY)) / sobelWeight;
                }
            }
        }
        result = arr;
        auto endTime = std::chrono::high_resolution_clock::now();
        time->sobelTime = endTime - startTime;
    }
    return result;
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
    std::cout << "  Load time: " << time.readingTime.count() << "\n";
    if (strcmp(operation, GAUSS) == 0)
        std::cout << "  " << operation << " time: " << time.gaussTime.count() << "\n";
    else if (strcmp(operation, SOBEL) == 0)
    {
        std::cout << "  " << GAUSS << " time: " << time.gaussTime.count() << "\n";
        std::cout << "  " << operation << " time: " << time.sobelTime.count() << "\n";
    }
    std::cout << "  Store time: " << time.writeTime.count() << "\n";
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