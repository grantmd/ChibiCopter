/*
    ChibiCopter - https://github.com/grantmd/ChibiCopter
    A quadcopter platform running under ChibiOS/RT.

    Handles communication with the ground/other UAVs and robots using mavlink
*/

#include "ch.h"
#include "hal.h"

#include "usb_cdc.h"

#include "Comms.h"

#include <mavlink.h>

/*===========================================================================*/
/* USB related stuff.                                                        */
/*===========================================================================*/

/*
 * USB Driver structure.
 */
static SerialUSBDriver SDU1;

/*
 * USB Device Descriptor.
 */
static const uint8_t vcom_device_descriptor_data[18] = {
  USB_DESC_DEVICE       (0x0110,        /* bcdUSB (1.1).                    */
                         0x02,          /* bDeviceClass (CDC).              */
                         0x00,          /* bDeviceSubClass.                 */
                         0x00,          /* bDeviceProtocol.                 */
                         0x40,          /* bMaxPacketSize.                  */
                         0x0483,        /* idVendor (ST).                   */
                         0x5740,        /* idProduct.                       */
                         0x0200,        /* bcdDevice.                       */
                         1,             /* iManufacturer.                   */
                         2,             /* iProduct.                        */
                         3,             /* iSerialNumber.                   */
                         1)             /* bNumConfigurations.              */
};

/*
 * Device Descriptor wrapper.
 */
static const USBDescriptor vcom_device_descriptor = {
  sizeof vcom_device_descriptor_data,
  vcom_device_descriptor_data
};

/* Configuration Descriptor tree for a CDC.*/
static const uint8_t vcom_configuration_descriptor_data[67] = {
  /* Configuration Descriptor.*/
  USB_DESC_CONFIGURATION(67,            /* wTotalLength.                    */
                         0x02,          /* bNumInterfaces.                  */
                         0x01,          /* bConfigurationValue.             */
                         0,             /* iConfiguration.                  */
                         0xC0,          /* bmAttributes (self powered).     */
                         50),           /* bMaxPower (100mA).               */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x00,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x01,          /* bNumEndpoints.                   */
                         0x02,          /* bInterfaceClass (Communications
                                           Interface Class, CDC section
                                           4.2).                            */
                         0x02,          /* bInterfaceSubClass (Abstract
                                         Control Model, CDC section 4.3).   */
                         0x01,          /* bInterfaceProtocol (AT commands,
                                           CDC section 4.4).                */
                         0),            /* iInterface.                      */
  /* Header Functional Descriptor (CDC section 5.2.3).*/
  USB_DESC_BYTE         (5),            /* bLength.                         */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x00),         /* bDescriptorSubtype (Header
                                           Functional Descriptor.           */
  USB_DESC_BCD          (0x0110),       /* bcdCDC.                          */
  /* Call Management Functional Descriptor. */
  USB_DESC_BYTE         (5),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x01),         /* bDescriptorSubtype (Call Management
                                           Functional Descriptor).          */
  USB_DESC_BYTE         (0x00),         /* bmCapabilities (D0+D1).          */
  USB_DESC_BYTE         (0x01),         /* bDataInterface.                  */
  /* ACM Functional Descriptor.*/
  USB_DESC_BYTE         (4),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x02),         /* bDescriptorSubtype (Abstract
                                           Control Management Descriptor).  */
  USB_DESC_BYTE         (0x02),         /* bmCapabilities.                  */
  /* Union Functional Descriptor.*/
  USB_DESC_BYTE         (5),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x06),         /* bDescriptorSubtype (Union
                                           Functional Descriptor).          */
  USB_DESC_BYTE         (0x00),         /* bMasterInterface (Communication
                                           Class Interface).                */
  USB_DESC_BYTE         (0x01),         /* bSlaveInterface0 (Data Class
                                           Interface).                      */
  /* Endpoint 2 Descriptor.*/
  USB_DESC_ENDPOINT     (USB_CDC_INTERRUPT_REQUEST_EP|0x80,
                         0x03,          /* bmAttributes (Interrupt).        */
                         0x0008,        /* wMaxPacketSize.                  */
                         0xFF),         /* bInterval.                       */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x01,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x02,          /* bNumEndpoints.                   */
                         0x0A,          /* bInterfaceClass (Data Class
                                           Interface, CDC section 4.5).     */
                         0x00,          /* bInterfaceSubClass (CDC section
                                           4.6).                            */
                         0x00,          /* bInterfaceProtocol (CDC section
                                           4.7).                            */
                         0x00),         /* iInterface.                      */
  /* Endpoint 3 Descriptor.*/
  USB_DESC_ENDPOINT     (USB_CDC_DATA_AVAILABLE_EP,     /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         0x0040,        /* wMaxPacketSize.                  */
                         0x00),         /* bInterval.                       */
  /* Endpoint 1 Descriptor.*/
  USB_DESC_ENDPOINT     (USB_CDC_DATA_REQUEST_EP|0x80,  /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         0x0040,        /* wMaxPacketSize.                  */
                         0x00)          /* bInterval.                       */
};

/*
 * Configuration Descriptor wrapper.
 */
static const USBDescriptor vcom_configuration_descriptor = {
  sizeof vcom_configuration_descriptor_data,
  vcom_configuration_descriptor_data
};

/*
 * U.S. English language identifier.
 */
static const uint8_t vcom_string0[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t vcom_string1[] = {
  USB_DESC_BYTE(38),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0,
  'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0,
  'c', 0, 's', 0
};

/*
 * Device Description string.
 */
static const uint8_t vcom_string2[] = {
  USB_DESC_BYTE(56),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'C', 0, 'h', 0, 'i', 0, 'b', 0, 'i', 0, 'O', 0, 'S', 0, '/', 0,
  'R', 0, 'T', 0, ' ', 0, 'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0,
  'a', 0, 'l', 0, ' ', 0, 'C', 0, 'O', 0, 'M', 0, ' ', 0, 'P', 0,
  'o', 0, 'r', 0, 't', 0
};

/*
 * Serial Number string.
 */
static const uint8_t vcom_string3[] = {
  USB_DESC_BYTE(8),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '0' + CH_KERNEL_MAJOR, 0,
  '0' + CH_KERNEL_MINOR, 0,
  '0' + CH_KERNEL_PATCH, 0
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor vcom_strings[] = {
  {sizeof vcom_string0, vcom_string0},
  {sizeof vcom_string1, vcom_string1},
  {sizeof vcom_string2, vcom_string2},
  {sizeof vcom_string3, vcom_string3}
};

/*
 * Handles the GET_DESCRIPTOR callback. All required descriptors must be
 * handled here.
 */
static const USBDescriptor *get_descriptor(USBDriver *usbp,
                                           uint8_t dtype,
                                           uint8_t dindex,
                                           uint16_t lang) {

  (void)usbp;
  (void)lang;
  switch (dtype) {
  case USB_DESCRIPTOR_DEVICE:
    return &vcom_device_descriptor;
  case USB_DESCRIPTOR_CONFIGURATION:
    return &vcom_configuration_descriptor;
  case USB_DESCRIPTOR_STRING:
    if (dindex < 4)
      return &vcom_strings[dindex];
  }
  return NULL;
}

/**
 * @brief   IN EP1 state.
 */
static USBInEndpointState ep1instate;

/**
 * @brief   EP1 initialization structure (IN only).
 */
static const USBEndpointConfig ep1config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  sduDataTransmitted,
  NULL,
  0x0040,
  0x0000,
  &ep1instate,
  NULL,
  NULL
};

/**
 * @brief   OUT EP2 state.
 */
USBOutEndpointState ep2outstate;

/**
 * @brief   EP2 initialization structure (IN only).
 */
static const USBEndpointConfig ep2config = {
  USB_EP_MODE_TYPE_INTR,
  NULL,
  sduInterruptTransmitted,
  NULL,
  0x0010,
  0x0000,
  NULL,
  &ep2outstate,
  NULL
};

/**
 * @brief   OUT EP2 state.
 */
USBOutEndpointState ep3outstate;

/**
 * @brief   EP3 initialization structure (OUT only).
 */
static const USBEndpointConfig ep3config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  NULL,
  sduDataReceived,
  0x0000,
  0x0040,
  NULL,
  &ep3outstate,
  NULL
};

/*
 * Handles the USB driver global events.
 */
static void usb_event(USBDriver *usbp, usbevent_t event) {

  switch (event) {
  case USB_EVENT_RESET:
    return;
  case USB_EVENT_ADDRESS:
    return;
  case USB_EVENT_CONFIGURED:
    chSysLockFromIsr();

    /* Enables the endpoints specified into the configuration.
       Note, this callback is invoked from an ISR so I-Class functions
       must be used.*/
    usbInitEndpointI(usbp, USB_CDC_DATA_REQUEST_EP, &ep1config);
    usbInitEndpointI(usbp, USB_CDC_INTERRUPT_REQUEST_EP, &ep2config);
    usbInitEndpointI(usbp, USB_CDC_DATA_AVAILABLE_EP, &ep3config);

    /* Resetting the state of the CDC subsystem.*/
    sduConfigureHookI(usbp);

    chSysUnlockFromIsr();
    return;
  case USB_EVENT_SUSPEND:
    return;
  case USB_EVENT_WAKEUP:
    return;
  case USB_EVENT_STALLED:
    return;
  }
  return;
}

/*
 * Serial over USB driver configuration.
 */
static const SerialUSBConfig serusbcfg = {
  &USBD1,
  {
    usb_event,
    get_descriptor,
    sduRequestsHook,
    NULL
  }
};



// Setup some mavlink vars
mavlink_system_t mavlink_system;
static mavlink_status_t comms_status;
static mavlink_message_t comms_msg_in;
static int comms_packet_drops, comms_packet_success = 0;

// Setup some timers and callbacks
static VirtualTimer vt1;
static void ledoff(void *p) {

	(void)p;
	palClearPad(GPIOD, GPIOD_LED4); // green
}

static WORKING_AREA(COMMSWA, 128);
static msg_t Comms(void *arg){

	(void)arg;
	chRegSetThreadName("Comms");
	while (TRUE){
		// Read a byte off the receiver
		uint8_t c = chnGetTimeout((BaseChannel *)&SDU1, TIME_INFINITE);

		if (mavlink_parse_char(MAVLINK_COMM_0, c, &comms_msg_in, &comms_status)){
			// Handle message

			switch (comms_msg_in.msgid){
				case MAVLINK_MSG_ID_HEARTBEAT:
					// E.g. read GCS heartbeat and go into
					// comm lost mode if timer times out
					break;
				case MAVLINK_MSG_ID_COMMAND_LONG:
					// EXECUTE ACTION
					break;
				default:
					//Do nothing
					break;
			}

			// Update global packet drops counter
			comms_packet_drops += comms_status.packet_rx_drop_count;
			comms_packet_success += comms_status.packet_rx_success_count;
		}
	}
	return 0;
}

/*
 * Activates the serial driver 2
 * PA2(TX) and PA3(RX) are routed to USART2.
 */

void CommsInit(void){

	mavlink_system.sysid = 1;
	mavlink_system.compid = MAV_COMP_ID_IMU;
	mavlink_system.type = MAV_TYPE_QUADROTOR;
	mavlink_system.mode = MAV_MODE_PREFLIGHT;
	mavlink_system.state = MAV_STATE_BOOT;
	mavlink_system.nav_mode = MAV_AUTOPILOT_INVALID;

	/*
	 * Activates the USB driver and then the USB bus pull-up on D+.
	 */
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1000);
	sduObjectInit(&SDU1);
	sduStart(&SDU1, &serusbcfg);
	usbConnectBus(serusbcfg.usbp);

	CommsSendHeartbeat();
	chThdCreateStatic(COMMSWA, sizeof(COMMSWA), NORMALPRIO, Comms, NULL);
}

/*
 * Sends a heartbeat message out, so everyone knows we're still alive
 */

void CommsSendHeartbeat(void){
	palSetPad(GPIOD, GPIOD_LED4); // green
	mavlink_msg_heartbeat_send(MAVLINK_COMM_0, mavlink_system.type, mavlink_system.nav_mode,  mavlink_system.mode, 0, mavlink_system.state);

	chSysLock();
	if (chVTIsArmedI(&vt1))
		chVTResetI(&vt1);
	chVTSetI(&vt1, MS2ST(500), ledoff, NULL);
	chSysUnlock();
}

/*
 * Sends a system status message
 */

void CommsSendSysStatus(void){
	// Indices: 
	// 0: 3D gyro
	// 1: 3D acc
	// 2: 3D mag
	// 3: absolute pressure
	// 4: differential pressure
	// 5: GPS
	// 6: optical flow
	// 7: computer vision position
	// 8: laser based position
	// 9: external ground-truth (Vicon or Leica)
	// Controls:
	// 10: 3D angular rate control
	// 11: attitude stabilization
	// 12: yaw position control
	// 13: z/altitude control
	// 14: x/y position control
	// 15: motor outputs / control
	uint32_t sensors_present = 0xFC23;
	uint32_t sensors_enabled = sensors_present;

	mavlink_msg_sys_status_send(MAVLINK_COMM_0, sensors_present, sensors_enabled, sensors_enabled, 0, 11100, -1, -1, comms_packet_drops/comms_packet_success*10000, comms_packet_drops, 0, 0, 0, 0);
}

void CommsSendAttitude(uint32_t time_boot_ms, float roll, float pitch, float yaw, float rollspeed, float pitchspeed, float yawspeed){
	mavlink_msg_attitude_send(MAVLINK_COMM_0, time_boot_ms, roll, pitch, yaw, rollspeed, pitchspeed, yawspeed);
}


/**
 * @brief Send buffer over a comm channel
 *
 * @param chan MAVLink channel to use, usually MAVLINK_COMM_0 = UART0
 * @param buf Data to send
 * @param len Length of data
 */
inline void comms_send_bytes(mavlink_channel_t chan, const uint8_t *buf, uint16_t len){
	if (chan == MAVLINK_COMM_0){
		chnWriteTimeout(&SDU1, buf, len, TIME_INFINITE);
	}
}