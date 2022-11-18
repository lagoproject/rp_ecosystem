/*
 * @brief This is a simple parser programm for NMEA messages
 *
 * @Author L. Horacio Arnaldi <lharnaldi@gmail.com>
 *
 * (c) LabDPR  http://labdpr.cab.cnea.gov.ar
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include "nmea_rp.h"

int rp_GetComma(char num, char* line){
	char *ptr = line;
	int count = 0;
	while ((count != num) && ((ptr = strstr(ptr, ",")) != NULL)) {
		ptr += 1;
		count++;
	}
	return ptr-line;
}

int rp_NmeaParseGprmc(char *nmea, gprmc_t *GPS){
	uint8_t ch, status, tmp;
	float lati_cent_tmp, lati_second_tmp;
	float long_cent_tmp, long_second_tmp;
	float speed_tmp=0;
	char *buf = nmea;
	ch = buf[5];
	status = buf[rp_GetComma(2, buf)];

	GPS->NS = buf[rp_GetComma(4, buf)];
	GPS->EW = buf[rp_GetComma(6, buf)];

	//GPS->latitude = Get_Double_Number(&buf[rp_GetComma(3, buf)]);
	//GPS->longitude = Get_Double_Number(&buf[rp_GetComma(5, buf)]);

	GPS->latitude_Degree = (int)GPS->latitude / 100; // Latitud separada
	lati_cent_tmp = (GPS->latitude - GPS->latitude_Degree * 100);
	GPS->latitude_Cent = (int)lati_cent_tmp;
	lati_second_tmp = (lati_cent_tmp - GPS->latitude_Cent) * 60;
	GPS->latitude_Second = (int)lati_second_tmp;

	GPS->longitude_Degree = (int)GPS->longitude / 100; // Longitud separada
	long_cent_tmp = (GPS->longitude - GPS->longitude_Degree * 100);
	GPS->longitude_Cent = (int)long_cent_tmp;
	long_second_tmp = (long_cent_tmp - GPS->longitude_Cent) * 60;
	GPS->longitude_Second = (int)long_second_tmp;

	//speed_tmp = Get_Float_Number(&buf[rp_GetComma(7, buf)]); // Velocidad (unidad: milla náutica / hora)
	GPS->speed = speed_tmp * 1.85; //1Millas náuticas =1.85Kilómetros
	//GPS->direction = Get_Float_Number(&buf[rp_GetComma(8, buf)]); // ángulo

	GPS->D.hour = (buf[7] - '0') * 10 + (buf[8] - '0'); // Tiempo
	GPS->D.minute = (buf[9] - '0') * 10 + (buf[10] - '0');
	GPS->D.second = (buf[11] - '0') * 10 + (buf[12] - '0');
	tmp = rp_GetComma(9, buf);
	GPS->D.day = (buf[tmp + 0] - '0') * 10 + (buf[tmp + 1] - '0'); // fecha
	GPS->D.month = (buf[tmp + 2] - '0') * 10 + (buf[tmp + 3] - '0');
	GPS->D.year = (buf[tmp + 4] - '0') * 10 + (buf[tmp + 5] - '0') + 2000;

	//UTC2BTC(&GPS->D);

	return 1;
	//}
	//}

	//return 0;
	}

void rp_NmeaParseGpgga(char *nmea, gpgga_t *loc){
	char *ptr = nmea;

	ptr = strchr(ptr, ',')+1; //skip time -> not valid now
	//loc->gtime = atof(ptr);

	ptr = strchr(ptr, ',')+1;
	loc->latitude = atof(ptr);

	ptr = strchr(ptr, ',')+1;
	switch (ptr[0]) {
		case 'N':
			loc->NS = 'N';
			break;
		case 'S':
			loc->NS = 'S';
			break;
		case ',':
			loc->NS = '\0';
			break;
	}

	ptr = strchr(ptr, ',')+1;
	loc->longitude = atof(ptr);

	ptr = strchr(ptr, ',')+1;
	switch (ptr[0]) {
		case 'W':
			loc->EW = 'W';
			break;
		case 'E':
			loc->EW = 'E';
			break;
		case ',':
			loc->EW = '\0';
			break;
	}

	ptr = strchr(ptr, ',')+1;
	loc->quality = (uint8_t)atoi(ptr);

	ptr = strchr(ptr, ',')+1;
	loc->satellites = (uint8_t)atoi(ptr);

	ptr = strchr(ptr, ',')+1;

	ptr = strchr(ptr, ',')+1;
	loc->height = atof(ptr);
}

/*
   void rp_NmeaParseGprmc(char *nmea, gprmc_t *loc){
   char *ptr = nmea;

//			GPS->D.hour = (buf[7] - '0') * 10 + (buf[8] - '0'); // Tiempo
//			GPS->D.minute = (buf[9] - '0') * 10 + (buf[10] - '0');
//			GPS->D.second = (buf[11] - '0') * 10 + (buf[12] - '0');
//			tmp = rp_GetComma(9, buf);
//			GPS->D.day = (buf[tmp + 0] - '0') * 10 + (buf[tmp + 1] - '0'); // fecha
//			GPS->D.month = (buf[tmp + 2] - '0') * 10 + (buf[tmp + 3] - '0');
//			GPS->D.year = (buf[tmp + 4] - '0') * 10 + (buf[tmp + 5] - '0') + 2000;
ptr = strchr(ptr, ',')+1; //skip time
//loc->gtime = atof(ptr);
loc->D.hour = *(ptr-'0')*10 + *(ptr+1 - '0'); //(buf[7] - '0') * 10 + (buf[8] - '0'); // Tiempo
loc->D.minute = *(ptr+2 - '0')*10 + *(ptr+3 - '0'); //(buf[9] - '0') * 10 + (buf[10] - '0');
loc->D.second = *(ptr+4 - '0')*10 + *(ptr+5 - '0'); //(buf[11] - '0') * 10 + (buf[12] - '0');
ptr = strchr(ptr, ',')+1; //skip status

ptr = strchr(ptr, ',')+1;
loc->latitude = atof(ptr);

ptr = strchr(ptr, ',')+1;
switch (ptr[0]) {
case 'N':
loc->NS = 'N';
break;
case 'S':
loc->NS = 'S';
break;
case ',':
loc->NS = '\0';
break;
}

ptr = strchr(ptr, ',')+1;
loc->longitude = atof(ptr);

ptr = strchr(ptr, ',')+1;
switch (ptr[0]) {
case 'W':
loc->EW = 'W';
break;
case 'E':
loc->EW = 'E';
break;
case ',':
loc->EW = '\0';
break;
}

ptr = strchr(ptr, ',')+1;
loc->speed = atof(ptr);

ptr = strchr(ptr, ',')+1;
loc->course = atof(ptr);
}*/

/**
 * Get the message type (GPGGA, GPRMC, etc..)
 *
 * This function filters out also wrong packages (invalid checksum)
 *
 * @param message The NMEA message
 * @return The type of message if it is valid
 */
uint8_t rp_NmeaGetMessageType(const char *message){
	uint8_t checksum = 0;
	if ((checksum = rp_NmeaValidChecksum(message)) != _EMPTY) {
		return checksum;
	}

	if (strstr(message, NMEA_GPGGA_STR) != NULL) {
		return NMEA_GPGGA;
	}

	if (strstr(message, NMEA_GPRMC_STR) != NULL) {
		return NMEA_GPRMC;
	}

	return NMEA_UNKNOWN;
}

uint8_t rp_NmeaValidChecksum(const char *message) {
	uint8_t checksum= (uint8_t)strtol(strchr(message, '*')+1, NULL, 16);

	char p;
	uint8_t sum = 0;
	++message;
	while ((p = *message++) != '*') {
		sum ^= p;
	}

	if (sum != checksum) {
		return NMEA_CHECKSUM_ERR;
	}

	return _EMPTY;
}

