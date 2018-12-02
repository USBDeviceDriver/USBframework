//  main code starter
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "usbapi.h"
#include "usbdebug.h"
#include "usbstruct.h"
#include "rtc.h"

#include "LPC17xx.h"

#define usbMAX_SEND_BLOCK		( 20 / portTICK_PERIOD_MS )
#define usbBUFFER_LEN			( 20 )

#define INCREMENT_ECHO_BY 1
#define BAUD_RATE	115200

#define INT_IN_EP		0x81
#define BULK_OUT_EP		0x05
#define BULK_IN_EP		0x82

#define MAX_PACKET_SIZE	64

#define LE_WORD(x)		((x)&0xFF),((x)>>8)

static unsigned char abBulkBuf[64];
//static unsigned char abClassReqData[8];


// forward declaration of interrupt handler
void USBIntHandler(void);

#define BULK_IN_1_EP		0x82	//refer pg.no.110 of Axelson, Field: bEndpointAddress
#define BULK_IN_2_EP		0x85
#define BULK_OUT_1_EP		0x08


static const unsigned char abDescriptors[] = {

/* Device descriptor */
	0x12,					// Length of descriptor
	DESC_DEVICE,       		// Type of descriptor
	LE_WORD(0x0200),		// bcdUSB
	0x00,              		// bDeviceClass
	0x00,              		// bDeviceSubClass
	0x00,              		// bDeviceProtocol
	MAX_PACKET_SIZE0,  		// bMaxPacketSize
	LE_WORD(0xFFFF),		// idVendor
	LE_WORD(0x0004),		// idProduct
	LE_WORD(0x0100),		// bcdDevice
	0x01,              		// iManufacturer
	0x02,              		// iProduct
	0x03,              		// iSerialNumber
	0x01,              		// bNumConfigurations

// Configuration Descriptor
	0x09,
	DESC_CONFIGURATION,
	LE_WORD(0x27),  		// wTotalLength
	0x01,  					// bNumInterfaces
	0x01,  					// bConfigurationValue
	0x00,  					// iConfiguration
	0x80,  					// bmAttributes		//bus powered
	0x32,  					// bMaxPower	(100mA Current)

// Interface Descriptor
	0x09,
	DESC_INTERFACE,
	0x00,  		 			// bInterfaceNumber
	0x00,   				// bAlternateSetting
	0x03,   				// bNumEndPoints
	0xFF,   				// bInterfaceClass
	0x00,   				// bInterfaceSubClass
	0x00,   				// bInterfaceProtocol
	0x00,   				// iInterface

// Bulk IN 1 Endpoint
	0x07,
	DESC_ENDPOINT,
	BULK_IN_1_EP,					// bEndpointAddress
	0x02,   					// bmAttributes = Bulk
	LE_WORD(MAX_PACKET_SIZE),	// wMaxPacketSize
	0x00,						// bInterval

// Bulk IN 2 Endpoint
	0x07,
	DESC_ENDPOINT,
	BULK_IN_2_EP,				// bEndpointAddress
	0x02,   					// bmAttributes = Bulk
	LE_WORD(MAX_PACKET_SIZE),	// wMaxPacketSize
	0x00,						// bInterval

// Bulk OUT 1 Endpoint
	0x07,
	DESC_ENDPOINT,
	BULK_OUT_1_EP,				// bEndpointAddress
	0x02,   					// bmAttributes = Bulk
	LE_WORD(MAX_PACKET_SIZE),	// wMaxPacketSize
	0x00,						// bInterval

// string descriptors
	0x04,
	DESC_STRING,
	LE_WORD(0x0409),

// manufacturer string
	0x14,
	DESC_STRING,
	'P', 0, 'A', 0, 'S', 0, 'S' , 0, ' ', 0, 'L', 0, 'T', 0, 'D', 0, '.', 0,

// product string
	0x10,
	DESC_STRING,
	'U', 0, 'S', 0, 'B', 0, ' ', 0, 'A', 0, 'R', 0, 'M', 0,


// serial number string
	0x12,
	DESC_STRING,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '1', 0,

// terminator
	0
};

/**
	Local function to handle incoming bulk data

	@param [in] bEP
	@param [in] bEPStatus
 */
static void BulkOut(unsigned char bEP, unsigned char bEPStatus)
{
	lcd_puts(LCD_LINE1, "SUN INFO");
	int iLen;
	//long lHigherPriorityTaskWoken = pdFALSE;

	( void ) bEPStatus;

	// get data from USB into intermediate buffer
	iLen = USBHwEPRead(bEP, abBulkBuf, sizeof(abBulkBuf));
	lcd_puts(LCD_LINE2, abBulkBuf);
}


/**
	Local function to handle outgoing bulk data

	@param [in] bEP
	@param [in] bEPStatus
 */
static void BulkIn(unsigned char bEP, unsigned char bEPStatus)
{
	rtc_time_t tm;
	//lcd_puts(LCD_LINE2, "SUNBEAM INFOTECH");

	//long lHigherPriorityTaskWoken = pdFALSE;

	( void ) bEPStatus;
	rtc_get(&tm);

	sprintf(abBulkBuf, "%02d:%02d:%02d", tm.hour, tm.min, tm.sec);
	lcd_puts(LCD_LINE1, abBulkBuf);

	USBHwEPWrite(bEP, abBulkBuf, 64);

	sprintf(abBulkBuf, "%02d/%02d/%04d", tm.dom, tm.mon, tm.year);
	lcd_puts(LCD_LINE2, abBulkBuf);

	// send over USB

	USBHwEPWrite(bEP, abBulkBuf, 64);

}

//void USBIntHandler(void)
void USB_IRQHandler(void)
{
	USBHwISR();
}

int main(void)
{
	rtc_time_t tm = { .sec = 0, .min = 0, .hour = 0, .dom = 1, .mon = 1, .year = 2015, .dow = 4, .doy = 1};
	SystemInit();
	// initialise stack
	USBInit();

	// register descriptors
	USBRegisterDescriptors(abDescriptors);


	// register endpoint handlers
//	USBHwRegisterEPIntHandler(INT_IN_EP, NULL);
	USBHwRegisterEPIntHandler(BULK_IN_1_EP, BulkIn);
	USBHwRegisterEPIntHandler(BULK_OUT_1_EP, BulkOut);

	// enable bulk-in interrupts on NAKs
	USBHwNakIntEnable(INACK_BI);

	DBG("Starting USB communication\n");
	lcd_init();
	rtc_init(&tm);
	lcd_puts(LCD_LINE1, "HELLO ");
	//NVIC_SetPriority( USB_IRQn, configUSB_INTERRUPT_PRIORITY );
	NVIC_EnableIRQ( USB_IRQn );

	// connect to bus

	DBG("Connecting to USB bus\n");
	USBHwConnect(TRUE);

	// echo any character received (do USB stuff in interrupt)
	for( ;; );
}
