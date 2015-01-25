/* This program asks for a bmp file and a message and encodes the message in the least significant bit of the pixels of the image. Then writes the image in the file 'encoded_picture.bmp' */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpstructures.h"
#include "bmputils.h"

#define STRLENGTH 252

int encode() {
	
	char original_filename[STRLENGTH+1];
	char destination_filename[STRLENGTH+4+1] = "enc_"; // We will prepend 'enc_' to the original filename
	char *message;
	char res;
	int i, k, n, err, numchars;
	bmpfile picture;
	
	/* Ask for the file to read. Retrieve the information of the file */
		
	printf("Enter the file where the message will be encoded: ");
	fgets(original_filename, STRLENGTH, stdin);
		
	/* We should remove the end of line '\n' from the string. Otherwise when we pass it to fopen() the '\n' will belong to the string and the file will not be found */
	for (i=0; original_filename[i] != '\0'; i++) {
		if (original_filename[i] == '\n') {
			original_filename[i] = '\0';
		}
	}
		
	err = bmpread(original_filename, &picture);
	if (err != 0) {
		if (err == 1) {
			printf("File not found. Exit.\n");
		}
		else if (err == 2) {
			printf("Not really a bitmap. Exit.\n");
		}
		else if (err == 3) {
			printf("Not an 8 bit greyscale bitmap. Exit.\n");
		}
		return 1;
	}
		
	/* Ask for the message to be encoded in the picture. The amount of characters of the message multiplied by 8 must not exeed the number of pixels of our picture */
		
	numchars = picture.height * picture.width / 8; // numchars gets the integer part so the remainder is automatically discarded
	message = malloc(numchars * sizeof(char));
	printf("Enter the message to be encoded (max %d characters): ", numchars-2); // One of the characters is the end return and the other one the '\0' end of string.
	fgets(message, numchars-2, stdin);
		
		/* Put each bit of the message in a pixel of the picture. We will use the following statements:
	res = x & (1 << n) // res is zero if the (n+1)th bit of x is zero
				// res is different from zero if the (n+1)th bit of x is one
	x = x | (1 << n)   // sets the (n+1)th bit to 1 
	x = x & ~(1 << n)  // sets the (n+1)th bit to 0 */
		
	k = 0;
	for (i=0; message[i] != '\0'; i++) {
		for (n=7; n>=0; n--) {
			res = message[i] & (1 << n);
			if (res == 0) { // The bit was 0 so we set the first bit of the kth pixel to 0
				picture.bitmap[0][k] = picture.bitmap[0][k] & ~(1 << 0);
			}
			else {		// The bit was 1 so we set the first bit of the kth pixel to 1
				picture.bitmap[0][k] = picture.bitmap[0][k] | (1 << 0);
			}
			k = k+1;
		}
	}
	for (n=7; n>=0; n--) { // We also encode the '\0' to mark the end of the message.
		picture.bitmap[0][k] = picture.bitmap[0][k] & ~(1 << 0);
		k = k+1;
	}
		
	/* Write the new picture to disk */
		
	strcat(destination_filename, original_filename);
	bmpwrite(destination_filename, picture.bitmap, picture.height, picture.width);
			
	printf("Message successfully encoded to %s\n", destination_filename);
		
	/* Free memory */
		
	free(picture.colortable[0]);
	free(picture.colortable);
	free(picture.bitmap[0]);
	free(picture.bitmap);
	free(message);
	
	return 0;
}

int decode() {
	
	char original_filename[STRLENGTH+1];
	char *message;
	char res;
	int i, k, n, err, numchars;
	bmpfile picture;
		
	/* Ask for the file to decode and read it */
		
	printf("Enter the bmp file you want to read: ");
	fgets(original_filename, STRLENGTH, stdin);
		
	/* We should remove the end of line '\n' from the string. Otherwise when we pass it to fopen() the '\n' will belong to the string and the file will not be found */
	for (i=0; original_filename[i] != '\0'; i++) {
		if (original_filename[i] == '\n') {
			original_filename[i] = '\0';
		}
	}
		
	err = bmpread(original_filename, &picture);
	if (err != 0) {
		if (err == 1) {
			printf("File not found. Exit.\n");
		}
		else if (err == 2) {
			printf("Not really a bitmap. Exit.\n");
		}
		else if (err == 3) {
			printf("Not an 8 bit greyscale bitmap. Exit.\n");
		}
		return 1;
	}
		
	/* Retrieve the message */
		
	numchars = picture.height * picture.width / 8; // numchars gets the integer part so the remainder is automatically discarded
	message = malloc(numchars * sizeof(char));
		
	i = 0;
	k = 0;
	for (n=7; n>=0; n--) {
		res = picture.bitmap[0][k] & (1 << 0);
		if (res == 0) { // The bit was 0
			message[i] = message[i] & ~(1 << n);
		}
		else {		// The bit was 1
			message[i] = message[i] | (1 << n);
		}
		k = k+1;
	}
	i = i+1;
	while ((i<numchars) && (message[i-1] != '\0')) {
		for (n=7; n>=0; n--) {
			res = picture.bitmap[0][k] & (1 << 0);
			if (res == 0) {
				message[i] = message[i] & ~(1 << n);
			}
			else {
				message[i] = message[i] | (1 << n);
			}
			k = k+1;
		}
		i = i+1;
	}
		
	printf("The encoded message is: ");
	printf("%s\n", message);
		
	/* Free memory */
		
	free(picture.colortable[0]);
	free(picture.colortable);
	free(picture.bitmap[0]);
	free(picture.bitmap);
	free(message);
	
	return 0;
}

int main() {

	char answer, dumchar;
	
	/* Ask what to do */
	
	do {
		printf("What do you want to do?\n");
		printf("a) Encode a message in a bmp file\n");
		printf("b) Retrieve a message from a bmp file\n");
		answer = fgetc(stdin);
		while ((dumchar = fgetc(stdin)) != EOF) {	// Read until past the '\n' to put the read header of stdin just before the EOF (otherwise the next fgets will not read what is expected to read.
			if (dumchar == '\n') {
				break;
			}
		}
		if ((answer != 'a') && (answer != 'b')) {
			printf("Not a valid option, try again\n\n");
		}
	} while ((answer != 'a') && (answer != 'b'));
		
	if (answer == 'a') {
		encode();
	}
	else if (answer == 'b') {
		decode();
	}
	
	return 0;
}
