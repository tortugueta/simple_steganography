#include "bmpstructures.h"
#include <stdlib.h>
#include <stdio.h>

int bmpread(char* filename, bmpfile* imatge) {

	int i, j;	
	FILE *file;	

	/* Start reading the file */
	
	file = fopen(filename, "rb");
	if (file == NULL) {
		return 1;
	}

	/* Read general file and bitmap information */
	
	fread(&imatge->lletra1, sizeof(char), 1, file);
	fread(&imatge->lletra2, sizeof(char), 1, file);
	if ((imatge->lletra1 != 'B') | (imatge->lletra2 != 'M')) {
		fclose(file);
		return 2;
	}
	fread(&imatge->filesize, sizeof(unsigned int), 1, file);
	fread(&imatge->empty, sizeof(unsigned int), 1, file);
	fread(&imatge->offset, sizeof(unsigned int), 1, file);
	fread(&imatge->bytesheader, sizeof(unsigned int), 1, file);
	fread(&imatge->width, sizeof(unsigned int), 1, file);
	fread(&imatge->height, sizeof(unsigned int), 1, file);
	fread(&imatge->colorplanes, sizeof(short int), 1, file);
	fread(&imatge->bitsperpixel, sizeof(short int), 1, file);
	if (imatge->bitsperpixel != 8) {
		fclose(file);
		return 3;
	}
	fread(&imatge->compression, sizeof(unsigned int), 1, file);
	fread(&imatge->bmpsize, sizeof(unsigned int), 1, file);
	fread(&imatge->hres, sizeof(unsigned int), 1, file);
	fread(&imatge->vres, sizeof(unsigned int), 1, file);
	fread(&imatge->numcolors, sizeof(unsigned int), 1, file);
	fread(&imatge->importantcolors, sizeof(unsigned int), 1, file);
	
	/* Read the color table */
	imatge->colortable = malloc(256 * sizeof(unsigned char*));
	imatge->colortable[0] = malloc(256*4 * sizeof(unsigned char));
	for (i=1; i<256; i++) {
		imatge->colortable[i] = imatge->colortable[i-1] + 4;
	}	
	
	for (i=0; i<256; i++) {
		for (j=0; j<4; j++) {
			fread(&imatge->colortable[i][j], sizeof(unsigned char), 1, file);
		}
	}
	
	/* Determine the padding needed to achieve rows with a multiple-of-four number of bytes and start reading the actual bitmap. First allocate memory to store the pixel values */
	
	imatge->bitmap = malloc(imatge->height * sizeof(unsigned char*));
	imatge->bitmap[0] = malloc(imatge->height*imatge->width * sizeof(unsigned char));
	for (i=1; i<imatge->height; i++) {
		imatge->bitmap[i] = imatge->bitmap[i-1] + imatge->width;
	}
	
	if ((imatge->width % 4) == 0) {
		imatge->numpading = 0;
	}
	else {
		imatge->numpading = 4 - (imatge->width % 4);
	}
	for (i=imatge->height-1; i>=0; i--) {
		for (j=0; j<imatge->width; j++) {
			fread(&imatge->bitmap[i][j], sizeof(unsigned char), 1, file);
		}
		fseek(file, imatge->numpading*sizeof(unsigned char), SEEK_CUR);
	}

	fclose(file);	

	return 0;
}

void bmpwrite(char* filename, unsigned char** bitmap, int rows, int columns) {

	FILE *file;
	bmpfile imatge;
	int i, j;
	unsigned char* numpadvector;
	
	/* Open file */

	file = fopen(filename, "wb");

	/* Fill all information about the bmp file */
	
	/* The first two letters identifying the file as a bmp */
	imatge.lletra1 = 'B';
	imatge.lletra2 = 'M';
	
	/* Calculate the total file size */
	imatge.height = rows;
	imatge.width = columns;
	if ((imatge.width % 4) == 0) {
		imatge.numpading = 0;
	}
	else {
		imatge.numpading = 4 - (imatge.width % 4);
	}
	imatge.filesize = 54 + 1024 + imatge.height * (imatge.width + imatge.numpading); // 54 Bytes for the header, 1024 Bytes for the color table and the remaining for the actual bitmap, padding included.
	
	/* Set the 4 unused application specific bits to zero */
	imatge.empty = 0;
	
	/* Offset where the bitmap data can be found is 54 Bytes of the header plus 1024 Bytes of the color table. */
	imatge.offset = 54 + 1024;
	
	/* Number of Bytes of the header, between the offset and the color table */
	imatge.bytesheader = 40;
	
	/* Number of color planes being used */
	imatge.colorplanes = 1;
	
	/* Bits per pixel */
	imatge.bitsperpixel = 8;
	
	/* Compression. 0 means no compression at all */
	imatge.compression = 0;
	
	/* Size of the raw bmp data, including the padding */
	imatge.bmpsize = imatge.height * (imatge.width + imatge.numpading);
	
	/* Horizontal and vertical resolution (in pixels per meter). 2835 is a typical number */
	imatge.hres = 2835;
	imatge.vres = 2835;
	
	/* Number of colors in the palette (color table) and number of important colors. In a grayscale bitmap we have 256 colors and all of them are 'important', whatever that means */
	imatge.numcolors = 256;
	imatge.importantcolors = 256;
	
	/* Build the color table. The table will be written to disk in a single shot. */
	
	imatge.colortable = malloc(256 * sizeof(unsigned char*));
	imatge.colortable[0] = malloc(256*4 * sizeof(unsigned char));
	for (i=1; i<256; i++) {
		imatge.colortable[i] = imatge.colortable[i-1] + 4;
	}
		
	for (i=0; i<256; i++) {
		for (j=0; j<3; j++) {
			imatge.colortable[i][j] = i;
		}
		imatge.colortable[i][3] = 0;
	}
	
	/* The bitmap will be written directly to disk, bearing in mind that we have to add the necessary padding and that the lower row goes first. Here we build a vector of numpading unsigned chars filled with zeros that we will use to add the padding. */
	
	numpadvector = malloc(imatge.numpading * sizeof(unsigned char));
	for (i=0; i<imatge.numpading; i++) {
		numpadvector[i] = 0;
	}
	
	/* Write to disk */
	
	fwrite(&imatge.lletra1, sizeof(char), 1, file);
	fwrite(&imatge.lletra2, sizeof(char), 1, file);
	fwrite(&imatge.filesize, sizeof(unsigned int), 1, file);
	fwrite(&imatge.empty, sizeof(unsigned int), 1, file);
	fwrite(&imatge.offset, sizeof(unsigned int), 1, file);
	fwrite(&imatge.bytesheader, sizeof(unsigned int), 1, file);
	fwrite(&imatge.width, sizeof(unsigned int), 1, file);
	fwrite(&imatge.height, sizeof(unsigned int), 1, file);
	fwrite(&imatge.colorplanes, sizeof(short int), 1, file);
	fwrite(&imatge.bitsperpixel, sizeof(short int), 1, file);
	fwrite(&imatge.compression, sizeof(unsigned int), 1, file);
	fwrite(&imatge.bmpsize, sizeof(unsigned int), 1, file);
	fwrite(&imatge.hres, sizeof(unsigned int), 1, file);
	fwrite(&imatge.vres, sizeof(unsigned int), 1, file);
	fwrite(&imatge.numcolors, sizeof(unsigned int), 1, file);
	fwrite(&imatge.importantcolors, sizeof(unsigned int), 1, file);
	fwrite(imatge.colortable[0], sizeof(unsigned char), 256*4, file);
	for (i=imatge.height-1; i>=0; i--) {
		fwrite(bitmap[i], sizeof(unsigned char), imatge.width, file);
		fwrite(numpadvector, sizeof(unsigned char), imatge.numpading, file);
	}

	fclose(file);	

	/* Free memory */
	free(imatge.colortable[0]);
	free(imatge.colortable);
	free(numpadvector);	
}
