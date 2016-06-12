//#include "stdafx.h"
#include "ComFtdiFpc1011.h"
#include "WinTypes.h"
#include <unistd.h>
#include <stdio.h>

struct ftdi_context *ft_con;

/* open FTDI device and store handle in global scope, setup initial pin states of
   sensor interface */
int openDevice(void) {

	/* try to open the FTDI usb device */
	struct ftdi_device_list *devs;
	ft_con = ftdi_new();
	int status = ftdi_usb_find_all(ft_con, &devs, VENDOR_ID, PRODUCT_ID);
	printf("Status: %i\n", status);
	if( status < 1 ){
		printf("No devices\n");
		return -1;
	}

	printf("Found devices\n");
	
	status = ftdi_usb_open_dev(ft_con, devs->dev);
	printf("Status: %i\n", status);
	if( status < 0 ){
		printf("Could not open device\n");
		return -1;
	}

	else{
		printf("Success\n");
	}

	//Instead of DWORD bytesInBuffer = 0;
	
	
	/*if(FT_Open(0, &m_devHandle) != FT_OK) {
		m_devHandle = NULL;
		return -1;
	}*/

	status = ftdi_usb_reset( ft_con );

	if( status < 0 ){
		printf("Could not reset device\n");
		return -1;
	}

	else{
		printf("Device reset!\n");
	}

	status = ftdi_usb_purge_rx_buffer( ft_con );

	if( status < 0) {
		printf("Purge failed!\n");	
	}
	else{
		printf("Purge success!\n");
	}

	
	status = ftdi_set_event_char( ft_con, false, 0 );
	if( status < 0 ){
		printf( "Event character set failed!\n" );
	}
	else{
		printf( "Event character successfully disabled.\n" ); 
	}

	status = ftdi_set_error_char( ft_con, false, 0 );
	if( status < 0 ){
		printf( "Error character set failed!\n" );
	}
	else{
		printf( "Error character successfully disabled.\n" );
	}

	status = ftdi_set_latency_timer( ft_con, 32 );
	if ( status < 0 ){
		printf( "Unable to set latency timer!\n" );
	}	
	else{
		printf( "Latency timer set.\n" );
	}

	status = ftdi_set_bitmode ( ft_con, 0x00, BITMODE_MPSSE );
	if( status < 0 ){
		printf( "Bitmode set failed!\n" );
	}	
	else{
		printf( "Bitmode successfully set.\n" );
	}
	
	usleep(50*1000); //Sleep for 50 msek
	
	/* device succesfuly opened, setup pins for communication with FPC1011F */
	unsigned char value, direction;
	value = 0;

	unsigned char msg[50];
	int mp = 0;

	msg[mp++] = FPC_DISABLE_ADATPIVE_CLOCKING;
	msg[mp++] = FPC_DISABLE_LOOPBACK;
	msg[mp++] = FPC_DISABLE_CLOCKDIV_5;
	msg[mp++] = FPC_DISABLE_3_PHASE;
	
	msg[mp++] = FPC_SET_CLOCK_DIV;
	msg[mp++] = 3;
	msg[mp++] = 0;
	
	/* of the lower pins MOSI, CLK and CS are output pins, CS is active low so we set it high here */
	direction = FPC_ADBUS1_MOSI + FPC_ADBUS0_CLK + FPC_ADBUS3_CS;
	value = FPC_ADBUS3_CS;

	msg[mp++] = FPC_SET_LOW_PINS;
	msg[mp++] = value;
	msg[mp++] = direction;
	
	/* reset signal is connected to higher pins and is active low, so set it high here */
	msg[mp++] = FPC_SET_HIGH_PINS;
	msg[mp++] = FPC_ACBUS6_RESET;
	msg[mp++] = FPC_ACBUS6_RESET;
	
	if(writeFTDI(msg, mp))
		return -1;
	
	printf( "Open complete!\n" );
	return 0;
}

int closeDevice( void ){
	if( ft_con != NULL ){
		int status = ftdi_usb_close( ft_con );
		if ( status < 0 ){
			printf( "Close failed!" );
		}
		else{
			printf( "Device successfully closed" );
		}
	}
	return 0;
}

/* write command/data to FTDI device */
int writeFTDI(unsigned char* data, int length) {
	if( ft_con == NULL)
		return -1;

	int written = ftdi_write_data( ft_con, data, length );		

	if( written < 0 )
		return -1;

	if(written != length)
		return -1;

	return 0;
}

int resetSensor() {
	unsigned char cmd[3];

	cmd[0] = FPC_SET_HIGH_PINS;
	cmd[1] = 0; // set reset low
	cmd[2] = FPC_ACBUS6_RESET;
	
	if(writeFTDI(cmd, 3)){
	    printf("Failed to reset sensor!");
		return -1;
	}
	
	usleep(50*1000);
	
	cmd[1] = FPC_ACBUS6_RESET; // set reset high
	if(writeFTDI(cmd, 3)){
	    printf("Failed to reset sensor!");
		return -1;
	}

	usleep(50*1000);
    printf("Successfully reset sensor\n");
	return 0;
}

/* read back data from FTDI device*/
int readFTDI(unsigned char* data, int length) {
	if( ft_con == NULL){
	    printf("Failed to read data from device");
		return -1;
	}

	int bytesread = 0;
	int read_status = 0;

	while(bytesread != length) {
			read_status = ftdi_read_data( ft_con, data + bytesread, length - bytesread);
			if ( read_status < 0 ){
			    printf("Failed to read from device!\n");
				return -1;
			}
            printf( "Successfully read from device" );
			bytesread += read_status;
	}
	return 0;
}

int captureImage(unsigned char adcref, unsigned char drivc, unsigned char sensemode, unsigned char* pdata) {
	
	if( ft_con == NULL)
		return -1;

	unsigned char cmd[100];
	FpcMpsseCommandBuilder cmdBuilder(cmd);

	cmdBuilder.addSetSS(0); //set chip select active
	cmdBuilder.addSend(60); //clock out 60 bytes

	/* write image settings and capture image command */
	   
	cmdBuilder.addByte(FPC_WR_DRIVC);
	cmdBuilder.addByte(drivc);
	cmdBuilder.addByte(FPC_WR_ADCREF);
	cmdBuilder.addByte(adcref);
	cmdBuilder.addByte(FPC_WR_SENSEM);
	cmdBuilder.addByte(sensemode);
	cmdBuilder.addByte(FPC_RD_SENSOR);
	cmdBuilder.addByte(0x00);
	
	/* queue 50 dummy bytes to ensure that image is available for readback */
	for(int a = 0; a <50; a++) {
		cmdBuilder.addByte(0x00);
	}

	/* queue read image data command and read 30400 bytes on SPI interface */
	cmdBuilder.addByte(FPC_RD_SPIDATA);
	cmdBuilder.addByte(0x00);
	cmdBuilder.addRead(30400);
	cmdBuilder.addSetSS(1);
	cmdBuilder.addSendImmmediate();

	if(writeFTDI(cmd, cmdBuilder.getSize()))
		return -1;

	if(readFTDI(pdata, 30400))
		return -1;

	return 0;

}

/* FpcMpsseCommandBuilder class for filling an array with FTDI MPSSE commands */

FpcMpsseCommandBuilder::FpcMpsseCommandBuilder(unsigned char* commandArray) {
	init(commandArray);		
}

void FpcMpsseCommandBuilder::init(unsigned char* commandArray) {
	m_command = commandArray;
	m_index = 0;
}


/* add MPSSE command for setting ADBUS3 (sensor CS)  high/low */
void FpcMpsseCommandBuilder::addSetSS(bool high) {
	m_command[m_index++] = FPC_SET_LOW_PINS;
	m_command[m_index++] = high? FPC_ADBUS3_CS:0;
	m_command[m_index++] = FPC_ADBUS0_CLK + FPC_ADBUS3_CS + FPC_ADBUS1_MOSI;
}


/* add MPSSE command for clocking data out according to SPI mode 0 MSB first,
   length bytes of data must follow*/
void FpcMpsseCommandBuilder::addSend(int length) {
	m_command[m_index++] = FPC_OUT_NEG;
	m_command[m_index++] = FPC_TRANSFER_SIZE_L(length);
	m_command[m_index++] = FPC_TRANSFER_SIZE_H(length);
}

/* add MPSSE comfmand for sampling data according to SPI mode 0  MSB first,
   length bytes of data will be available for readback*/
void FpcMpsseCommandBuilder::addRead(int length) {
	m_command[m_index++] = FPC_IN_POS;
	m_command[m_index++] = FPC_TRANSFER_SIZE_L(length);
	m_command[m_index++] = FPC_TRANSFER_SIZE_H(length);
}

/* add a databyte to command array */
void FpcMpsseCommandBuilder::addByte(unsigned char byte) {
	m_command[m_index++] = byte;
}


void FpcMpsseCommandBuilder::addSendImmmediate() {
	m_command[m_index++] = FPC_SEND_IMMEDIATE;
}

int FpcMpsseCommandBuilder::getSize(void) {
	return m_index;
}
