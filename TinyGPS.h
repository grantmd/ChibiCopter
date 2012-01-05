/*
  TinyGPS - a small GPS library for Arduino providing basic NMEA parsing
  Based on work by and "distance_to" courtesy of Maarten Lamers.
  Copyright (C) 2008-2011 Mikal Hart
  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _TINYGPS_H_
#define _TINYGPS_H_

#include "ch.h"
#include "hal.h"

#define PI 3.14159265
#define _GPS_VERSION 10 // software version of this library
#define _GPS_MPH_PER_KNOT 1.15077945
#define _GPS_MPS_PER_KNOT 0.51444444
#define _GPS_KMPH_PER_KNOT 1.852
#define _GPS_MILES_PER_METER 0.00062137112
#define _GPS_KM_PER_METER 0.001
//#define _GPS_NO_STATS

enum {GPS_INVALID_AGE = 0xFFFFFFFF, GPS_INVALID_ANGLE = 999999999, GPS_INVALID_ALTITUDE = 999999999, GPS_INVALID_DATE = 0,
  GPS_INVALID_TIME = 0xFFFFFFFF, GPS_INVALID_SPEED = 999999999, GPS_INVALID_FIX_TIME = 0xFFFFFFFF};

// private
enum {_GPS_SENTENCE_GPGGA, _GPS_SENTENCE_GPRMC, _GPS_SENTENCE_OTHER};

// internal utilities
int _from_hex(char a);
unsigned long _parse_decimal(void);
unsigned long _parse_degrees(void);
unsigned char _term_complete(void);
inline unsigned char _gpsisdigit(char c);
long _gpsatol(const char *str);
int _gpsstrcmp(const char *str1, const char *str2);
inline float _radians(float degrees);


// public
//static void TinyGPS_init(void);
unsigned char TinyGPS_encode(char c); // process one character received from GPS

// lat/long in hundred thousandths of a degree and age of fix in milliseconds
inline void TinyGPS_get_position(long *latitude, long *longitude, unsigned long *fix_age);

// date as ddmmyy, time as hhmmsscc, and age in milliseconds
inline void TinyGPS_get_datetime(unsigned long *date, unsigned long *time, unsigned long *fix_age);

// signed altitude in centimeters (from GPGGA sentence)
inline long TinyGPS_altitude(void);

// course in last full GPRMC sentence in 100th of a degree
inline unsigned long TinyGPS_course(void);

// speed in last full GPRMC sentence in 100ths of a knot
unsigned long TinyGPS_speed(void);

#ifndef _GPS_NO_STATS
void TinyGPS_stats(unsigned long *chars, unsigned short *good_sentences, unsigned short *failed_cs);
#endif

inline void TinyGPS_f_get_position(float *latitude, float *longitude, unsigned long *fix_age);

inline void TinyGPS_crack_datetime(int *year, char *month, char *day, 
  char *hour, char *minute, char *second, char *hundredths, unsigned long *fix_age);

inline float TinyGPS_f_altitude(void);
inline float TinyGPS_f_course(void);
inline float TinyGPS_f_speed_knots(void);
inline float TinyGPS_f_speed_mph(void);
inline float TinyGPS_f_speed_mps(void);
inline float TinyGPS_f_speed_kmph(void);

inline int TinyGPS_library_version(void);

float TinyGPS_distance_between (float lat1, float long1, float lat2, float long2);

#endif /* _TINYGPS_H_ */
