#include <stdio.h>
#include <stdlib.h>
#include "tinygps.h"

#define BAUDRATE B9600  // Configuración UART para GPS - Velocidad
#define FALSE 0
#define TRUE 1
#define VERSION "0.1"

/*GPS*/
long alt; // Variable donde se guardara la altitud
long satellites;
float flat, flon; // Variables donde se guardarán la latitud y longitud
unsigned long age, fage;
int fyear;
byte fmes, fdia, fhora, fminutos, fs, fhs; // variables para la hora y fecha
int newdata = 0;
int res, fd;
int count=0, aux;
char buf[255];
struct termios newtio;
struct sigaction saio;
int res, fd;
int wait_flag=TRUE;
char devicename[80] = "/dev/ttyPS1";   // Nombre del dispositivo GPS para UART
int i; // Variable utilizada para los lazos FOR

void signal_handler_IO (int status)
{
wait_flag = FALSE;
}


int main(void) {

	// Abir la comunicacion UART para el GPS

	fd = open(devicename, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (fd < 0)
	{
		perror(devicename);
		exit(1);
	}

	//Se installa el serial handler para el GPS
	saio.sa_handler = signal_handler_IO;
	sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
	saio.sa_flags = 0;
	saio.sa_restorer = NULL;
	sigaction(SIGIO,&saio,NULL);

	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL, FASYNC);
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VMIN]=1;
	newtio.c_cc[VTIME]=0;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	//
	count=0;
	aux = 0;

	while (count != 2 && aux < 10 )//Si no se recibe una señal valida del GPS en este lapso el programa contnua
	{
		usleep (10);

		aux++;
		res = read(fd,buf,255);
		if (res > 0)
		{
			for (i=0; i < res; i++)  //Lee todos los caracteres del string
			{
				if(gps_encode(buf[i]))
					newdata = 1;
			}

			if (newdata)
			{
				alt = gps_f_altitude();
				satellites = gps_satellites();
				gps_crack_datetime(&fyear, &fmes, &fdia, &fhora, &fminutos, &fs, &fhs, &fage);
				if (fhora < 5)
				{
					fhora = 24 + fhora;
					fdia = fdia -1;
				}

				gps_f_get_position(&flat, &flon, &age);
				newdata = 0;
				count++;
			}
		}
        printf("# ------------------------------\n# Fecha:%d/%d/%d \n# Hora:%d:%d:%d:%d \n# Latitud= %f \n# Longitud= %f \n# Altitud: %lum\n# Satélites: %lu\n# -------------------------------\n",fyear, fmes, fdia, fhora-5, fminutos, fs, fhs, flat, flon, alt, satellites);

	}

	close(fd);


}

