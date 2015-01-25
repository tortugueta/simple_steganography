/* This module contains functions to read and write bmp files */

extern int bmpread(char* filename, bmpfile* imatge);
	
	/* Receives the file name of an 8-bit grayscale bmp file and a bmpfile struct. The function fills all the fields of the received struct.
	The return value is 0 if everything goes fine.
	If the file does not exist the return value is 1.
	If the file is not a bitmap the return value is 2.
	If the file is a bitmap but not an 8 bit grayscale the return value is 3.

	Beware! This function allocates memory for the color table and the bitmap. In the calling function, afther the bmpfile struct is no longer needed four free() are in order:
	free(imatge.colortable[0]);
	free(imatge.colortable);
	free(imatge.bitmap[0]);
	free(imatge.bitmap);
	*/

extern void bmpwrite(char* filename, unsigned char** bitmap, int rows, int columns);

	/* Receives the file name of the bmp file to be written and the address to a matrix of unsigned chars (it must be passed as unsigned char**, no matrix[][] allowed) along with its size. The function will generate a grayscale bmp file from the matrix. */
