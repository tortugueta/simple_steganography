/* This header contains declaration of a suitable struct to handle all information related to a bmp file */

typedef struct {
	char lletra1;
	char lletra2;
	unsigned int filesize;
	unsigned int empty;
	unsigned int offset;
	unsigned int bytesheader;
	unsigned int width;
	unsigned int height;
	short int colorplanes;
	short int bitsperpixel;
	unsigned int compression;
	unsigned int bmpsize;
	unsigned int hres;
	unsigned int vres;
	unsigned int numcolors;
	unsigned int importantcolors;
	unsigned char **colortable;
	int numpading;
	unsigned char **bitmap;
} bmpfile;

