
#include "bmp.h"

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

void TextDisplay(bmpInfoHeader *info, unsigned char *img)
{
  int x, y;
  /* Reducimos la resolución vertical y horizontal para que la imagen entre en pantalla */
  static const int reduccionX=6, reduccionY=4;
  /* Si la componente supera el umbral, el color se marcará como 1. */
  static const int umbral=90;
  /* Asignamos caracteres a los colores en pantalla */
  static unsigned char colores[9]=" bgfrRGB";
  int r,g,b;

  /* Dibujamos la imagen */
  for (y=info->height; y>0; y-=reduccionY)
    {
      for (x=0; x<info->width; x+=reduccionX)
    {
      b=(img[3*(x+y*info->width)]>umbral);
      g=(img[3*(x+y*info->width)+1]>umbral);
      r=(img[3*(x+y*info->width)+2]>umbral);

      printf("%c", colores[b+g*2+r*4]);
    }
      printf("\n");
    }
}
