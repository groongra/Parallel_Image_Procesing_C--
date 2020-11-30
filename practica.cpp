/**
 * Actualmente este fichero lo que hace es un ls
*/

#include "iostream" //Importante para poder imprimir por pantalla entre otras cosas
#include <fstream>  //Manejo de ficheros de entrada y salida
#include <cstring>  //Utilizado para comparar Strings
#include <dirent.h> //Manejo entre directorios
#include <cmath>
#include <chrono>

char origen[256];  //El path origen que me han pasado
char destino[256]; //El path origen que me han pasado
int op = -1;

typedef struct infoImagen
{
    char B;                // Array de chars "BM"
    char M;                // Array de chars "BM"
    int sFile;             // Tamaño del fichero
    int reservado;         //Espacio reservado
    int offsetImagen;      //Inicio del contenido de los pixeles de la imagen
    int sCabecera;         // Tamaño de la cabecera
    int anchura;           // Anchura de la imagen
    int altura;            // Altura de la imagen
    short nPlanos;         // Numero de planos de la imagen
    short bitPorPixel;     //Bits por pixeles de la imange
    int compresion;        // Compresion de la imagen
    int sImagen;           // Tamaño total solo de la imagen (altura*anchura*3)
    int rX;                // Resolucion horizontal
    int rY;                // Resolucion vertical
    int sColor;            // Tamaño de la tabla de color
    int colorImportante;   // Colores Importantes
    unsigned char *imagen; // Datos de la imange BMP
} infoImagen;

typedef struct tiempo
{
    int total = 0;
    long loadTime = 0;
    int gaussTime = 0;
    int sobelTime = 0;
    int storeTime = 0;

} tiempo;
using namespace std;

char *obtenerFilePath(char *path, char *fichero);
int operacion(char *fichero, tiempo *time);
infoImagen leerImagen(const char *fileName, short *error);
void escribirImagen(const char *filePathDestino, infoImagen imagen);
infoImagen gauss(infoImagen imagen);
infoImagen sobel(infoImagen datos);
int comprobarBMP(infoImagen datos);
void printError(int tipo, char **argv)
{
    switch (tipo)
    {
    case 0:
        cout << "Wrong format:\n";
        break;
    case 1:
        cout << "Unexpected operation: " << argv[1] << "\n";
        break;
    case 2:
        cout << "Cannot open directory [" << argv[2] << "]\n";
        break;
    case 3:
        cout << "Output directory [" << argv[3] << "] does not exist\n";
        break;
    }
    cout << "  image-seq operation in_path out_path\n    operation: copy, gauss, sobel\n$\n";
}
int main(int argc, char *argv[])
{
    cout << "$image-seq ";
    //Si el número de argumentos que me pasa el programa es menor que 4 es que está mal
    if (argc != 4)
    {
        printError(0, argv);
        return -1;
    }
    cout << argv[1] << " " << argv[2] << " " << argv[3] << "\n";
    //Obtener la operación pasada por argumento
    if (strcmp(argv[1], "copy") == 0)
    {
        op = 1; //En el caso de que el trabajo sea copy
    }
    else if (strcmp(argv[1], "gauss") == 0)
    {
        op = 2; //En el caso de que el trabajo sea gauss
    }
    else if (strcmp(argv[1], "sobel") == 0)
    {
        op = 3; //En el caso de que el trabajo sea sobel
    }
    else
    {
        printError(1, argv);
        return -1;
    }
    memcpy(origen, argv[2], strlen(argv[2]));
    memcpy(destino, argv[3], strlen(argv[3]));
    if (origen[strlen(origen) - 1] != '/')
        strcat(origen, "/"); // En el caso de que no exista la barra en dir origen
    if (destino[strlen(destino) - 1] != '/')
        strcat(destino, "/"); // En el caso de que no exista la barra en dir origen
    cout << "Input path: " << origen << "\n"
         << "Output path: " << destino << "\n";
    struct dirent *eDirOrigen;          // Lee los ficheros que hay en el directorio de origen
    DIR *dirOrigen = opendir(origen);   // Obtengo todos los ficheros del origen
    DIR *dirDestino = opendir(destino); // Obtengo todos los ficheros del destino
    // Debe de existir los dos directorios
    if (dirOrigen == NULL)
    {
        printError(2, argv);
        return -1;
    }
    if (dirDestino == NULL)
    {
        printError(3, argv);
        return -1;
    }

    while ((eDirOrigen = readdir(dirOrigen)) != NULL) // Mientras el elemento que me pase el directorio no sea nulo
    {
        if (strcmp(eDirOrigen->d_name, ".") && strcmp(eDirOrigen->d_name, "..")) // Evito que utilicen como fichero el . y ..
        {
            tiempo time;
            auto start_time = chrono::high_resolution_clock::now();
            if (operacion(eDirOrigen->d_name, &time) == -1) // Tareas de la imagen
                return -1;
            auto end_time = chrono::high_resolution_clock::now();
            time.total = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
            cout << "File: \"" << origen << eDirOrigen->d_name << "\" (time: " << time.total << ")\n"
                 << "  Load time: " << time.loadTime << "\n"
                 << "  Sobel time: " << time.sobelTime << "\n"
                 << "  Gauss time: " << time.gaussTime << "\n"
                 << "  Store time: " << time.storeTime << "\n";
        }
    }
    closedir(dirOrigen);  //Cierro el directorio de origen
    closedir(dirDestino); //Cierro el directorio de destino
    cout << "$\n";
    return 0;
}

/* Esta función realiza la acción indicada por el usuario a cada uno de los archivos */
int operacion(char *fichero, tiempo *time)
{
    char *filePathOrigen = obtenerFilePath(origen, fichero);
    short error = 0;

    /*---------------- Leer Imagen -------------------*/
    auto start_time = chrono::high_resolution_clock::now();
    infoImagen imagenOrigen = leerImagen(filePathOrigen, &error);
    auto end_time = chrono::high_resolution_clock::now();
    time->loadTime = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
    if (error != 0)
        return -1;
    free(filePathOrigen);

    /*---------------- Gauss -------------------*/
    start_time = chrono::high_resolution_clock::now();
    infoImagen imagenDestinoGauss = gauss(imagenOrigen);
    end_time = chrono::high_resolution_clock::now();
    time->gaussTime = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();

    /*---------------- Sobel -------------------*/
    start_time = chrono::high_resolution_clock::now();
    infoImagen imagenDestinoSobel = sobel(imagenOrigen);
    end_time = chrono::high_resolution_clock::now();
    time->sobelTime = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();

    /* Elegir la imagen que se va a Escribir */
    switch (op)
    {
    case 1:
        break;
    case 2:
        imagenOrigen = imagenDestinoGauss; // En el caso de Gauss
        break;
    case 3:
        imagenOrigen = imagenDestinoSobel; // En el caso de Sobel
        break;
    default:
        perror("El programa nunca debería llegar aqui");
        return -1;
    }
    char *filePathDestino = obtenerFilePath(destino, fichero);

    /*---------------- Escribir el nuevo fichero -------------------*/
    start_time = chrono::high_resolution_clock::now();
    escribirImagen(filePathDestino, imagenOrigen);
    end_time = chrono::high_resolution_clock::now();
    time->storeTime = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
    free(imagenOrigen.imagen); //Liberar la imagen leida y sobreescrita
    free(filePathDestino);     //Liberar el path de destino
    return 0;
}

/* Esta funcion obtiene el path donde se encuentra el fichero juntando la carpeta origen y el nombre del archivo */
char *obtenerFilePath(char *path, char *fichero)
{
    char *filePath = (char *)malloc(256);        // Creo un espacio donde guardar los paths a los archivos
    memcpy(filePath, path, strlen(path));        // Copio la carpeta
    strncat(filePath, fichero, strlen(fichero)); // Copio el nombre del fichero
    return filePath;                             // Devuelvo el puntero al path completo hacia el archivo
}

/* Esta función lee la imagen que ha recibido por parámetro y comprueba que todos los parámetros necesarios 
   son correctos. También actualiza los valores anchura y altura pasados por parámetro*/
infoImagen leerImagen(const char *fileName, short *error)
{
    FILE *leerDF = fopen(fileName, "rb"); // Descriptor de fichero de la imagen
    infoImagen tmp;
    fread(&tmp, 1, 2, leerDF);
    fread(&tmp.sFile, sizeof(int), 6, leerDF);
    fread(&tmp.nPlanos, sizeof(short), 2, leerDF);
    fread(&tmp.compresion, sizeof(int), 6, leerDF);
    if (comprobarBMP(tmp) != 0)
    {
        *error = 1;
        return tmp;
    }
    int unpaddedRowSize = tmp.anchura * 3;
    int paddedRowSize;
    if (unpaddedRowSize % 4 != 0)
        paddedRowSize = unpaddedRowSize + (4 - (unpaddedRowSize % 4));
    else
        paddedRowSize = unpaddedRowSize;
    cout<<paddedRowSize<<" | "<<unpaddedRowSize<<"\n";

    int totalSize = unpaddedRowSize * tmp.altura;
    tmp.imagen = (unsigned char *)malloc(totalSize);

    unsigned char *currentRowPointer = tmp.imagen + ((tmp.altura - 1) * unpaddedRowSize);
	for (int i = 0; i < tmp.altura; i++)
	{
		fseek(leerDF, tmp.offsetImagen + (i * paddedRowSize), SEEK_SET);
		fread(currentRowPointer, 1, unpaddedRowSize, leerDF);
		currentRowPointer -= unpaddedRowSize;
	}
    fclose(leerDF); // Cierro el descriptor de fichero
    return tmp;
}

/*Esta funcion obtiene todos los parametros necesarios para leer una imagen y escribe la imagen pasada por argumento
   ademas del lugar donde guardarla, la altura y la anchura*/
void escribirImagen(const char *fileName, infoImagen imagen)
{
    FILE *escribirDF = fopen(fileName, "wb");
    // Escribir cada uno de los parámetros de la cabecera
    fwrite(&imagen, 1, 2, escribirDF);                      // Escribo BM
    fwrite(&imagen.sFile, sizeof(int), 6, escribirDF);      // Escribo los siguientes enteros de la cabecera
    fwrite(&imagen.nPlanos, sizeof(short), 2, escribirDF);  // Escribo los shorts de la cabecera
    fwrite(&imagen.compresion, sizeof(int), 6, escribirDF); // Escribo los últimos enteros de la cabecera
    fseek(escribirDF, imagen.offsetImagen, SEEK_SET);       // Establezco la posición donde se escribe la imagen
    int unpaddedRowSize = imagen.anchura * 3;
    int paddedRowSize ;
    if (unpaddedRowSize % 4 != 0)
        paddedRowSize = unpaddedRowSize + (4 - (unpaddedRowSize % 4));
    else
        paddedRowSize = unpaddedRowSize;

    cout<<paddedRowSize<<" | "<<unpaddedRowSize<<"\n";
    for (int i = 0; i < imagen.altura; i++)
    {
        int pixelOffset = ((imagen.altura - i) - 1) * unpaddedRowSize;
        fwrite(&imagen.imagen[pixelOffset], 1, paddedRowSize, escribirDF);
    }
    fclose(escribirDF); // Cierro el descriptor de fichero de escribir
}

int comprobarBMP(infoImagen datos)
{

    if (datos.B != 'B' || datos.M != 'M')
    {
        perror("El archivo no es BMP");
        return -1;
    }
    else if (datos.nPlanos != 1)
    {
        perror("El número de planos es distinto de 1");
        return -1;
    }
    else if (datos.bitPorPixel != 24)
    {
        perror("El número de bits por pixel no es 24");
        return -1;
    }
    else if (datos.compresion != 0)
    {
        perror("La compresion del archivo no es 0");
        return -1;
    }
    return 0;
}

infoImagen gauss(infoImagen datos)
{
    infoImagen tmp = datos;
    tmp.imagen = (unsigned char *)malloc(tmp.sImagen);
    if (op != 2)
        free(tmp.imagen);
    return tmp;
}

infoImagen sobel(infoImagen datos)
{
    infoImagen tmp = datos;
    tmp.imagen = (unsigned char *)malloc(tmp.sImagen);
    if (op != 3)
        free(tmp.imagen);
    return tmp;
}