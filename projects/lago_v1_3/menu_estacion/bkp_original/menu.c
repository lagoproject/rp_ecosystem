#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/types.h>
#include "tinygps.h"
#include "bmp180.h"
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/vfs.h>
#include <poll.h>
#include <pthread.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#define FSPATH "/media/usb" // Dirección donde se monta el ssd

#define BAUDRATE B9600	// Configuración UART para GPS - Velocidad
#define FALSE 0
#define TRUE 1
#define VERSION "0.1"

volatile int STOP=FALSE;

int wait_flag=TRUE;
char devicename[80] = "/dev/ttyPS1";   // Nombre del dispositivo GPS para UART
int status;

// Globales
char str_aux[256], str_aux_sensor[256],*token;	// String que usaremos para mostrar mensajes y almacenar lo que escribiremos/leeremos por pipe.

float h = 0;  // Variable donde se guardara la humedad
float t = 0;  // Variable donde se guardara la temperatura
float p = 0;  // Variable donde se guardara la presión


int i, j, k, x, y; // Variable utilizada para los lazos FOR

 /*bmp180*/
int BMPAddr = 0x77; // dirección del Bmp180

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
/*Párametros WCT*/
int n, AV, AVn, trigger1, trigger2, samples, samples_count, nfiles, tyvek, flag;
char file_name[25], new_file[25], new_copy[25];
char c_tyvek[2], c_AV[5], c_trigger1[4], c_trigger2[4], c_samples[6], c_files[10];
/* ssd*/
int ssd_flag = FALSE;

/*fds_flag*/
int fds_flag=FALSE;

/*Defaul-Mode*/
int init_time = 10;
int d_flag = FALSE;

/*Archivo Test Save Sensors in SSD*/

FILE *fsensors=NULL;
int fsensors_flag = FALSE;

int primera_flag = 0;

/*BUS 		I2C */
int file;
char *bus = "/dev/i2c-0";

// Archivo default
FILE *fdefault = NULL;
FILE *fcount = NULL;
int fdefault_flag = FALSE, fcount_flag = FALSE;
int num;

/*Archivo datos*/
//FILE *fdata = NULL;
//FILE *frate = NULL;
int mfd;
int position_0, position_1;
uint32_t *slcr, *axi_hp0, *axi_hp1;
uint32_t limit = 0;
uint32_t b_state_actual = 0, b_state_last = 0;
uint32_t read1 = 0;
uint32_t state;

int16_t ch[2];
int trig_count = 0;
float rp_temp, rp_vp_vn, rp_aux0, rp_aux1, rp_aux8, rp_aux9;  // Variables donde se guardaran los datos del ADCs lentos
void *cfg, *sts_0, *sts_1, *b_state, *buf_V_MON, *buf_CDE_IN, *buf_temp_RP, *buffer_0, *buffer_1, *buf_cpu;
float actual_V_MON, actual_CDE_IN, actual_temp_RP;  //Variables sts de ADC auxiliar
char *name = "/dev/mem";

/***********************/
int fd_data, fd_rate;
/**********************/

int move_count = 0;
int count_copy = 0;

// Cabeceras funciones
void signal_handler_IO (int status);

void *Thread_move(void *name_mov)
{
    int read_fd, read_fd_rate;
    int write_fd, write_fd_rate;
    int m = 0;  //intentos mount
    struct stat stat_buf;
    struct stat stat_buf_rate;
    off_t offset = 0;
    off_t offset_rate = 0;
    
    char dir2[125];
    char dir1_rate[125];
    char dir2_rate[125];
    char *dir1;

    dir1 = (char*) name_mov;

	for (m = 0; m < 3; m++)
	{

	    if (mount("/dev/sda1", "/media/usb", "ext4", MS_NOATIME, ""))
	    {

	        if (errno == EBUSY)
	        {
	            printf("Mount ERROR: Mountpoint busy\n");
	        }
	        else
	        {
	            printf("Mount ERROR: %s\n", strerror(errno));
	        }
	        sleep (1);
	    }
	    else
	    {
	    	printf("Montaje Exitoso. Intento: %d\n", m);
	        printf("Inicia copia del archivo:%s\n", dir1);
	        sprintf(dir2,"/media/usb/%s",dir1);

	        /* Open the input file. */
	        read_fd = open (dir1, O_RDONLY);
	        /* Stat the input file to obtain its size. */
	        fstat (read_fd, &stat_buf);
	        /* Open the output file for writing, with the same permissions as the
	         source file. */
	        write_fd = open (dir2, O_WRONLY | O_CREAT, stat_buf.st_mode);
	        /* Blast the bytes from one file to the other. */
	        sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
	        /* Close up. */
	        close (read_fd);

	        fsync(write_fd);
	        sleep(3);

	        close (write_fd);


	        printf("Archivo: %s copiado\n", dir1);

	        // Copia del archivo de Rate

	        sprintf(dir1_rate,"rate_%s",dir1);
	        sprintf(dir2_rate,"/media/usb/%s",dir1_rate);
	        printf("Inicia copia del archivo:%s\n", dir1_rate);

	        /* Open the input file. */
	        read_fd_rate = open (dir1_rate, O_RDONLY);
	        /* Stat the input file to obtain its size. */
	        fstat (read_fd_rate, &stat_buf_rate);
	        /* Open the output file for writing, with the same permissions as the
	         source file. */
	        write_fd_rate = open (dir2_rate, O_WRONLY | O_CREAT, stat_buf_rate.st_mode);
	        /* Blast the bytes from one file to the other. */
	        sendfile (write_fd_rate, read_fd_rate, &offset_rate, stat_buf_rate.st_size);
	        /* Close up. */
	        close (read_fd_rate);

	        fsync(write_fd_rate);
	        usleep(500);
	        close (write_fd_rate);

	        sleep(1);

	        m = 4;
	        
	        printf("Archivo: %s copiado\n", dir1_rate);
	        remove(dir1);
	        
	        umount("/media/usb");
			//printf("Desmontado\n");
	    }

	    sleep(1);
	}
    return NULL;
}

int main(void)
{
	pthread_t thread_id;

 	//Función para esperar el tiempo
 	struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };

	// Variables comunes padre e hijo
	int pidPadre=getpid();	// Obtenemos el Pid del Padre
	int pidHijo=0;			// Variable para guardar la salida del fork
	int bucle=1;			// Padre e hijo usaran un bucle, por lo tanto definimos el valor a "true"
	int nbytes=0;			// Se usara para saber cuantos bytes se han escrito/leido de la pipe.

	int pipePaH[2];			// Pipe de comunicacion Padre -> Hijo
	int pipeHaP[2];			// Pipe de comunicacion Hijo -> Padre

	// Variables padre
	char opcion[2];			// Usaremos para guardar la opcion del menu seleccionada.
	int opc;
	// Creamos las pipes, si fallan (-1) mostramos error. Y enviamos una señal SIGKILL (kill -9 pid) para finalizar el programa.
	if ( -1 == pipe(pipePaH) )
	{
		bzero(str_aux,256);
		sprintf(str_aux,"Error al crear la pipe Padre a Hijo\n");
		write(1,str_aux,strlen(str_aux));
		raise(SIGKILL);
	}
	if ( -1 == pipe(pipeHaP) )
	{
		bzero(str_aux,256);
		sprintf(str_aux,"Error al crear la pipe Hijo a Padre\n");
		write(1,str_aux,strlen(str_aux));
		raise(SIGKILL);
	}

	// Presentacion
	bzero(str_aux,256);
	sprintf(str_aux,"\n\n\t\tESCUELA POLITÉCNICA NACIONAL\n\n\tFACULTAD DE INGENIERIA ELÉCTRICA Y ELECTRÓNICA\n\n\t\t     FACULTAD DE CIENCIAS\n\n\tLaboratorio de Astrofísica y Astropartículas\n");
	write(1,str_aux,strlen(str_aux));

	bzero(str_aux,256);
	sprintf(str_aux,"\n Software diseñado para la adquisición de datos de la estación en la cual\nse realiza el control automático de alto voltaje en el PMT, establecimiento\nde parámetros como: TYVEK, Nivel de trigger, Tiempo de adquisición por archivo \ny Número de archivos a generar. \nAdemás de la adquisición automática de variables ambientales como temperatura,\npresión atmosférica, humedad y posición geográfica de la estación.\n** Inicio automático con valores por default **\n");
	write(1,str_aux,strlen(str_aux));

    if (mount("/dev/sda1", "/media/usb", "ext4", MS_NOATIME, ""))
    {

        if (errno == EBUSY)
        {
            printf("Mount ERROR: Mountpoint busy\n");
        }
        else
        {
            printf("Mount ERROR: %s\n", strerror(errno));
        }
    }
    else
    {
		//Lee tamaño disponible en el Disk-Usb
		struct statfs info;
		statfs (FSPATH, &info);
		long int sizeKB = info.f_bavail * (info.f_bsize/1000); // Calcular tamaño en KBytes
		float sizeMB = sizeKB/1000;
		float sizeGB = sizeMB/1000;
		int nfilesmax =  sizeGB/(samples*0.000000012);

		bzero(str_aux,256);
		sprintf(str_aux,"\nMONTAJE DE DISCO SSD EXITOSO!!\nEspacio disponible:%.2fGB\n", sizeGB);
		write(1,str_aux,strlen(str_aux));
	}

    umount("/media/usb");
	//printf("Desmontado \n");


	// Creamos el proceso duplicado (hijo) y guardamos el retorno en la variable.
	pidHijo=fork();
	switch (pidHijo)
	{
		case -1:
			// Imposible crear el fork -> Matamos el programa
			bzero(str_aux,256);
			sprintf(str_aux,"Imposible crear un fork! \n");
			write(1,str_aux,strlen(str_aux));
			raise(SIGKILL);
		break;

		case 0:
			// Fork ha ido ok
			// SOY EL PROCESO HIJO

			// Cerramos el canal de escritura de Padre -> Hijo
			close(pipePaH[1]);

			// Cerramos el canal de lectura de Hijo -> Padre
			close(pipeHaP[0]);

			while (bucle)
			{
				bzero(str_aux,256);
				nbytes = read(pipePaH[0],str_aux,256);

				if ( strncmp("RS", str_aux, 2) == 0)
				{
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

					while (count != 2 && aux < 10 )//Si no se recibe una señal valida del GPS en este lapso el programa continua
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
					}

					close(fd);


					if((file = open(bus, O_RDWR)) < 0)
					{
						printf("Failed to open the bus. \n");
						exit(1);
					}

					// Sensor de Humedad SI7021 I2C con su dirección 0x40
					ioctl(file, I2C_SLAVE, 0x40);

					// Envia comando para la lectura de humedad(0xF5)
					char config[1] = {0xF5};
					write(file, config, 1);
					sleep(1);

					// Se lee los 2 bytes de h data
					// h msb, h lsb
					char data[2] = {0};
					if(read(file, data, 2) != 2)
					{
						h = 0; // Si la lectura no es válida colaca cero.
					}
					else
					{
						// Convierte el data en humedad. Ver datasheet
						h = (((data[0] * 256 + data[1]) * 125.0) / 65536.0) - 6;
						h = (h*1.013) -7.601;
					}

					// Inicializacion para el sensor BMP180
					void *bmp = bmp180_init(BMPAddr, bus);
					bmp180_eprom_t eprom;
					bmp180_dump_eprom(bmp, &eprom);
					bmp180_set_oss(bmp, 1);

					if(bmp != NULL)
					{
						t = bmp180_temperature(bmp);
						t = (t*0.9896)-0.0375;

				    	p = bmp180_pressure(bmp);
				    	p = (p*0.9982)-0.7651 ;
					}

					bzero(str_aux,256);

					if (flat > 2)
					{
						flat = 0;
					}

					if (p == 99977) // Valor que entrega el BMP cuando pierde comincacion, por lo que solo se envia el resto de sensores
					{
						sprintf(str_aux,"# ------------------------------\n# Fecha:%d/%d/%d \n# Hora:%d:%d:%d:%d \n# Latitud= %f \n# Longitud= %f \n# Altitud: %lum\n# Satélites: %lu\n# Humedad: I/O ERROR\n# Temperatura: I/O ERROR\n# Presión: I/O ERROR\n# ------------------------------\n",fyear, fmes, fdia, fhora-5, fminutos, fs, fhs, flat, flon, alt, satellites);
	                }
					else if (h == 0) // Valor que entrega el SI7021 cuando pierde comincacion, por lo que solo se envia el resto de sensores
					{
						sprintf(str_aux,"# ------------------------------\n# Fecha:%d/%d/%d \n# Hora:%d:%d:%d:%d \n# Latitud= %f \n# Longitud= %f \n# Altitud: %lum\n# Satélites: %lu\n# Humedad: I/O ERROR\n# Temperatura: %.2fC\n# Presión: %.2fhPa\n# ------------------------------\n",fyear, fmes, fdia, fhora-5, fminutos, fs, fhs, flat, flon, alt, satellites, t, p/100);
					}
					else
					{
						sprintf(str_aux,"# ------------------------------\n# Fecha:%d/%d/%d \n# Hora:%d:%d:%d:%d \n# Latitud= %f \n# Longitud= %f \n# Altitud: %lum\n# Satélites: %lu\n# Humedad: %.2fRH\n# Temperatura: %.2fC\n# Presión: %.2fhPa\n# ------------------------------\n",fyear, fmes, fdia, fhora-5, fminutos, fs, fhs, flat, flon, alt, satellites, h, t, p/100);
					}

					//Se envia los datos de los sensores por pipe al proceso Padre.

					write(pipeHaP[1],str_aux,strlen(str_aux));

					bmp180_close(bmp);

					close(file);
				}
				else if ( 0 == strncmp("FN",str_aux,2) )
				{
				bucle=0;
				}

			}
		break;

		default:
			// Fork ha ido ok.
			// SOY EL PROCESO PADRE

			// Cerramos los canales de las pipes que el padre no vaya a usar.
			close(pipeHaP[1]);
			close(pipePaH[0]);
			/*
			// Mostramos un mensaje con el PID del padre y el PID del hijo ().
			bzero(str_aux,256);
			//sprintf(str_aux,"[START] Padre, con el pid %d, mi hijo con el pid %d \n",pidPadre,pidHijo);
			write(1,str_aux,strlen(str_aux));
			*/
			while (bucle)
			{
				//Leemos y mostramos los parámetros por Default
				if((fdefault = fopen("default.txt", "r")) == NULL)
				{
					fds_flag = FALSE;
				}
				else
				{
					fds_flag = TRUE;
					bzero(str_aux,256);
			        /* Busca el inicio del archivo */
			        fseek(fdefault, 0, SEEK_SET);
			        /* Lee e imprime los valores por default guardados anteriormente en el archivo */
			        fread(str_aux, 29, 1, fdefault); // Numero de caracteres guardados aproximadamente 29
			        const char s[2] = "\n";
			        token = strtok(str_aux, s); //Busca en el estring y separa los strings que esten separados por un ENTER ("\n")
                    //Separa y asigna cada string a su respectiva variable
			        while( token != NULL )
			        {
			            j++;
			            switch(j)
			            {
			                case 1: // Si posee TYVECK
		                        sprintf(c_tyvek,token);
		                        if(c_tyvek[0] == 's')
		                        {
		                          tyvek = 1;
		                        }
		                        else
		                        {
		                          tyvek = 0;
		                        }

			                case 2: // El alto Voltaje
		                        sprintf(c_AV,token);
		                        AV = atoi(c_AV);

			                case 3: // El trigger
		                        sprintf(c_trigger1,token);
		                        trigger1 = atoi(c_trigger1);
		                        

         					case 4: // El trigger
		                        sprintf(c_trigger2,token);
		                        trigger2 = atoi(c_trigger2);

			                case 5: // Nombre del archivo
		                        sprintf(file_name,token);

			                case 6: // Número de samples
		                        sprintf(c_samples,token);
		                        samples = atoi(c_samples);

		                    case 7: // Número de archivos
		                        sprintf(c_files,token);
		                        nfiles = atoi(c_files);
			            }
			            token = strtok(NULL, s);
			        }
			        j = 0;
			        // se Muestra los parámetros leidos
		            bzero(str_aux,256);
		            sprintf(str_aux,"\nPARÁMETROS POR DEFAULT:\na) TYVEK (s/n):%s\nb) Alto voltaje en el PMT:%dV\nc) Nivel de trigger1:%dmV\nNivel de trigger2:%dmV\nd) Nombre del archivo:%s.dat\ne) Tiempo de adquisición por archivo:%d\nf) Número de archivos:%d\n", c_tyvek, AV, trigger1, trigger2, file_name, samples, nfiles);
		            write(1,str_aux,strlen(str_aux));
		            fclose(fdefault);
    			}

				// Mostramos el menu de opciones del padre.
				bzero(str_aux,256);
				sprintf(str_aux,"\n\n\t *** MENU ***\n\n1. Mostrar Fecha, Hora, Latitud, Longitud, Humedad, Temp Ambiente\n      Presión y Altitud. \n2. Definir parámetros del WCD. (Alto Voltaje, Pulsos, Nombre del Archivo).\n3. Inicio de Toma de Datos\n4. Salir y Finalizar Programa\n\n\tInicio automático en 10 segundos.\n\tIngrese Una Opción : ");
				write(1,str_aux,strlen(str_aux));

				if(poll(&mypoll, 1, init_time*1000))
				{
					// Leemos la opcion por pantalla
					//read(0,opcion,2);
					read(0,opcion,2);
					// Quitamos el \n que se ha leido al hacer enter
					opcion[1]='\0'; // Quitamos el \n que se ha leido al hacer enter, de ahi que sea un char de [2], para evitar ese \n
					opc = atoi(&opcion[0]);// Pasamos el caracter numerico leido a integer.
				}
				else
				{
					opc = 1;
					bzero(str_aux,256);
					sprintf(str_aux,"\n\t\tInicio Automático!!\n ");
					write(1,str_aux,strlen(str_aux));
				}

				switch (opc)
				{
					case 1: // Se pregunta al hijo el valor de los sensores
						// Enviamos la señal RS (READ SENSORS) al hijo
						write(pipePaH[1],"RS",2);

						// Limpiamos el buffer
						bzero(str_aux,256);

						// Esperamos respuesta del hijo
						nbytes = read(pipeHaP[0],str_aux,256);

						// Escribimos por pantalla lo que nos ha comunicado el hijo.
						write(1,str_aux,nbytes);
					break;

					case 2: // El usuario asigna nuevos parámetros para el WCD y estos se los guarda en el archivo dafault.
						do
						{
							// Preguntamos.
							bzero(str_aux,256);
							sprintf(str_aux,"\na) El tanque tiene recubrimiento interno de TYVEK (s/n): \n ");
							write(1,str_aux,strlen(str_aux));

							// Leemos
							nbytes = read(0,c_tyvek,2);
							c_tyvek[1]='\0';

							if(c_tyvek[0] == 's')
							{
								tyvek = 1;
								flag = 0;
							}
							else if(c_tyvek[0] == 'n')
							{
								tyvek = 0;
								flag = 0;
							}
							else
							{
								// Mostramos error y volvemos al menu.
								bzero(str_aux,256);
								sprintf(str_aux,"\nIngreso incorrecto. Vuelva a ingresar. \n ");
								write(1,str_aux,strlen(str_aux));
								flag = 1;
							}
						}
						while (flag);

						do
						{
							bzero(str_aux,256);
							sprintf(str_aux,"\nb) Ingrese el nivel de alto voltaje deseado en el PMT( Ejemplo:[0 - 2000] ): \n");
							write(1,str_aux,strlen(str_aux));

							// Leemos
							nbytes = read(0,c_AV,5);
							c_AV[5]='\0';
							AVn = atoi(c_AV);

							printf("AV ingresado:%d\n",AVn);

							if (AVn > 0  &&  AVn < 2001)
							{
								flag = 0;
							}
							else
							{
								bzero(str_aux,256);
								sprintf(str_aux,"\nIngreso incorrecto. Vuelva a ingresar. \n ");
								write(1,str_aux,strlen(str_aux));
								flag = 1;
							}


						}
						while (flag);

						do
						{
							bzero(str_aux,256);
							sprintf(str_aux,"\nc) Ingrese el nivel de trigger en mV para la entrada 1 (Ejemplo: [30 - 950]mV)): \n");
							write(1,str_aux,strlen(str_aux));

							// Leemos
							nbytes = read(0,c_trigger1,4);
							c_trigger1[4]='\0';
							trigger1 = atoi(c_trigger1);
							printf("Nivel de trigger 1 ingresado:%d\n",trigger1);

							if (trigger1 > 29  &&  trigger1 < 951)
							{
								flag = 0;
							}
							else
							{
								bzero(str_aux,256);
								sprintf(str_aux,"\nIngreso incorrecto. Vuelva a ingresar. \n ");
								write(1,str_aux,strlen(str_aux));
								flag = 1;
							}
						}
						while (flag);

						do
						{
							bzero(str_aux,256);
							sprintf(str_aux,"\nd) Ingrese el nivel de trigger en mV para la entrada 2 (Ejemplo: [30 - 3000]mV)): \n");
							write(1,str_aux,strlen(str_aux));

							// Leemos
							nbytes = read(0,c_trigger2,4);
							c_trigger2[4]='\0';
							trigger2 = atoi(c_trigger2);
							printf("Nivel de trigger 2 ingresado:%d\n",trigger2);

							if (trigger2 > 29  &&  trigger2 < 3001)
							{
								flag = 0;
							}
							else
							{
								bzero(str_aux,256);
								sprintf(str_aux,"\nIngreso incorrecto. Vuelva a ingresar. \n ");
								write(1,str_aux,strlen(str_aux));
								flag = 1;
							}
						}
						while (flag);

						do
						{
							bzero(str_aux,256);
							sprintf(str_aux,"\ne) Ingrese el nombre del archivo sin números y sin la extensión (Max. 20 caracteres Ejemplo: datos):\n \n");
							write(1,str_aux,strlen(str_aux));

							// Leemos
							bzero(str_aux,256);
							nbytes = read(0,str_aux,25);
							str_aux[strcspn(str_aux, "\n")] = 0;
							if ((strcspn(str_aux, "\n")) > 0 && (strcspn(str_aux, "\n")) < 21)
							{
								sprintf(file_name,str_aux);
								bzero(str_aux,256);
								sprintf(str_aux,"\nnombre del archivo: %s.dat \n",file_name);
								write(1,str_aux,strlen(str_aux));
								flag = 0;
							}
						    else
						    {
								bzero(str_aux,256);
								sprintf(str_aux,"\nIngreso incorrecto. Vuelva a ingresar. \n ");
								write(1,str_aux,strlen(str_aux));
								flag = 1;
							}
						}
						while(flag);

						do
						{
							bzero(str_aux,256);
							sprintf(str_aux,"\nf)Ingrese el tiempo de adquisición por cada archivo a guardarse en minutos (Ejemplo: [1 - 30]) :\n \n");
							write(1,str_aux,strlen(str_aux));

							// Leemos
							nbytes = read(0,c_samples,6);
							c_samples[6]='\0';
							samples = atoi(c_samples);
							//printf("samples ingresado:%d\n",samples);

							if (samples > 0  &&  samples < 31)
							{
								flag = 0;
								//samples = tiempo
								printf("Tiempo ingresado:%d minutos\n",samples);
							}
							else
							{
								bzero(str_aux,256);
								sprintf(str_aux,"\nIngreso incorrecto. Vuelva a ingresar. \n ");
								write(1,str_aux,strlen(str_aux));
								flag = 1;
							}
						}
						while (flag);

						do
						{
						    if (mount("/dev/sda1", "/media/usb", "ext4", MS_NOATIME, ""))
						    {

						        if (errno == EBUSY)
						        {
						            printf("Mount ERROR: Mountpoint busy\n");
						        }
						        else
						        {
						            printf("Mount ERROR: %s\n", strerror(errno));
						        }
						    }
						    else
						    {
								//Lee tamaño disponible en el Disk-Usb
								struct statfs info;
	        					statfs (FSPATH, &info);
	        					long int sizeKB = info.f_bavail * (info.f_bsize/1000); // Calcular tamaño en KBytes
	        					float sizeMB = sizeKB/1000;
	        					float sizeGB = sizeMB/1000;
	        					int nfilesmax =  sizeMB/(samples*20);

								bzero(str_aux,256);
								sprintf(str_aux,"\nEspacio disponible:%.2fGB\ng) Ingrese el número de archivos a generarse(Ejemplo: [0 - %d]): \n", sizeGB, nfilesmax);
								write(1,str_aux,strlen(str_aux));

								// Leemos
								nbytes = read(0,c_files,6);

								c_files[6]='\0';
								nfiles = atoi(c_files);
								printf("Número de archivos deseados:%d\n",nfiles);



								if ((nfiles > 0)  &&  (nfiles < (nfilesmax + 1)))
								{
									flag = 0;
								}
								else
								{
									bzero(str_aux,256);
									sprintf(str_aux,"\nIngreso incorrecto. Vuelva a ingresar. \n ");
									write(1,str_aux,strlen(str_aux));
									flag = 1;
								}
							}

							umount("/media/usb");
							
						}

						while (flag);

						// Abre/Crea el archivo deafault para guardar los parámetros ingresados.

						if((fdefault = fopen("default.txt", "w")) == NULL)
						{
							fds_flag = FALSE;
							break;
						}
						else
						{
							printf("AV ingresado:%d\n",AVn);
							fprintf(fdefault,"%s\n%d\n%d\n%d\n%s\n%d\n%d\n",c_tyvek, AVn, trigger1, trigger2, file_name, samples, nfiles);
							fclose(fdefault);
							printf("AV ingresado:%d\n",AVn);

							bzero(str_aux,256);
							sprintf(str_aux,"\n\t\tIngreso Exitoso!! Eliga la opción 3 para iniciar la adquisición\n ");
							write(1,str_aux,strlen(str_aux));
						}
						//Manda a cero el archivo contador, este archivo se crea para guardar el numero de veces que se manda a correr la toma de datos
						// Se reseta cuando se ingresa nuevos parámetros
						if((fcount= fopen("contador.txt", "w")) == NULL)
						{
							fds_flag = FALSE;
							break;
						}
						else
						{

							fprintf(fcount,"%d",0);
							fclose(fcount);
						}
					break;

					case 3: // Ejecuta el programa principal de toma de datos

						// Lee el conteo guardado en el archivo contador.
						if((fcount = fopen("contador.txt", "r+")) == NULL)
						{
							break;
						}

						//Si la apertura del archivo es exitoso, se lee el contador y
						//se asigna un número al nombre del archivo para evitar que los archivos se sobreescriban
						else
						{
							/*Busca el inicio del archivo */
     				        fseek(fcount, 0, SEEK_SET);
					        /* Lee el dato */
					        fread(str_aux, 10, 1, fcount);
					        //Convierte en numero
					        num = atoi(str_aux);

					        bzero(str_aux,256);

					        file_name[strcspn(file_name, "1234567890")] = 0; // Toma el string que no contenga numeros

					        sprintf(file_name,"%s%d",file_name,num); //Asigna el numero leido al nuevo nombre del archivo
					        //write(1,file_name,strlen(file_name));

					        fseek(fcount, 0, SEEK_SET);
							fprintf(fcount,"%d", num+1);
							fclose(fcount);
						}

						if((mfd = open(name, O_RDWR)) < 0)
						{
						    perror("open");
						    return 1;
						}

slcr = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0xF8000000);

axi_hp0 = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0xF8008000);
axi_hp1 = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0xF8009000);

cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0x40000000);
sts_0 = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0x40001000);
sts_1 = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0x40002000);
b_state = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0x40003000);
buf_V_MON = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0x40004000);
buf_CDE_IN = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0x40005000);
buf_temp_RP = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0x40006000);

buffer_0 = mmap(NULL, 4096*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0x1E000000);
buffer_1 = mmap(NULL, 4096*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0x1F000000);
buf_cpu = mmap(NULL, 4096*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

memset(buffer_0, 0, 4096*sysconf(_SC_PAGESIZE));
memset(buffer_1, 0, 4096*sysconf(_SC_PAGESIZE));

// set FPGA
slcr[2] = 0xDF0D;    //SLCR_UNLOCK and 0xDF0D is the unlock key
slcr[144] = 0;       //FPGA_RST_CTRL
axi_hp0[0] &= ~1;    //AFI_RDCHAN_CTRL 64 bits enabled
axi_hp0[5] &= ~1;    //AFI_WRCHAN_CTRL 64 bits enabled
axi_hp1[0] &= ~1;    //AFI_RDCHAN_CTRL 64 bits enabled
axi_hp1[5] &= ~1;    //AFI_WRCHAN_CTRL 64 bits enabled

// reset rampaDAC
*((uint16_t *)(cfg + 0)) &= ~16;      //check this reset and sleep
*((uint16_t *)(cfg + 0)) |= 16;

// set CDE_IN CONTROL in 0
*((uint16_t *)(cfg + 0)) &= ~256;

// set PMT_DAC level in 0
*((uint16_t *)(cfg + 6)) = 0;

// reset ADCslow
*((uint16_t *)(cfg + 0)) &= ~32;
*((uint16_t *)(cfg + 0)) |= 32;

// wait 100 usecond
usleep(100);   //CAMBIAR A 100 ------------------------------------------------------------------------------------------

// set CDE_IN CONTROL
*((uint16_t *)(cfg + 0)) |= 256;

  // set PMT_DAC level
AV = (AV*1.5954) + 44.735;
*((uint16_t *)(cfg + 6)) = AV;

// wait 30 second for AV PMT
sleep(30);    //CAMBIAR A 30 .--------------------------------------------------------------------------------------------------

  // set trigger_lvl_a
trigger1 = trigger1*8.192;
*((uint16_t *)(cfg + 2)) = trigger1;

// set trigger_lvl_b
trigger2 = trigger2*8.192;
*((uint16_t *)(cfg + 4)) = trigger2;

// reset Writter
*((uint16_t *)(cfg + 0)) &= ~15;
*((uint16_t *)(cfg + 0)) |= 15;


b_state_actual = 0;
b_state_last = 4095;

k = 0;
move_count = 0;

for (y = 0; y < nfiles; y++)
{
	if ((y == 0) && (primera_flag == 0))
	{
	    bzero(new_file,25);
	    sprintf(new_file,"%s_%d.dat",file_name, y);
	    fd_data = open (new_file, O_RDWR | O_CREAT | O_APPEND);
	    printf("\nCrea Archivo: %s\n", new_file);
	    bzero(str_aux,256);
		sprintf(str_aux,"rate_%s_%d.dat",file_name, y);
		fd_rate = open (str_aux, O_RDWR | O_CREAT | O_APPEND);

	// Enviamos la señal RS (READ SENSORS) al hijo
		write(pipePaH[1],"RS",2);

		// Limpiamos el buffer
		bzero(str_aux,256);

		// Esperamos respuesta del hijo
		nbytes = read(pipeHaP[0],str_aux,256);

								            
        write (fd_data, (void *) str_aux, strlen(str_aux));
		//write(fd_rate,str_aux,strlen(str_aux));
        write (fd_rate, (void *) str_aux, strlen(str_aux));
		

		close (fd_data);
		close (fd_rate);

		primera_flag = 1;
    }


	k = 0;

	while (k != (samples*6))
	{
		position_0 = *((uint32_t *)(sts_0 + 0));
		//printf("# position_0  0 %d\n",position_0 );

		position_1 = *((uint32_t *)(sts_1 + 0));
		//printf("# position_1  0 %d\n",position_1 );

		b_state_actual = *((uint32_t *)(b_state + 0));
		//printf("# b_state_actual %d\n", b_state_actual);

		actual_V_MON = *((uint32_t *)(buf_V_MON + 0));
		//printf("# actual_V_MON = %.2fV\n", actual_V_MON*0.000107);

		actual_CDE_IN = *((uint32_t *)(buf_CDE_IN + 0));
		//printf("# actual_CDE_IN = %.2fV\n", actual_CDE_IN*0.000107);

		actual_temp_RP = *((uint32_t *)(buf_temp_RP + 0));
		//printf("# actual_temp_RP = %.2fV\n", (actual_temp_RP*0.00769 - 273.15));


		actual_V_MON = 419.02*actual_V_MON - 11.15; //cálculo de alto voltaje monitoreado
		actual_CDE_IN = 719.12*actual_CDE_IN + 16.85; ////cálculo de alto voltaje establecido para el PMT

		// if ((actual_V_MON > (actual_CDE_IN + 50)) 3|| (actual_V_MON > (actual_CDE_IN - 50)))
		// {
		//    sleep (300);
		//    y = nfiles;   //se envia a terminar la adquisición y el apagado del PMT
		// }fprint


		// if (actual_temp_RP > 100)
		// {
		// 	sleep (300);  //espera 5 min para que Red Pitaya baje su temperatura.
		// 	break;
		// }

		if (b_state_actual != b_state_last)
		{
		    printf("#------------");
		    switch (b_state_actual)
		    {
			    case 255:
				    read1 = 11111; //leer buffer_0
				    limit = *((uint32_t *)(sts_0 + 0));
				    memcpy (buf_cpu, buffer_0, limit*8);
				    printf("# en buffer_0 un rate promedio de %.0f (en 10 seg); %d segundos de datos para el archivo %d de un total %d segundos\n", ((((limit * 2) - 80) / 220) / 0.98), ((k * 10) + 10), y, (samples * 60));
			    break;

			    case 4095:
				    read1 = 10101; //leer buffer_1
				    limit = *((uint32_t *)(sts_1 + 0));
				    memcpy (buf_cpu, buffer_1, limit*8);
				    printf("# en buffer_1 un rate promedio de %.0f (en 10 seg); %d segundos de datos para el archivo %d de un total %d segundos\n", ((((limit * 2) - 80) / 220) / 0.98), ((k * 10) + 10), y, (samples * 60));
			    break;

			    default:
			        read1 = 0;
			    break;
		    }

		        bzero(new_file,25);
			    sprintf(new_file,"%s_%d.dat",file_name, y);
			    fd_data = open (new_file, O_RDWR | O_CREAT | O_APPEND);
			    
			    bzero(str_aux,256);
				sprintf(str_aux,"rate_%s_%d.dat",file_name, y);
				fd_rate = open (str_aux, O_RDWR | O_CREAT | O_APPEND);


      	    if (x == 6)
			{
				// Enviamos la señal RS (READ SENSORS) al hijo
				write(pipePaH[1],"RS",2);

				// Limpiamos el buffer
				bzero(str_aux_sensor,256);

				// Esperamos respuesta del hijo
				nbytes = read(pipeHaP[0],str_aux_sensor,256);

				write (fd_data, (void *) str_aux_sensor, strlen(str_aux_sensor));
				write (fd_rate, (void *) str_aux_sensor, strlen(str_aux_sensor));

				x = 0;
			}

	        b_state_last = b_state_actual;

	        for(i = 0; i < (limit*2); ++i)
	        {
	            ch[0] = *((int16_t *)(buf_cpu + 4*i + 0));
	            ch[1] = *((int16_t *)(buf_cpu + 4*i + 2));
	            state = *((uint32_t *)(buf_cpu + 4*i));

			    switch(state>>30)
			    {
			        case 0:  //adc_dat_a & adc_dat_b
			            //fprintf(fdata,"%5hd %5hd\n", (((ch[0]>>13)<<14) + ((ch[0]>>13)<<15) + ch[0]), (((ch[1]>>13)<<14) + ((ch[1]>>13)<<15) + ch[1]));
			        	bzero(str_aux,256);
			        	sprintf(str_aux,"%5hd %5hd\n", (((ch[0]>>13)<<14) + ((ch[0]>>13)<<15) + ch[0]), (((ch[1]>>13)<<14) + ((ch[1]>>13)<<15) + ch[1]));
			        	write (fd_data, (void *) str_aux, strlen(str_aux));
			        break;

			        case 1: //tr1_s & tr2_s & clk_cnt_pps_i
			           // fprintf(fdata,"# t %d %d %d\n", (state>>28)&0x1, (state>>29)&0x1, state&0xFFFFFFF);
			        	bzero(str_aux,256);
			            sprintf(str_aux,"# t %d %d %d\n", (state>>28)&0x1, (state>>29)&0x1, state&0xFFFFFFF);
			            write (fd_data, (void *) str_aux, strlen(str_aux));
			        break;

			        case 3:
			            switch (state>>28)
			            {
			            	case 15:  //clk_cnt_pps_i
			            		bzero(str_aux,256);
					            sprintf(str_aux,"# ------------------------------\n");
					            write (fd_data, (void *) str_aux, strlen(str_aux));
					            bzero(str_aux,256);
					            sprintf(str_aux,"# clk %d\n", state&0xFFFFFFF);
					            write (fd_data, (void *) str_aux, strlen(str_aux));
		            		break;

			          		case 14:
			              		switch (state>>27)
			              		{
			                        case 28: //trig_cnt_reg
						                trig_count = state&0x7FFFFFF;
						                trig_count = trig_count + 1;
						                bzero(str_aux,256);
						                sprintf(str_aux,"# count %d\n", trig_count);
						                write (fd_data, (void *) str_aux, strlen(str_aux));

						                //fprintf(fd2,"%d\n", state&0x7FFFFFF);
			                   		break;

			               			case 29:
			                  			switch (state>>16)
			                  			{
						                    case 59399: //rate
						                    bzero(str_aux,256);
						                    sprintf(str_aux,"# rate %d\n", (state)&0xFFFF);
						                    write (fd_data, (void *) str_aux, strlen(str_aux));
						                    bzero(str_aux,256);
						                    sprintf(str_aux,"%d\n", (state)&0xFFFF);
						                    write (fd_rate, (void *) str_aux, strlen(str_aux));
						                    break;

						                    case 59392: //rp_temp
						                    rp_temp = ((state)&0xFFFF);
						                    rp_temp = rp_temp * 0.00769 - 273.15;
						                    bzero(str_aux,256);
						                    sprintf(str_aux,"# rp_temp %.2fC\n", rp_temp);
						                    write (fd_data, (void *) str_aux, strlen(str_aux));
						                    break;

						                    case 59393: //rp_vp_vn
						                    rp_vp_vn = (state)&0xFFFF;
						                    rp_vp_vn = rp_vp_vn * 0.000186;
						                    bzero(str_aux,256);
						                    sprintf(str_aux,"# rp_vp_vn %.2fV\n", rp_vp_vn);
						                    write (fd_data, (void *) str_aux, strlen(str_aux));
						                    break;

						                    case 59394: //rp_aux0
						                    rp_aux0 = (state)&0xFFFF;
						                    rp_aux0 = rp_aux0 * 0.000107;
						                    bzero(str_aux,256);
						                    sprintf(str_aux,"# BA_temp %.2fV\n", rp_aux0);
						                    write (fd_data, (void *) str_aux, strlen(str_aux));
						                    break;

						                    case 59395: //rp_aux1
						                    rp_aux1 = (state)&0xFFFF;
						                    rp_aux1 = rp_aux1 * 0.000107;
						                    bzero(str_aux,256);
						                    sprintf(str_aux,"# V_MON %.2fV\n", rp_aux1);
						                    write (fd_data, (void *) str_aux, strlen(str_aux));
						                    break;

						                    case 59396: //rp_aux8
						                    rp_aux8 = (state)&0xFFFF;
						                    rp_aux8 = rp_aux8 * 0.000107;
						                    bzero(str_aux,256);
						                    sprintf(str_aux,"# I_MON %.2fV\n", rp_aux8);
						                    write (fd_data, (void *) str_aux, strlen(str_aux));
						                    break;

						                    case 59397: //rp_aux9
						                    rp_aux9 = (state)&0xFFFF;
						                    rp_aux9 = rp_aux9 * 0.000107;
						                    bzero(str_aux,256);
						                    sprintf(str_aux,"# CDE_IN %.2fV\n", rp_aux9);
						                    write (fd_data, (void *) str_aux, strlen(str_aux));
						                    bzero(str_aux,256);
						                    sprintf(str_aux,"# ------------------------------\n");
						                    write (fd_data, (void *) str_aux, strlen(str_aux));
						                    break;

						                    default:
						                    bzero(str_aux,256);
						                    sprintf(str_aux,"# not RP_ADCslow\n");
						                    write (fd_data, (void *) str_aux, strlen(str_aux));
						                    break;
						                }
						            break;

					                default:
					                	bzero(str_aux,256);
						                sprintf(str_aux,"# not rate&ADCslow\n");
						                write (fd_data, (void *) str_aux, strlen(str_aux));
					                break;
				                }
						    break;

				            default:
				            bzero(str_aux,256);
				            sprintf(str_aux,"# not rate,META\n");
				            write (fd_data, (void *) str_aux, strlen(str_aux));
				            break;
				        }
			        break;

			        default:
			        bzero(str_aux,256);
			        sprintf(str_aux,"# not DATA\n");
			        write (fd_data, (void *) str_aux, strlen(str_aux));
			        break;
    	        }
	    	}

            fsync(fd_data);
			fsync(fd_rate);

			usleep(100);

            close (fd_data);
            close (fd_rate);


    	    switch (read1)
		    {
			    case 11111:
			    	memset(buffer_0, 0, 4096*sysconf(_SC_PAGESIZE));
			    	// reset writer A
			    	*((uint16_t *)(cfg + 0)) &= ~4;
			        *((uint16_t *)(cfg + 0)) |= 4;
			    break;

			    case 10101:
			        memset(buffer_1, 0, 4096*sysconf(_SC_PAGESIZE));
			        // reset writer B
			        *((uint16_t *)(cfg + 0)) &= ~8;
			        *((uint16_t *)(cfg + 0)) |= 8;
			    break;

			    default:
			    break;
		    }

		    memset(buf_cpu, 0, 4096*sysconf(_SC_PAGESIZE));

			k = k + 1;
			x = x + 1;

        }
    }


    printf("Fin Archivo: %d\n", y);

    if( (y > 0) && (y != (nfiles - 1)))
    {
        bzero(new_copy,25);
        sprintf(new_copy,"%s_%d.dat",file_name, move_count);
        pthread_create(&thread_id, NULL, Thread_move, (void*)new_copy);
        usleep(100);
        move_count = move_count + 1;



    }
    else if (y == (nfiles - 1))
    {
        while (move_count != (y + 1))
        {

            bzero(new_copy,25);
            sprintf(new_copy,"%s_%d.dat",file_name, move_count);
            pthread_create(&thread_id, NULL, Thread_move, (void*)new_copy);
            sleep(1);
            pthread_join(thread_id, NULL);
            move_count++;
        }
    }
}

printf("FIN ADQUISICIÓN\n");

//PARO tlast_cfg
*((uint16_t *)(cfg + 0)) |= 64;
*((uint16_t *)(cfg + 0)) &= ~64;

// set PMT_DAC level   --- OFF PMT
*((uint16_t *)(cfg + 6)) = 0;

sleep (30); //CAMBIAR A 30 -------------------------------------------------------------------------------------------------

// set CDE_IN CONTROL in 0
*((uint16_t *)(cfg + 0)) &= ~256;

*((uint16_t *)(cfg + 0)) &= ~15;  //leds

munmap(cfg, sysconf(_SC_PAGESIZE));
munmap(sts_0, sysconf(_SC_PAGESIZE));
munmap(sts_1, sysconf(_SC_PAGESIZE));
munmap(b_state, sysconf(_SC_PAGESIZE));
munmap(buf_V_MON, sysconf(_SC_PAGESIZE));
munmap(buf_CDE_IN, sysconf(_SC_PAGESIZE));
munmap(buf_temp_RP, sysconf(_SC_PAGESIZE));

munmap(buf_cpu, sysconf(_SC_PAGESIZE));

munmap(buffer_0, 4096*sysconf(_SC_PAGESIZE));
munmap(buffer_1, 4096*sysconf(_SC_PAGESIZE));


					break;
					case 4:
						// Enviamos por la pipe el mensaje de finalizacion al hijo-
						write(pipePaH[1],"FN",2);
						// Esperamos a que el hijo muera (al morir envia un señal de finalizacion que capturamos con wait).
						wait(&pidHijo);

						printf("FIN ADQUISICIÓN\n");

					    /*fsync(fd_data);
					    fsync(fd_rate);

					    close(fd_data);
					    close(fd_rate);

						//PARO tlast_cfg
						*((uint16_t *)(cfg + 0)) |= 64;
						*((uint16_t *)(cfg + 0)) &= ~64;

						// set PMT_DAC level   --- OFF PMT
						*((uint16_t *)(cfg + 6)) = 0;

						sleep (1); //CAMBIAR A 30 -------------------------------------------------------------------------------------------------

						// set CDE_IN CONTROL in 0
						*((uint16_t *)(cfg + 0)) &= ~256;


						munmap(cfg, sysconf(_SC_PAGESIZE));
						munmap(sts_0, sysconf(_SC_PAGESIZE));
						munmap(sts_1, sysconf(_SC_PAGESIZE));
						munmap(b_state, sysconf(_SC_PAGESIZE));
						munmap(buf_V_MON, sysconf(_SC_PAGESIZE));
						munmap(buf_CDE_IN, sysconf(_SC_PAGESIZE));
						munmap(buf_temp_RP, sysconf(_SC_PAGESIZE));

						munmap(buf_cpu, sysconf(_SC_PAGESIZE));

						munmap(buffer_0, 4096*sysconf(_SC_PAGESIZE));
						munmap(buffer_1, 4096*sysconf(_SC_PAGESIZE)); */

					break;

					default:
						// Mostramos mensaje de error
						bzero(str_aux,256);
						sprintf(str_aux,"\nHas elegido una opción no valida. \n ");
						write(1,str_aux,strlen(str_aux));
					break;
				}
			}
		break;
	}
	return 0;
}

void signal_handler_IO (int status)
{
wait_flag = FALSE;
}
