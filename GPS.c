/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Talks to a GPS receiver over serial
*/

#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "GPS.h"

gps_state_t gps_state; // Parsing state
gps_data_t gps_data; // Valid lat/lon etc data
gps_data_t gps_working_data; // Where we store data we're working on
gps_satellites_t gps_satellites; // Valid satellite data
gps_satellites_t gps_working_satellites; // Where we store satellite data we're working on

/*
 * GPS setup
 */
static const SerialConfig sd1cfg = {
	57600,
	0,
	USART_CR2_STOP1_BITS | USART_CR2_LINEN,
	0
};

static WORKING_AREA(GPSWA, 128);
static msg_t GPS(void *arg){

	(void)arg;
	chRegSetThreadName("GPS");

	while (TRUE){
		// Read a byte off the GPS
		uint8_t c = chnGetTimeout((BaseChannel *)&SD1, TIME_INFINITE);
		chnPutTimeout(&SD2, c, TIME_INFINITE);
		if (GPSParseChar(c)){
      		palSetPad(GPIOD, GPIOD_LED5); // red
		}
	}
	return 0;
}

/*
 * Activates the serial driver 1 using the driver default configuration, but at 57600
 * PA9(TX) and PA10(RX) are routed to USART1.
 */

void GPSInit(void){

	sdStart(&SD1, &sd1cfg);
	palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7)); // not currently connected
	palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7)); // incoming data from the GPS

	GPSParserInit();

	chThdCreateStatic(GPSWA, sizeof(GPSWA), NORMALPRIO, GPS, NULL);
}

/*
 * Public functions for accessing our parsed and validated results in a thread-safe manner
 */

uint8_t fixType(void){
	chMtxLock(&gps_data.mtx);
	uint8_t fixType = gps_data.fixType;
	chMtxUnlock();

	return fixType;
}

int32_t getLatitude(void){
	chMtxLock(&gps_data.mtx);
	int32_t lat = gps_data.lat;
	chMtxUnlock();

	return lat;
}

int32_t getLongitude(void){
	chMtxLock(&gps_data.mtx);
	int32_t lon = gps_data.lon;
	chMtxUnlock();

	return lon;
}

int32_t getAltitude(void){
	chMtxLock(&gps_data.mtx);
	int32_t alt = gps_data.alt;
	chMtxUnlock();

	return alt;
}

unsigned long getCourse(void){
	chMtxLock(&gps_data.mtx);
	chMtxUnlock();
}

unsigned long getSpeed(void){
	chMtxLock(&gps_data.mtx);
	chMtxUnlock();
}

uint8_t getNumSatellites(void){
	chMtxLock(&gps_satellites.mtx);
	uint8_t numSatellites = gps_satellites.numVisible;
	chMtxUnlock();

	return numSatellites;
}

unsigned long getHdop(void){
	chMtxLock(&gps_data.mtx);
	chMtxUnlock();
}

/*
 * Begin parsing logic
 */

/*
 * Initializes our GPS/NMEA parser
 */

void GPSParserInit(void){
	gps_state.sentence_type = GPS_SENTENCE_UNKNOWN;
	gps_state.valid = 0;
	gps_state.sentence_offset = gps_state.term_offset = 0;

	// Init our mutexes on the data structs
	chMtxInit(&gps_data.mtx);
	chMtxInit(&gps_satellites.mtx);
}

/*
 * Parse a character from incoming GPS data using a state machine
 * Returns whether we have completed a valid sentence
 */

char GPSParseChar(char c){
	switch (c){
		case ',':
			// end of term
			_GPSParseTerm();
			gps_state.sentence_offset++;
			gps_state.term_offset = 0;
			break;

		case '\r':
		case '\n':
		case '*':
			// end of sentence/term
			_GPSParseTerm();
			_GPSParseSentence();
			gps_state.sentence_offset = gps_state.term_offset = 0;
			break;

		case '$':
			// start of sentence
			gps_state.sentence_type = GPS_SENTENCE_UNKNOWN;
			gps_state.sentence_offset = gps_state.term_offset = 0;
			gps_state.valid = 0;
			break;

		default:
			// Regular data
			if (gps_state.term_offset < sizeof(gps_state.term)-1) gps_state.term[gps_state.term_offset++] = c;
			break;
	}

	return gps_state.valid;
}

/*
 * Parse a just-completed term and stash to working storage
 */
void _GPSParseTerm(void){
	// First term is always sentence type
	if (gps_state.sentence_offset == 0){
		if (strncmp(gps_state.term, _GPGGA_TERM, gps_state.term_offset) == 0){
			gps_state.sentence_type = GPS_SENTENCE_GPGGA;
		}
		else if (strncmp(gps_state.term, _GPRMC_TERM, gps_state.term_offset) == 0){
			gps_state.sentence_type = GPS_SENTENCE_GPRMC;
		}
		else if (strncmp(gps_state.term, _GPGLL_TERM, gps_state.term_offset) == 0){
			gps_state.sentence_type = GPS_SENTENCE_GPGLL;
		}
		else if (strncmp(gps_state.term, _GPGSA_TERM, gps_state.term_offset) == 0){
			gps_state.sentence_type = GPS_SENTENCE_GPGSA;
		}
		else if (strncmp(gps_state.term, _GPGSV_TERM, gps_state.term_offset) == 0){
			gps_state.sentence_type = GPS_SENTENCE_GPGSV;
		}
		else if (strncmp(gps_state.term, _GPVTG_TERM, gps_state.term_offset) == 0){
			gps_state.sentence_type = GPS_SENTENCE_GPVTG;
		}

		return;
	}

	// Null-terminate the term
	if (gps_state.term_offset < sizeof(gps_state.term)) gps_state.term[gps_state.term_offset] = '\0';

	// Other terms vary depending on sentence type
	switch (gps_state.sentence_type){
		case GPS_SENTENCE_GPGGA:
			switch (gps_state.sentence_offset){
				case 1: // UTC
					break;
				case 2: // Latitude
					gps_working_data.lat = _GPSParseLatLonTerm();
					break;
				case 3: // N or S (North or South)
					if (strncmp(gps_state.term, "S", 1) == 0) gps_working_data.lat *= -1;
					break;
				case 4: // Longitude
					gps_working_data.lon = _GPSParseLatLonTerm();
					break;
				case 5: // E or W (East or West)
					if (strncmp(gps_state.term, "W", 1) == 0) gps_working_data.lon *= -1;
					break;
				case 6: // GPS Quality Indicator (fix type)
					gps_working_data.dataGood = (strncmp(gps_state.term, "0", 1) > 0) ? 1 : 0;
					if (strncmp(gps_state.term, "0", 1) == 0) gps_working_data.fixType = 0;
					break;
				case 7: // Number of satellites in view
					gps_working_satellites.numVisible = atoi(gps_state.term);
					break;
				case 8: // Horizontal Dilution of precision (meters)
					break;
				case 9: // Antenna Altitude above/below mean-sea-level (geoid) (in meters)
					break;
				case 10: // Units of antenna altitude, meters
					// Ignored, always M
					break;
				case 11: // Geoidal separation
					break;
				case 12: // Units of geoidal separation, meters
					// Ignored, always M
					break;
				case 13: // Age of differential GPS data
					break;
				case 14: // Differential reference station ID
					break;
				default:
					break;
			}
			break;

		case GPS_SENTENCE_GPRMC:
			switch (gps_state.sentence_offset){
				case 1: // UTC
					break;
				case 2: // Status, V=Navigation receiver warning A=Valid
					gps_working_data.dataGood = (strncmp(gps_state.term, "A", 1) == 0) ? 1 : 0;
					break;
				case 3: // Latitude
					gps_working_data.lat = _GPSParseLatLonTerm();
					break;
				case 4: // N or S (North or South)
					if (strncmp(gps_state.term, "S", 1) == 0) gps_working_data.lat *= -1;
					break;
				case 5: // Longitude
					gps_working_data.lon = _GPSParseLatLonTerm();
					break;
				case 6: // E or W (East or West)
					if (strncmp(gps_state.term, "W", 1) == 0) gps_working_data.lon *= -1;
					break;
				case 7: // Speed over ground, knots
					break;
				case 8: // Track made good, degrees true
					break;
				case 9: // Date, ddmmyy
					break;
				case 10: // Magnetic Variation, degrees
					// I don't appear to have this on my GPS
					break;
				case 11: // E or W
					// I don't appear to have this on my GPS
					break;
				case 12: // FAA mode indicator (NMEA 2.3 and later)
					break;
				default:
					break;
			}
			break;

		case GPS_SENTENCE_GPGLL:
			switch (gps_state.sentence_offset){
				case 1: // Latitude
					gps_working_data.lat = _GPSParseLatLonTerm();
					break;
				case 2: // N or S (North or South)
					if (strncmp(gps_state.term, "S", 1) == 0) gps_working_data.lat *= -1;
					break;
				case 3: // Longitude
					gps_working_data.lon = _GPSParseLatLonTerm();
					break;
				case 4: // E or W (East or West)
					if (strncmp(gps_state.term, "W", 1) == 0) gps_working_data.lon *= -1;
					break;
				case 5: // UTC
					break;
				case 6: // Status A - Data Valid, V - Data Invalid
					gps_working_data.dataGood = (strncmp(gps_state.term, "A", 1) == 0) ? 1 : 0;
					break;
				case 7: // FAA mode indicator (NMEA 2.3 and later)
					break;
				default:
					break;
			}
			break;

		case GPS_SENTENCE_GPGSA:
			switch (gps_state.sentence_offset){
				case 1: // Selection mode: M=Manual, forced to operate in 2D or 3D, A=Automatic, 3D/2D
					break;
				case 2: // Mode (1 = no fix, 2 = 2D fix, 3 = 3D fix)
					gps_working_data.dataGood = (strncmp(gps_state.term, "0", 1) > 0) ? 1 : 0;
					gps_working_data.fixType = gps_state.term[0];
					break;
				case 3: // ID of 1st satellite used for fix
					gps_working_satellites.prn[0] = atoi(gps_state.term);
					break;
				case 4: // ID of 2nd satellite used for fix
					gps_working_satellites.prn[1] = atoi(gps_state.term);
					break;
				case 5: // ID of 3rd satellite used for fix
					gps_working_satellites.prn[2] = atoi(gps_state.term);
					break;
				case 6: // ID of 4th satellite used for fix
					gps_working_satellites.prn[3] = atoi(gps_state.term);
					break;
				case 7: // ID of 5th satellite used for fix
					gps_working_satellites.prn[4] = atoi(gps_state.term);
					break;
				case 8: // ID of 6th satellite used for fix
					gps_working_satellites.prn[5] = atoi(gps_state.term);
					break;
				case 9: // ID of 7th satellite used for fix
					gps_working_satellites.prn[6] = atoi(gps_state.term);
					break;
				case 10: // ID of 8th satellite used for fix
					gps_working_satellites.prn[7] = atoi(gps_state.term);
					break;
				case 11: // ID of 9th satellite used for fix
					gps_working_satellites.prn[8] = atoi(gps_state.term);
					break;
				case 12: // ID of 10th satellite used for fix
					gps_working_satellites.prn[9] = atoi(gps_state.term);
					break;
				case 13: // ID of 11th satellite used for fix
					gps_working_satellites.prn[10] = atoi(gps_state.term);
					break;
				case 14: // ID of 12th satellite used for fix
					gps_working_satellites.prn[11] = atoi(gps_state.term);
					break;
				case 15: // PDOP
					break;
				case 16: // HDOP
					break;
				case 17: // VDOP
					break;
				default:
					break;
			}
			break;

		case GPS_SENTENCE_GPGSV:
			switch (gps_state.sentence_offset){
				case 1: // total number of GSV messages to be transmitted in this group
					break;
				default: // The rest of this is fucking complicated
					break;
			}
			break;

		case GPS_SENTENCE_GPVTG:
			switch (gps_state.sentence_offset){
				case 1: // Track Degrees
					break;
				case 2: // T = True
					// Ignored, always T
					break;
				case 3: // Track Degrees
					break;
				case 4: // M = Magnetic
					// Ignored, always M
					break;
				case 5: // Speed Knots
					break;
				case 6: // N = Knots
					// Ignored, always N
					break;
				case 7: // Speed Kilometers Per Hour
					break;
				case 8: // K = Kilometers Per Hour
					// Ignored, always K
					break;
				case 9: // FAA mode indicator (NMEA 2.3 and later)
					break;
				default:
					break;
			}
			break;

		case GPS_SENTENCE_UNKNOWN:
		default:
			break;
	}
}

/*
 * Special logic for parsing lat/lon terms. Takes a string like '4533.35' and return 4555583
 */

/*int32_t _GPSParseLatLonTerm(void){
	double parsed_d = atof(gps_state.term);
	int32_t parsed_i = atoi(gps_state.term);

	double minutes = (parsed_i % 100) + (parsed_d - parsed_i);
	int16_t degrees = parsed_i / 100;

	return (degrees * 100000) + ((minutes / 6) * 10000);
}*/

int32_t _GPSParseLatLonTerm(void){
	char *p;
	unsigned long left = _GPSatol(gps_state.term); // Convert characters to the left of the decimal point to a number
	unsigned long tenk_minutes = (left % 100UL) * 10000UL; // Extract the minutes (eg in 4533, the minutes are 33)
	for (p=gps_state.term; _GPSIsDigit(*p); ++p); // Move pointer to the first non-digit
	if (*p == '.'){ // If we found a decimal point, extract seconds
		unsigned long mult = 1000;
		while (_GPSIsDigit(*++p)){
			tenk_minutes += mult * (*p - '0'); // Add the seconds after the minutes
			mult /= 10;
		}
	}
	return (left / 100) * 100000 + tenk_minutes / 6; // Extract degrees from left (45), stick it at the front, add minutes converted
}

/*
 * Convert an array of characters to a long
 */
long _GPSatol(const char *str){
	long ret = 0;
	while (_GPSIsDigit(*str))
		ret = 10 * ret + *str++ - '0';
	return ret;
}

/*
 * Is this character a digit from 0-9?
 */
char _GPSIsDigit(char c){
	return c >= '0' && c <= '9';
}

/*
 * Parse a just-completed sentence, verify checksum, and copy working storage to public storage in a thread-safe manner
 * Sets gps_state.valid if sentence was valid and copied to public storage
 */
void _GPSParseSentence(void){
	// TODO: Add checksumming and return early if invalid

	switch(gps_state.sentence_type){
		case GPS_SENTENCE_GPGGA:
			chMtxLock(&gps_data.mtx);
			// Copy data
			chMtxUnlock();
			break;
		case GPS_SENTENCE_GPRMC:
			break;
		case GPS_SENTENCE_GPGLL:
			break;
		case GPS_SENTENCE_GPGSA:
			break;
		case GPS_SENTENCE_GPGSV:
			break;
		case GPS_SENTENCE_GPVTG:
			break;
		default:
			// hmmmmm.....
			break;
	}
}