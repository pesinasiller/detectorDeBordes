all: detectorDeBordes.c 
	gcc -o detectorDeBordes detectorDeBordes.c -lpthread
