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

#include <math.h>

#include "ch.h"
#include "hal.h"
#include "TinyGPS.h"

#define _GPRMC_TERM   "GPRMC"
#define _GPGGA_TERM   "GPGGA"

// properties
unsigned long _time, _new_time = GPS_INVALID_TIME;
unsigned long _date, _new_date = GPS_INVALID_DATE;
long _latitude, _new_latitude = GPS_INVALID_ANGLE;
long _longitude, _new_longitude = GPS_INVALID_ANGLE;
long _altitude, _new_altitude = GPS_INVALID_ALTITUDE;
unsigned long  _speed, _new_speed = GPS_INVALID_SPEED;
unsigned long  _course, _new_course = GPS_INVALID_ANGLE;

unsigned long _last_time_fix, _new_time_fix = GPS_INVALID_FIX_TIME;
unsigned long _last_position_fix, _new_position_fix = GPS_INVALID_FIX_TIME;

// parsing state variables
char _parity = 0;
unsigned char _is_checksum_term = 0;
char _term[15];
char _sentence_type = _GPS_SENTENCE_OTHER;
char _term_number = 0;
unsigned char _term_offset = 0;
unsigned char _gps_data_good = 0;

#ifndef _GPS_NO_STATS
// statistics
unsigned long _encoded_characters = 0;
unsigned short _good_sentences = 0;
unsigned short _failed_checksum = 0;
unsigned short _passed_checksum = 0;
#endif

/*static void TinyGPS_init(void){
  _term[0] = '\0';
}*/

//
// public methods
//

unsigned char TinyGPS_encode(char c)
{
  unsigned char valid_sentence = 0;

  ++_encoded_characters;
  switch(c)
  {
  case ',': // term terminators
    _parity ^= c;
  case '\r':
  case '\n':
  case '*':
    if (_term_offset < sizeof(_term))
    {
      _term[_term_offset] = 0;
      valid_sentence = _term_complete();
    }
    ++_term_number;
    _term_offset = 0;
    _is_checksum_term = c == '*';
    return valid_sentence;

  case '$': // sentence begin
    _term_number = _term_offset = 0;
    _parity = 0;
    _sentence_type = _GPS_SENTENCE_OTHER;
    _is_checksum_term = 0;
    _gps_data_good = 0;
    return valid_sentence;
  }

  // ordinary characters
  if (_term_offset < sizeof(_term) - 1)
    _term[_term_offset++] = c;
  if (!_is_checksum_term)
    _parity ^= c;

  return valid_sentence;
}

// lat/long in hundred thousandths of a degree and age of fix in milliseconds
inline void TinyGPS_get_position(long *latitude, long *longitude, unsigned long *fix_age)
{
  if (latitude) *latitude = _latitude;
  if (longitude) *longitude = _longitude;
  if (fix_age) *fix_age = _last_position_fix == GPS_INVALID_FIX_TIME ? 
    GPS_INVALID_AGE : chTimeNow() - _last_position_fix;
}

// date as ddmmyy, time as hhmmsscc, and age in milliseconds
inline void TinyGPS_get_datetime(unsigned long *date, unsigned long *time, unsigned long *fix_age)
{
  if (date) *date = _date;
  if (time) *time = _time;
  if (fix_age) *fix_age = _last_time_fix == GPS_INVALID_FIX_TIME ? 
    GPS_INVALID_AGE : chTimeNow() - _last_time_fix;
}

// signed altitude in centimeters (from GPGGA sentence)
inline long TinyGPS_altitude(void) { return _altitude; }

// course in last full GPRMC sentence in 100th of a degree
inline unsigned long TinyGPS_course(void) { return _course; }

// speed in last full GPRMC sentence in 100ths of a knot
unsigned long TinyGPS_speed(void) { return _speed; }


#ifndef _GPS_NO_STATS
void TinyGPS_stats(unsigned long *chars, unsigned short *sentences, unsigned short *failed_cs)
{
  if (chars) *chars = _encoded_characters;
  if (sentences) *sentences = _good_sentences;
  if (failed_cs) *failed_cs = _failed_checksum;
}
#endif

inline void TinyGPS_f_get_position(float *latitude, float *longitude, unsigned long *fix_age)
{
  long lat, lon;
  TinyGPS_get_position(&lat, &lon, fix_age);
  *latitude = lat / 100000.0;
  *longitude = lon / 100000.0;
}

inline void TinyGPS_crack_datetime(int *year, char *month, char *day, 
  char *hour, char *minute, char *second, char *hundredths, unsigned long *fix_age)
{
  unsigned long date, time;
  TinyGPS_get_datetime(&date, &time, fix_age);
  if (year) 
  {
    *year = date % 100;
    *year += *year > 80 ? 1900 : 2000;
  }
  if (month) *month = (date / 100) % 100;
  if (day) *day = date / 10000;
  if (hour) *hour = time / 1000000;
  if (minute) *minute = (time / 10000) % 100;
  if (second) *second = (time / 100) % 100;
  if (hundredths) *hundredths = time % 100;
}

inline float TinyGPS_f_altitude(void)    { return TinyGPS_altitude() / 100.0; }
inline float TinyGPS_f_course(void)      { return TinyGPS_course() / 100.0; }
inline float TinyGPS_f_speed_knots(void) { return TinyGPS_speed() / 100.0; }
inline float TinyGPS_f_speed_mph(void)   { return _GPS_MPH_PER_KNOT * TinyGPS_f_speed_knots(); }
inline float TinyGPS_f_speed_mps(void)   { return _GPS_MPS_PER_KNOT * TinyGPS_f_speed_knots(); }
inline float TinyGPS_f_speed_kmph(void)  { return _GPS_KMPH_PER_KNOT * TinyGPS_f_speed_knots(); }

inline int TinyGPS_library_version(void) { return _GPS_VERSION; }

//
// internal utilities
//
int _from_hex(char a) 
{
  if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else
    return a - '0';
}

unsigned long _parse_decimal(void)
{
  char *p = _term;
  unsigned char isneg = *p == '-';
  if (isneg) ++p;
  unsigned long ret = 100UL * _gpsatol(p);
  while (_gpsisdigit(*p)) ++p;
  if (*p == '.')
  {
    if (_gpsisdigit(p[1]))
    {
      ret += 10 * (p[1] - '0');
      if (_gpsisdigit(p[2]))
        ret += p[2] - '0';
    }
  }
  return isneg ? -ret : ret;
}

unsigned long _parse_degrees(void)
{
  char *p;
  unsigned long left = _gpsatol(_term);
  unsigned long tenk_minutes = (left % 100UL) * 10000UL;
  for (p=_term; _gpsisdigit(*p); ++p);
  if (*p == '.')
  {
    unsigned long mult = 1000;
    while (_gpsisdigit(*++p))
    {
      tenk_minutes += mult * (*p - '0');
      mult /= 10;
    }
  }
  return (left / 100) * 100000 + tenk_minutes / 6;
}

// Processes a just-completed term
// Returns true if new sentence has just passed checksum test and is validated
unsigned char _term_complete(void)
{
  if (_is_checksum_term)
  {
    char checksum = 16 * _from_hex(_term[0]) + _from_hex(_term[1]);
    if (checksum == _parity)
    {
      if (_gps_data_good)
      {
#ifndef _GPS_NO_STATS
        ++_good_sentences;
#endif
        _last_time_fix = _new_time_fix;
        _last_position_fix = _new_position_fix;

        switch(_sentence_type)
        {
        case _GPS_SENTENCE_GPRMC:
          _time      = _new_time;
          _date      = _new_date;
          _latitude  = _new_latitude;
          _longitude = _new_longitude;
          _speed     = _new_speed;
          _course    = _new_course;
          break;
        case _GPS_SENTENCE_GPGGA:
          _altitude  = _new_altitude;
          _time      = _new_time;
          _latitude  = _new_latitude;
          _longitude = _new_longitude;
          break;
        }

        return 1;
      }
    }

#ifndef _GPS_NO_STATS
    else
      ++_failed_checksum;
#endif
    return 0;
  }

  // the first term determines the sentence type
  if (_term_number == 0)
  {
    if (!_gpsstrcmp(_term, _GPRMC_TERM))
      _sentence_type = _GPS_SENTENCE_GPRMC;
    else if (!_gpsstrcmp(_term, _GPGGA_TERM))
      _sentence_type = _GPS_SENTENCE_GPGGA;
    else
      _sentence_type = _GPS_SENTENCE_OTHER;
    return 0;
  }

  if (_sentence_type != _GPS_SENTENCE_OTHER && _term[0])
  switch((_sentence_type == _GPS_SENTENCE_GPGGA ? 200 : 100) + _term_number)
  {
    case 101: // Time in both sentences
    case 201:
      _new_time = _parse_decimal();
      _new_time_fix = chTimeNow(); // millis
      break;
    case 102: // GPRMC validity
      _gps_data_good = _term[0] == 'A';
      break;
    case 103: // Latitude
    case 202:
      _new_latitude = _parse_degrees();
      _new_position_fix = chTimeNow(); // millis
      break;
    case 104: // N/S
    case 203:
      if (_term[0] == 'S')
        _new_latitude = -_new_latitude;
      break;
    case 105: // Longitude
    case 204:
      _new_longitude = _parse_degrees();
      break;
    case 106: // E/W
    case 205:
      if (_term[0] == 'W')
        _new_longitude = -_new_longitude;
      break;
    case 107: // Speed (GPRMC)
      _new_speed = _parse_decimal();
      break;
    case 108: // Course (GPRMC)
      _new_course = _parse_decimal();
      break;
    case 109: // Date (GPRMC)
      _new_date = _gpsatol(_term);
      break;
    case 206: // Fix data (GPGGA)
      _gps_data_good = _term[0] > '0';
      break;
    case 209: // Altitude (GPGGA)
      _new_altitude = _parse_decimal();
      break;
  }

  return 0;
}

inline unsigned char _gpsisdigit(char c) { return c >= '0' && c <= '9'; }

long _gpsatol(const char *str)
{
  long ret = 0;
  while (_gpsisdigit(*str))
    ret = 10 * ret + *str++ - '0';
  return ret;
}

int _gpsstrcmp(const char *str1, const char *str2)
{
  while (*str1 && *str1 == *str2)
    ++str1, ++str2;
  return *str1;
}

inline float _radians(float degrees) { return degrees * PI / 180.0; }

float TinyGPS_distance_between (float lat1, float long1, float lat2, float long2) 
{
  // returns distance in meters between two positions, both specified 
  // as signed decimal-degrees latitude and longitude. Uses great-circle 
  // distance computation for hypothetical sphere of radius 6372795 meters.
  // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
  // Courtesy of Maarten Lamers
  float delta = _radians(long1-long2);
  float sdlong = sin(delta);
  float cdlong = cos(delta);
  lat1 = _radians(lat1);
  lat2 = _radians(lat2);
  float slat1 = sin(lat1);
  float clat1 = cos(lat1);
  float slat2 = sin(lat2);
  float clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong); 
  delta = pow(delta, 2); 
  delta += pow(clat2 * sdlong, 2); 
  delta = sqrt(delta); 
  float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong); 
  delta = atan2(delta, denom); 
  return delta * 6372795; 
}
