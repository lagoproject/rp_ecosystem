/**
 * $Id: $
 *
 * @brief GPS NMEA messages parser.
 *
 * @Author L. Horacio Arnaldi <lharnaldi@gmail.com>
 *
 * (c) LabDPR  http://labdpr.cab.cnea.gov.ar
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef _NMEA_RP_H_
#define _NMEA_RP_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#define _EMPTY 0x00
#define NMEA_GPRMC 0x01
#define NMEA_GPRMC_STR "$GPRMC"
#define NMEA_GPGGA 0x02
#define NMEA_GPGGA_STR "$GPGGA"
#define NMEA_UNKNOWN 0x00
#define _COMPLETED 0x03

#define NMEA_CHECKSUM_ERR 0x80
#define NMEA_MESSAGE_ERR 0xC0

typedef struct{
    int year; 
    int month; 
    int day;
    int hour;
    int minute;
    int second;
}DATE_TIME;

//typedef struct gdata{
//    double  latitude;         // longitud
//    double  longitude;        // Latitud
//    int     latitude_Degree;  // grado
//    int     latitude_Cent;    // min
//    int     latitude_Second;  // segundos
//    int     longitude_Degree; // grado
//    int     longitude_Cent;   // min
//    int     longitude_Second; // segundos
//    float   speed;            // velocidad
//    float   direction;        // rumbo
//    float   height;           // altitud
//    int     satellite;
//    uint8_t NS;
//    uint8_t EW;
//    DATE_TIME D;
//}GPS_INFO;

typedef struct gpgga {
    double gtime;
    double latitude;
    double longitude;
    uint8_t quality;
    uint8_t satellites;
    float   height;           // altitud
    char    NS;
    char    EW;
} gpgga_t;

typedef struct gprmc {
    //double gtime;
    //double latitude;
    //char lat;
    //double longitude;
    //char lon;
    //double speed;
    //double course;
    double  latitude;         // longitud
    double  longitude;        // Latitud
    int     latitude_Degree;  // grado
    int     latitude_Cent;    // min
    int     latitude_Second;  // segundos
    int     longitude_Degree; // grado
    int     longitude_Cent;   // min
    int     longitude_Second; // segundos
    float   speed;            // velocidad
    float   course;           // rumbo
    float   height;           // altitud
    int     satellite;
    char    NS;
    char    EW;
    DATE_TIME D;
} gprmc_t;

uint8_t rp_NmeaGetMessageType(const char *);
uint8_t rp_NmeaValidChecksum(const char *);
void rp_NmeaParseGpgga(char *, gpgga_t *);
int rp_NmeaParseGprmc(char *, gprmc_t *);

#endif

