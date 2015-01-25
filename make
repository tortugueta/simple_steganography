gcc -Wall bmputils.c -c
gcc -Wall -lm -lfftw3 phasemark.c bmputils.o -o phasemark
gcc -Wall -lm encode_message.c bmputils.o -o encode_message
