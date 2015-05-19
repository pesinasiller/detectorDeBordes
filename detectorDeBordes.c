#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h> //AGREGAMOS LIBRERIA POSIX 
#define NUM_THREADS 4 //número de hilos 
#define DIF 16
// NOMBRE DEL ARCHIVO A PROCESAR
char filename[]="imagen.bmp";

#pragma pack(2) // Empaquetado de 2 bytes
typedef struct {
	 unsigned char magic1; // 'B'
	 unsigned char magic2; // 'M'
	 unsigned int size; // Tamaño
	 unsigned short int reserved1, reserved2;
	 unsigned int pixelOffset; // offset a la imagen
} HEADER;

#pragma pack() // Empaquetamiento por default
typedef struct {
	 unsigned int size; // Tamaño de este encabezado INFOHEADER
	 int cols, rows; // Renglones y columnas de la imagen
	 unsigned short int planes;
	 unsigned short int bitsPerPixel; // Bits por pixel
	 unsigned int compression;
	 unsigned int cmpSize;
	 int xScale, yScale;
	 unsigned int numColors;
	 unsigned int importantColors;
} INFOHEADER;

typedef struct {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} PIXEL;

typedef struct {
	HEADER header;
	INFOHEADER infoheader;
	PIXEL *pixel;
} IMAGE;
IMAGE imagenfte,imagendst;


int loadBMP(char *filename, IMAGE *image)
{
	 FILE *fin;
	 int i=0;
	 int totpixs=0;
	 fin = fopen(filename, "rb+");
	 // Si el archivo no existe
	 if (fin == NULL)
		return(-1);
	 // Leer encabezado
	 fread(&image->header, sizeof(HEADER), 1, fin);
	 // Probar si es un archivo BMP
	 if (!((image->header.magic1 == 'B') && (image->header.magic2 == 'M')))
		return(-1);
	 fread(&image->infoheader, sizeof(INFOHEADER), 1, fin);
	 // Probar si es un archivo BMP 24 bits no compactado
	 if (!((image->infoheader.bitsPerPixel == 24) && !image->infoheader.compression))
		 return(-1);
	 image->pixel=(PIXEL *)malloc(sizeof(PIXEL)*image->infoheader.cols*image->infoheader.rows);
	 totpixs=image->infoheader.rows*image->infoheader.cols;

	 while(i<totpixs)
	 {
		fread(image->pixel+i, sizeof(PIXEL),512, fin);
		i+=512;
	 }
 	fclose(fin);

}

int saveBMP(char *filename, IMAGE *image)
{
	 FILE *fout;
	 int i,totpixs;

	 fout=fopen(filename,"wb");
	 if (fout == NULL)
		 return(-1); // Error

	 // Escribe encabezado
	 fwrite(&image->header, sizeof(HEADER), 1, fout);
	 // Escribe información del encabezado
	 fwrite(&image->infoheader, sizeof(INFOHEADER), 1, fout);
	 i=0;
	 totpixs=image->infoheader.rows*image->infoheader.cols;
	 while(i<totpixs)
	 {
		fwrite(image->pixel+i,sizeof(PIXEL),512,fout);
		i+=512;
	 }
	 fclose(fout);
}

unsigned char blackandwhite(PIXEL p)
{
	return((unsigned char) (0.3*((float)p.red)+0.59*((float)p.green)+0.11*((float)p.blue)));
}
int imageRows, imageCols;



void *processBMP(void *arg)
{
/*************** se recibe el número de hilo y se toman las imagenes fuente y destino*****************/
	int threadID=*((int *) arg); 
	IMAGE *imagefte=&imagenfte;
	IMAGE *imagedst=&imagendst;
/********************************/
	int i,j;
	int count=0;
	PIXEL *pfte,*pdst;
	PIXEL *v0,*v1,*v2,*v3,*v4,*v5,*v6,*v7;

	/* comienza desde el id del hilo y va aumentando según el número de hilos */
	for(i=threadID;i<imageRows-1;i+=NUM_THREADS)
		for(j=1;j<imageCols-1;j++)
		{	
			pfte=imagefte->pixel+imageCols*i+j;
			v0=pfte-imageCols-1;
			v1=pfte-imageCols;
			v2=pfte-imageCols+1;
			v3=pfte-1;
			v4=pfte+1;
			v5=pfte+imageCols-1;
			v6=pfte+imageCols;
			v7=pfte+imageCols+1;
			pdst=imagedst->pixel+imageCols*i+j;
			unsigned char pix_actual = blackandwhite(*pfte); 
			if(abs(pix_actual-blackandwhite(*v0))>DIF ||
			 abs(pix_actual-blackandwhite(*v1))>DIF ||
			 abs(pix_actual-blackandwhite(*v2))>DIF ||
			 abs(pix_actual-blackandwhite(*v3))>DIF ||
			 abs(pix_actual-blackandwhite(*v4))>DIF ||
			 abs(pix_actual-blackandwhite(*v5))>DIF ||
			 abs(pix_actual-blackandwhite(*v6))>DIF ||
			 abs(pix_actual-blackandwhite(*v7))>DIF)
			{
				pdst->red=0;
				pdst->green=0;
				pdst->blue=0;
			}
			else
			{
				pdst->red=255;
				pdst->green=255;
				pdst->blue=255;
			}
		}
}



int main()
{	int res;
	char namedest[80];
	long long start_ts;
	long long stop_ts;
	long long elapsed_time;
	long lElapsedTime;
	struct timeval ts;
	gettimeofday(&ts, NULL);
	start_ts = ts.tv_sec * 1000000 + ts.tv_usec; // Tiempo inicial


	strcpy(namedest,strtok(filename,"."));
	strcat(filename,".bmp");
	strcat(namedest,"_P.bmp");
	printf("Archivo fuente %s\n",filename);
	printf("Archivo destino %s\n",namedest);
	res=loadBMP(filename,&imagenfte);
	if(res==-1)
	{
		fprintf(stderr,"Error al abrir imagen\n");
		exit(1);
	}

	printf("Procesando imagen de: Renglones = %d, Columnas =%d\n",imagenfte.infoheader.rows,imagenfte.infoheader.cols);

/* 
se inicializa la imagen
*/

       	IMAGE *imagedst = &imagendst;
        IMAGE *imagefte = &imagenfte;
	memcpy(imagedst,imagefte,sizeof(IMAGE)-sizeof(PIXEL *));
	imageRows = imagefte->infoheader.rows;
	imageCols = imagefte->infoheader.cols;
	imagedst->pixel=(PIXEL *)malloc(sizeof(PIXEL)*imageRows*imageCols);
	

	
	pthread_t tid[NUM_THREADS]; //se crean los hilos
	int args[NUM_THREADS]; //argumentos que se enviarán a la función por cada hilo
	int i;
	for (i = 0; i < NUM_THREADS; i++)
	{
		args[i]=i;
		pthread_create(&tid[i], NULL, processBMP,(void *) &args[i]); //se llama a la función con cada hilo
	}

	
	 for (i = 0; i < NUM_THREADS; i++) //se terminan los hilos
    		pthread_join(tid[i], NULL);



	res=saveBMP(namedest,&imagendst);
	if(res==-1)
	{
		fprintf(stderr,"Error al escribir imagen\n");
		exit(1);
	}
	gettimeofday(&ts, NULL);
	stop_ts = ts.tv_sec * 1000000 + ts.tv_usec; // Tiempo final
	elapsed_time = stop_ts-start_ts;
	printf("Tiempo%2.3fsegundos\n",(float)elapsed_time/1000000.0);
}
