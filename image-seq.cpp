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
#include <unistd.h>

#define COPY "copy"
#define GAUSS "gauss"
#define SOBEL "sobel"
#define BPP24PX 24
#define COMPRESS 0
#define PLANES 1

typedef struct bmpFileHeader
{
  /* 2 bytes de identificación */
  uint32_t size;   /* FileSize */
  uint16_t resv1;  /* Reserved */
  uint16_t resv2;  /* Reserved*/
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

unsigned char *LoadBMP(FILE *f, bmpInfoHeader *bInfoHeader)
{
  bmpFileHeader header;   /* Header */
  unsigned char *imgdata; /* Img data */
  uint16_t type;          /* 2 bytes identificativos */
  if (f == NULL)
  {
    fclose(f);
    return NULL;
  }
  fread(&type, sizeof(uint16_t), 1, f); /* First 2 Bytes */
  if (type != 0x4D42)                   /* Check correct format */
  {
    fclose(f);
    return NULL;
  }
  fread(&header, sizeof(bmpFileHeader), 1, f); //Read file's header

  fread(bInfoHeader, sizeof(bmpInfoHeader), 1, f); //Read bmp's header

  imgdata = (unsigned char *)malloc(bInfoHeader->imgsize); //Allocate memory (imgsize)

  fseek(f, header.offset, SEEK_SET); //Set filedescriptor to the beginning of the image data (bmp file header offset)

  fread(imgdata, bInfoHeader->imgsize, 1, f); // Read image data, in other words, imgsize bytes*/

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

int fcopy(const void *bmp_img, size_t imgsize, char *copyPath, struct stat statbuff)
{
  int fdDest = 0;
  unsigned char c;
  if (fdDest = open(copyPath, 'w', (statbuff.st_mode & S_IRWXU) | (statbuff.st_mode & S_IRWXG) | (statbuff.st_mode & S_IRWXO)) < 0)
  {
    //std::cout << "BAD OPEN %s\n" copyPath;
    return 1;
  }
  while (read(fdSrc, &c, 1) == 1)
  {
    write(fdDest, &c, 1);
  }
  if (fwrite(bmp_img, imgsize, 1, fdDest) != imgsize)
    return -1;
  std::cout << "si";
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
  printf("\t| Image ata size: %u\n", info->imgsize);
  printf("\t| Horizontal resolution: %u\n", info->bpmx);
  printf("\t| Vertical resolution: %u\n", info->bpmy);
  printf("\t| Color palette: %d\n", info->colors);
  printf("\t| Relevant colors (0 all): %d\n", info->imxtcolors);
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

    if (copy != 0 && gauss != 0 && sobel != 0)
      runtimeError(-3, argv[1]);
  }

  DIR *dir_in, *dir_out;
  struct dirent *ent_dir_in;
  struct stat statbuff;
  char *copyPath;
  unsigned int copyPathSize;

  if ((dir_in = opendir(argv[2])) == NULL)
  { /* could not open input directory */
    runtimeError(-4, argv[2]);
  }
  else if ((dir_out = opendir(argv[3])) == NULL)
  { /* could not open output directory */
    runtimeError(-5, argv[3]);
  }
  while ((ent_dir_in = readdir(dir_in)) != NULL)
  {
    std::string bmp_file = ent_dir_in->d_name;

    if (lstat(ent_dir_in->d_name, &statbuff) < 0) //Check file LSTAT
    {
      printf("BAD LSTAT\n");
      return 1;
    }
    if (S_ISREG(statbuff.st_mode) &&
        (statbuff.st_mode & S_IXUSR)) //Check it is A REGULAR FILE WITH EXECUTION rights for the owner
    {
      char *extension = strchr(ent_dir_in->d_name, '.');
      if (extension && (strcmp(extension, ".bmp") == 0)) //Check BMP extension
      {

        std::cout << std::endl
                  << bmp_file << std::endl;
        unsigned char *bmp_img;
        bmpInfoHeader bmp_img_info;

        FILE *source = fopen(ent_dir_in->d_name, "r"); //Open source file

        bmp_img = LoadBMP(source, &bmp_img_info); //Obtain BMP header and file structure

        if (bmp_img == NULL)
          std::cout << ("> Can't open [" + bmp_file + "]\n"); //NO access

        if (checkBMP(&bmp_img_info))
        { //Apply tranformation

          displayBMPInfo(&bmp_img_info);

          if (copy)
          {
            //Copy
            copyPathSize = strlen(argv[3]);
            copyPathSize += 1 + strlen(ent_dir_in->d_name);
            copyPath = (char *)malloc(copyPathSize * sizeof(char));

            strcpy(copyPath, argv[2]);
            strcat(copyPath, ent_dir_in->d_name);

            printf("Copying %s in %s\n", ent_dir_in->d_name, copyPath);

            fcopy(bmp_img, bmp_img_info.imgsize, copyPath, statbuff);

            free(copyPath);
          }
          else if (gauss)
          {
            fgauss();
          }
          else if (sobel)
          {
            fsobel();
          }
          //close(destination);
        }

        //close(source);
      }
    }
    closedir(dir_in);
    closedir(dir_out);
    exit(0);
  }
}