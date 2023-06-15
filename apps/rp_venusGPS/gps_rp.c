/*
 * @brief This is a simple application for testing UART communication with a GPS
 * connected to a RedPitaya
 *
 * @Author L. Horacio Arnaldi <lharnaldi@gmail.com>
 *
 * (c) LabDPR  http://labdpr.cab.cnea.gov.ar
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include "gps_rp.h"
#include "nmea_rp.h"
#include "uart_rp.h"

void gps_init(void) {
  rp_UartInit();
  rp_UartConfig();
}

void gps_on(void) {
}

// Compute the GPS location using decimal scale
void gps_location(loc_t *coord) {
  uint8_t status = _EMPTY;
  while(status != _COMPLETED) {
    gpgga_t gpgga;
    gprmc_t gprmc;
    char buffer[256];

    rp_UartReadln(buffer, 256);
    switch (rp_NmeaGetMessageType(buffer)) {
      case NMEA_GPGGA:
        rp_NmeaParseGpgga(buffer, &gpgga);

        gps_convert_deg_to_dec(&(gpgga.latitude), gpgga.NS, &(gpgga.longitude), gpgga.EW);

        coord->latitude = gpgga.latitude;
        coord->longitude = gpgga.longitude;
        coord->height = gpgga.height;
        coord->satellite = gpgga.satellite;

        status |= NMEA_GPGGA;
        break;
      case NMEA_GPRMC:
        rp_NmeaParseGprmc(buffer, &gprmc);

        gps_convert_deg_to_dec(&(gprmc.latitude), gprmc.NS, &(gprmc.longitude), gprmc.EW);
        coord->latitude = gprmc.latitude;
        coord->longitude = gprmc.longitude;
        coord->height = gprmc.height;
        coord->speed = gprmc.speed;
        coord->course = gprmc.course;
        coord->D.hour = gprmc.D.hour;
        coord->D.minute = gprmc.D.minute;
        coord->D.second = gprmc.D.second;
        coord->D.day = gprmc.D.day;
        coord->D.month = gprmc.D.month;
        coord->D.year = gprmc.D.year;


        status |= NMEA_GPRMC;
        break;
    }
  }
}

void gps_off(void) {
  //Write off
  rp_UartClose();
}

// Convert lat e lon to decimals (from deg)
void gps_convert_deg_to_dec(double *latitude, char ns,  double *longitude, char we)
{
  double lat = (ns == 'N') ? *latitude : -1 * (*latitude);
  double lon = (we == 'E') ? *longitude : -1 * (*longitude);

  *latitude = gps_deg_dec(lat);
  *longitude = gps_deg_dec(lon);
}

double gps_deg_dec(double deg_point)
{
  double ddeg;
  double sec = modf(deg_point, &ddeg)*60;
  int deg = (int)(ddeg/100);
  int min = (int)(deg_point-(deg*100));

  double absdlat = round(deg * 1000000.);
  double absmlat = round(min * 1000000.);
  double absslat = round(sec * 1000000.);

  return round(absdlat + (absmlat/60) + (absslat/3600)) /1000000;
}

// GPS Parser **********************************************

// Example: $GPRMC,092741.00,A,5213.13757,N,00008.23605,E,0.272,,180617,,,A*7F
//char fmt[]="$GPRMC,dddtdd.ds,A,eeae.eeee,l,eeeae.eeee,o,jdk,c,dddy";
//
//int state;
//unsigned int temp;
//long ltmp;
//
//// GPS variables
//volatile unsigned int Time, Csecs, Knots, Course, Date;
//volatile long Lat, Long;
//volatile boolean Fix;
//
//void ParseGPS (char c) {
//  if (c == '$') { state = 0; temp = 0; ltmp = 0; }
//  char mode = fmt[state++];
//  // If received character matches format string, return
//  if (mode == c) return;
//  char d = c - '0';
//  // Ignore extra digits of precision
//  if (mode == ',') state--; 
//  // d=decimal digit; j=decimal digits before decimal point
//  else if (mode == 'd') temp = temp*10 + d;
//  else if (mode == 'j') { if (c != '.') { temp = temp*10 + d; state--; } }
//  // e=long decimal digit
//  else if (mode == 'e') ltmp = (ltmp<<3) + (ltmp<<1) + d; // ltmp = ltmp*10 + d;
//  // a=angular measure
//  else if (mode == 'a') ltmp = (ltmp<<2) + (ltmp<<1) + d; // ltmp = ltmp*6 + d;
//  // t=Time - hhmm
//  else if (mode == 't') { Time = temp*10 + d; temp = 0; }
//  // s=Centisecs
//  else if (mode == 's') { Csecs = temp*10 + d; temp = 0; }
//  // l=Latitude - in minutes * 1e-4
//  else if (mode == 'l') { if (c == 'N') Lat = ltmp; else Lat = -ltmp; ltmp = 0; }
//  // o=Longitude - in minutes * 1e-4
//  else if (mode == 'o') { if (c == 'E') Long = ltmp; else Long = -ltmp; ltmp = 0; }
//   // k=Speed - in knots*100
//  else if (mode == 'k') { Knots = temp*10 + d; temp = 0; }
//  // c=Course (Track) - in degrees*100. Allow for empty field.
//  else if (mode == 'c') {
//    if (c == ',') { Course = temp; temp = 0; state++; }
//    else if (c == '.') state--;
//    else { temp = temp*10 + d; state--; }
//  }
//  // y=Date - ddmm
//  else if (mode == 'y') { Date = temp*10 + d ; Fix = 1; state = 0; }
//  else state = 0;
//}
