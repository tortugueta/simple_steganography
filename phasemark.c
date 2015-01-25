#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>
#include "bmpstructures.h"
#include "bmputils.h"

#define STRLENGTH 252
#define PHASE 1.0

unsigned char** allocate_uchar_matrix(int rows, int columns) {
	
	unsigned char **matrix;
	int i;
	
	matrix = malloc(rows * sizeof(unsigned char*));
	matrix[0] = malloc(rows*columns * sizeof(unsigned char));
	for (i=1; i<rows; i++) {
		matrix[i] = matrix[i-1] + columns;
	}
	
	return matrix;	
}

double** allocate_double_matrix(int rows, int columns) {
	
	double** matrix;
	int i;
	
	matrix = malloc(rows * sizeof(double*));
	matrix[0] = malloc(rows*columns * sizeof(double));
	for (i=1; i<rows; i++) {
		matrix[i] = matrix[i-1] + columns;
	}
	
	return matrix;	
}

complex** allocate_complex_matrix(int rows, int columns) {
	
	complex** matrix;
	int i;
	
	matrix = malloc(rows * sizeof(complex*));
	matrix[0] = malloc(rows*columns * sizeof(complex));
	for (i=1; i<rows; i++) {
		matrix[i] = matrix[i-1] + columns;
	}
	
	return matrix;
}

complex addphase(complex z, double phase_deg) {
	
/* Input values
	- z: a complex number.
	- phase_deg: an angle in degrees.

   Output values
	- zprime = the complex number z rotated phase_deg degrees.
*/
	
	complex zprime;
	double phase_rad, arg;
	
	phase_rad = M_PI * phase_deg / 180;
	arg = carg(z);
	zprime = cabs(z) * (ccos(arg + phase_rad) + I*csin(arg + phase_rad));
	
	return zprime;
	
}

void difference(unsigned char** a, unsigned char** b, unsigned char** c, int rows, int columns) {
	
	int i, j;
	
	for (i=0; i<rows; i++) {
		for (j=0; j<columns; j++) {
			c[i][j] = (unsigned char) fabs((float) a[i][j] - (float) b[i][j]);
		}
	}
}

int main () {
	
	int i, j, err, dft_output_width;
	char original_filename[STRLENGTH+1];
	char destination_filename[STRLENGTH+4+1] = "enc_"; // Prepend 'enc_' to the original filename
	complex **dft_output;
	complex **dft_output_control;
	double **dft_input;
	double **output_bitmap_control;
	double **phasedif;
	unsigned char original_max;
	double dft_input_max, dft_input_min;
	unsigned char **output_bitmap;
	unsigned char **difference_bitmap;
	fftw_plan dft_forth, dft_back, dft_forth_control;
	bmpfile picture;
	
	/* Get the picture we want to encode */
	
	printf("Enter the source file: ");
	fgets(original_filename, STRLENGTH, stdin);
	for (i=0; original_filename[i] != '\0'; i++) {	// Remove the trailing newline '\n'
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

	/* Transform the pixel matrix with a DFT. Note that the DFT routine requires the input matrix to be double (if we want to use the real to complex transform) */
	
	/* Allocate memory for the input and output matrices. We need to cast the chars of the pixel matrix into a matrix of doubles. The output matrix is smaller in dimension because of the Hermitian redundancy */
	
	dft_input = allocate_double_matrix(picture.height, picture.width);
	dft_output_width = (int) (picture.width/2 + 1);
	dft_output = allocate_complex_matrix(picture.height, dft_output_width);
	
	/* Create the plans */
	
	dft_forth = fftw_plan_dft_r2c_2d(picture.height, picture.width, dft_input[0], dft_output[0], FFTW_ESTIMATE);
	dft_back = fftw_plan_dft_c2r_2d(picture.height, picture.width, dft_output[0], dft_input[0], FFTW_ESTIMATE);
	
	/* Cast the pixels to double */
	
	for (i=0; i<picture.height; i++) {
		for (j=0; j<picture.width; j++) {
			dft_input[i][j] = (double) picture.bitmap[i][j];
		}
	}
	
	/* Execute the transform */
	
	fftw_execute(dft_forth);
	
	/* Manipulate the transform */
	
	for (i=0; i<picture.height; i++) {
		for (j=0; j<dft_output_width; j++) {
			dft_output[i][j] = addphase(dft_output[i][j], PHASE);
		}
	}

	/* Transform back */
	
	fftw_execute(dft_back);
	
	/* Normalize. We do not reuse the initial char matrix so as not to lose the initial values and thus be able to perform comparisons between the input and output matrices. */
	
	/* Allocate memory for the final output matrix */
	
	output_bitmap = allocate_uchar_matrix(picture.height, picture.width);
	
	/* Find maximum of the original picture and both maximum and minimum of the modified picture */
	
	original_max = picture.bitmap[0][0];
	for (i=0; i<picture.height; i++) {
		for (j=0; j<picture.width; j++) {
			if (picture.bitmap[i][j] > original_max) {
				original_max = picture.bitmap[i][j];
			}
		}
	}
	dft_input_max = dft_input[0][0];
	for (i=0; i<picture.height; i++) {
		for (j=0; j<picture.width; j++) {
			if (dft_input[i][j] > dft_input_max) {
				dft_input_max = dft_input[i][j];
			}
		}
	}
	dft_input_min = dft_input[0][0];
	for (i=0; i<picture.height; i++) {
		for (j=0; j<picture.width; j++) {
			if (dft_input[i][j] < dft_input_min) {
				dft_input_min = dft_input[i][j];
			}
		}
	}
	
	/* Normalize */
	
	for (i=0; i<picture.height; i++) {
		for (j=0; j<picture.width; j++) {
			output_bitmap[i][j] = (unsigned char) rint(original_max * ((dft_input[i][j] - dft_input_min) / (dft_input_max - dft_input_min)));
		}
	}
	
	/* Write modified picture */
	
	strcat(destination_filename, original_filename);
	bmpwrite(destination_filename, output_bitmap, picture.height, picture.width);
	
	/* Compute difference between original and final images */
	
	difference_bitmap = allocate_uchar_matrix(picture.height, picture.width);
	difference(picture.bitmap, output_bitmap, difference_bitmap, picture.height, picture.width);
	bmpwrite("difference.bmp", difference_bitmap, picture.height, picture.width);
	
	/* Compute the DFT of the final bitmap and then compare the difference between the phases of the original picture and the phases that we get upon transformation. If we didn't loose any information when normalizing the difference should be precisely the phase that we intentionally added. Since the picture is changed during the normalization we will get something different. */
	
	output_bitmap_control = allocate_double_matrix(picture.height, picture.width);
	dft_output_control = allocate_complex_matrix(picture.height, dft_output_width);
	dft_forth_control = fftw_plan_dft_r2c_2d(picture.height, picture.width, output_bitmap_control[0], dft_output_control[0], FFTW_ESTIMATE);
		
	for (i=0; i<picture.height; i++) {
		for (j=0; j<picture.width; j++) {
			output_bitmap_control[i][j] = (double) output_bitmap[i][j];
		}
	}
	
	fftw_execute(dft_forth_control);
	
	/* Calculate the difference between the phases of dft_output_control and dft_output. Ideally it should be zero */
	
	phasedif = allocate_double_matrix(picture.height, dft_output_width);
	
	for (i=0; i<picture.height; i++) {
		for (j=0; j<dft_output_width; j++) {
			phasedif[i][j] = (180/M_PI) * (carg(dft_output_control[i][j]) - carg(dft_output[i][j]));
			if (phasedif[i][j] > 180) {
				phasedif[i][j] = phasedif[i][j] - 360;
			}
			else if (phasedif[i][j] < -180) {
				phasedif[i][j] = phasedif[i][j] + 360;
			}
		}
	}
	
	for (i=0; i<picture.height; i++) {
		for (j=0; j<dft_output_width; j++) {
			printf("%f\n", phasedif[i][j]);
		}
	}
		
	/* Free memory */
	
	free(picture.colortable[0]);
	free(picture.colortable);
	free(picture.bitmap[0]);
	free(picture.bitmap);
	free(dft_input[0]);
	free(dft_input);
	free(dft_output[0]);
	free(dft_output);
	fftw_destroy_plan(dft_forth);
	fftw_destroy_plan(dft_back);
	free(output_bitmap[0]);
	free(output_bitmap);
	free(difference_bitmap[0]);
	free(difference_bitmap);
	free(output_bitmap_control[0]);
	free(output_bitmap_control);
	free(dft_output_control[0]);
	free(dft_output_control);
	fftw_destroy_plan(dft_forth_control);
	free(phasedif[0]);
	free(phasedif);
	
	return 0;
}
