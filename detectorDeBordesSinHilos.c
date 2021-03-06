#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#define DIF 16
// NOMBRE DEL ARCHIVO A PROCESAR
char filename[]="imagen.bmp";

#pragma pack(2) // Empaquetado de 2 bytes
typedef struct {
	 unsigned char magic1; // 'B'
	 unsigned char magic2; // 'M'
	 unsigned int size; // Tama�o
	 unsigned short int reserved1, reserved2;
	 unsigned int pixelOffset; // offset a la imagen
} HEADER;

#pragma pack() // Empaquetamiento por default
typedef struct {
	 unsigned int size; // Tama�o de este encabezado INFOHEADER
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
	 // Escribe informaci�n del encabezado
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

void processBMP(IMAGE *imagefte, IMAGE *imagedst)
{
	int i,j;
	int count=0;
	PIXEL *pfte,*pdst;
	PIXEL *v0,*v1,*v2,*v3,*v4,*v5,*v6,*v7;
	int imageRows,imageCols;
	memcpy(imagedst,imagefte,sizeof(IMAGE)-sizeof(PIXEL *));
	imageRows = imagefte->infoheader.rows;
	imageCols = imagefte->infoheader.cols;
	imagedst->pixel=(PIXEL *)malloc(sizeof(PIXEL)*imageRows*imageCols);
	for(i=1;i<imageRows-1;i++)
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
//se evalua el pixel actual solo una vez para hacer las comparaciones
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
{
	int res;
	clock_t t_inicial,t_final;
	char namedest[80];
	t_inicial=clock();
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
	processBMP(&imagenfte,&imagendst);
	res=saveBMP(namedest,&imagendst);
	if(res==-1)
	{
		fprintf(stderr,"Error al escribir imagen\n");
		exit(1);
	}
	t_final=clock();
	printf("Tiempo %3.6f segundos\n",((float) t_final-(float)t_inicial)/CLOCKS_PER_SEC);
}
