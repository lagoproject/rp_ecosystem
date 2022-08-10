/**
 * $Id: $
 *
 * @brief GPS stuffs module.
 *
 * @Author L. Horacio Arnaldi <lharnaldi@gmail.com>
 *
 * (c) LabDPR  http://labdpr.cab.cnea.gov.ar
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef _GPS_RP_H_
#define _GPS_RP_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
}DTIME;


typedef struct location {
    //double gtime;
    //double latitude;
    //double longitude;
    //double speed;
    //double altitude;
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
    DTIME   D;
} loc_t;

// Initialize device
void gps_init(void);
// Activate device
void gps_on(void);
// Get the actual location
void gps_location(loc_t *);


// Turn off device (low-power consumption)
void gps_off(void);

// -------------------------------------------------------------------------
// Internal functions
// -------------------------------------------------------------------------

// convert deg to decimal deg latitude, (N/S), longitude, (W/E)
void gps_convert_deg_to_dec(double *, char, double *, char);
double gps_deg_dec(double);

#endif
