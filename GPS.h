/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Talks to a GPS over serial
*/

#include "ch.h"
#include "hal.h"

#ifndef _GPS_H_
#define _GPS_H_

// http://gpsd.berlios.de/NMEA.txt
#define GPS_SENTENCE_UNKNOWN 0
#define GPS_SENTENCE_GPGGA 1 // Time, position, and fix/satellite info
#define GPS_SENTENCE_GPRMC 2 // Time, position, some speed/heading
#define GPS_SENTENCE_GPGLL 3 // Time, position
#define GPS_SENTENCE_GPGSA 4 // GPS DOP and active satellites
#define GPS_SENTENCE_GPGSV 5 // Satellites in view
#define GPS_SENTENCE_GPVTG 6 // Track made good and Ground speed

#define _GPGGA_TERM   "GPGGA"
#define _GPRMC_TERM   "GPRMC"
#define _GPGLL_TERM   "GPGLL"
#define _GPGSA_TERM   "GPGSA"
#define _GPGSV_TERM   "GPGSV"
#define _GPVTG_TERM   "GPVTG"

// A struct to maintain decoding state
// No mutex here since access is single-threaded
typedef struct {
	uint8_t sentence_type;
	char term[15];
	uint8_t sentence_offset;
	uint8_t term_offset;
	char valid;
} gps_state_t;

// A struct to hold our decoded gps data
typedef struct {
	uint8_t fixType;
	uint8_t dataGood;
	int32_t lat;
	int32_t lon;
	int32_t alt;
	uint16_t eph;
	uint16_t epv;
	uint16_t vel;
	uint16_t cog;
	Mutex mtx;
} gps_data_t;

// A struct to hold satellite data
typedef struct {
	uint8_t numVisible; // Number of satellites visible
	uint8_t prn[12]; // Global satellite ID
	uint8_t elevation[12]; // Elevation (0: right on top of receiver, 90: on the horizon) of satellite
	uint8_t azimuth[12]; // Direction of satellite, 0: 0 deg, 255: 360 deg.
	uint8_t snr[12]; // Signal to noise ratio of satellite
	Mutex mtx;
} gps_satellites_t;

// Public functions
void GPSInit(void);
void GPSParserInit(void);
char GPSParseChar(char);

uint8_t fixType(void);
int32_t getLatitude(void); // lat in hundred thousandths of a degree
int32_t getLongitude(void); // long in hundred thousandths of a degree

int32_t getAltitude(void); // signed altitude in centimeters (from GPGGA sentence)
unsigned long getCourse(void); // course in last full GPRMC sentence in 100th of a degree
unsigned long getSpeed(void); // speed in last full GPRMC sentence in 100ths of a knot
uint8_t getNumSatellites(void); // number of satellites used in last full GPGGA sentence
unsigned long getHdop(void); // horizontal dilution of precision in 100ths

// Private functions
void _GPSParseTerm(void);
int32_t _parseLatLonTerm(void);
void _GPSParseSentence(void);

#endif /* _GPS_H_ */