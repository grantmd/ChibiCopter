/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Talks to a GPS receiver over serial
*/

#include <string.h>

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

uint8_t hasFix(void){
	chMtxLock(&gps_data.mtx);
	uint8_t hasFix = gps_data.hasFix;
	chMtxUnlock();

	return hasFix;
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
			gps_state.term[gps_state.term_offset++] = c;
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

	// Other terms vary depending on sentence type
	switch (gps_state.sentence_type){
		case GPS_SENTENCE_GPGGA:
			switch (gps_state.sentence_offset){
				case 1: // UTC
					break;
				case 2: // Latitude
					break;
				case 3: // N or S (North or South)
					break;
				case 4: // Longitude
					break;
				case 5: // E or W (East or West)
					break;
				case 6: // GPS Quality Indicator (fix type)
					break;
				case 7: // Number of satellites in view
					break;
				case 8: // Horizontal Dilution of precision (meters)
					break;
				case 9: // Antenna Altitude above/below mean-sea-level (geoid) (in meters)
					break;
				case 10: // Units of antenna altitude, meters
					break;
				case 11: // Geoidal separation
					break;
				case 12: // Units of geoidal separation, meters
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
					break;
				case 3: // Latitude
					break;
				case 4: // N or S (North or South)
					break;
				case 5: // Longitude
					break;
				case 6: // E or W (East or West)
					break;
				case 7: // Speed over ground, knots
					break;
				case 8: // Track made good, degrees true
					break;
				case 9: // Date, ddmmyy
					break;
				case 10: // Magnetic Variation, degrees
					break;
				case 11: // E or W
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
					break;
				case 2: // N or S (North or South)
					break;
				case 3: // Longitude
					break;
				case 4: // E or W (East or West)
					break;
				case 5: // UTC
					break;
				case 6: // Status A - Data Valid, V - Data Invalid
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
					break;
				case 3: // ID of 1st satellite used for fix
					break;
				case 4: // ID of 2nd satellite used for fix
					break;
				case 5: // ID of 3rd satellite used for fix
					break;
				case 6: // ID of 4th satellite used for fix
					break;
				case 7: // ID of 5th satellite used for fix
					break;
				case 8: // ID of 6th satellite used for fix
					break;
				case 9: // ID of 7th satellite used for fix
					break;
				case 10: // ID of 8th satellite used for fix
					break;
				case 11: // ID of 9th satellite used for fix
					break;
				case 12: // ID of 10th satellite used for fix
					break;
				case 13: // ID of 11th satellite used for fix
					break;
				case 14: // ID of 12th satellite used for fix
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
					break;
				case 3: // Track Degrees
					break;
				case 4: // M = Magnetic
					break;
				case 5: // Speed Knots
					break;
				case 6: // N = Knots
					break;
				case 7: // Speed Kilometers Per Hour
					break;
				case 8: // K = Kilometers Per Hour
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