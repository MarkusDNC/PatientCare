//#include "stdafx.h"
#include "ComFtdiFpc1011.h"
#include "WinTypes.h"
#include <unistd.h>
#include <stdio.h>

struct ftdi_context *ft_con_patient;
struct ftdi_context *ft_con_doctor;

/* open FTDI device and store handle in global scope, setup initial pin states of
   sensor interface */
int openDevice(void) {

	/* try to open the FTDI usb device */
	struct ftdi_device_list *devs;
	ft_con_patient = ftdi_new();
	ft_con_doctor = ftdi_new();

	//int status = ftdi_usb_find_all(ft_con, &devs, VENDOR_ID, PRODUCT_ID);

	int status = 0;
	int status1 = ftdi_usb_open( ft_con_patient, VENDOR_ID, PRODUCT_ID );
	int status2 = ftdi_usb_open( ft_con_doctor , VENDOR_ID, PRODUCT_ID_2 );

/*
	printf("FPC_1: %i\n", status1);
	printf("FPC_2: %i\n", status2);
	if( status1 < 0 ){
		printf("Could not find patient fingerprint readers.\n");
		return -1;
	}
	if( status2 < 0 ){
				printf("Could not find doctor fingerprint readers.\n");
		return -1;
	}

	printf("Found devices\n");
	*/
	
	//status = ftdi_usb_open_dev(ft_con, devs->dev);
	printf("Status 1: %i\n", status1);
	printf("Status 2: %i\n", status2);
	if( status1 < 0 ){
		printf("Could not open patient device.\n");
		return -1;
	}

	else if( status2 < 0 ){
		printf("Could not open doctor device.\n");
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

	status = ftdi_usb_reset( ft_con_doctor );

	if( status < 0 ){
		printf("Could not reset doctor device.\n");
		return -1;
	}

	else{
		printf("Doctor device reset!\n");
	}

	status = ftdi_usb_reset( ft_con_patient );

	if( status < 0 ){
		printf("Could not reset patient device.\n");
		return -1;
	}

	else{
		printf("Patient device reset!\n");
	}

	status = ftdi_usb_purge_rx_buffer( ft_con_doctor );

	if( status < 0) {
		printf("Purge of doctor device failed!\n");	
	}
	else{
		printf("Purge doctor device success!\n");
	}

	status = ftdi_usb_purge_rx_buffer( ft_con_patient );

	if( status < 0) {
		printf("Purge of patient device failed!\n");	
	}
	else{
		printf("Purge patient device success!\n");
	}

	
	status = ftdi_set_event_char( ft_con_doctor, false, 0 );
	if( status < 0 ){
		printf( "Event character set failed! (Doctor)\n" );
	}
	else{
		printf( "Event character successfully disabled. (Doctor)\n" ); 
	}

	status = ftdi_set_event_char( ft_con_patient, false, 0 );
	if( status < 0 ){
		printf( "Event character set failed! (Patient)\n" );
	}
	else{
		printf( "Event character successfully disabled. (Patient)\n" ); 
	}

	status = ftdi_set_error_char( ft_con_doctor, false, 0 );
	if( status < 0 ){
		printf( "Error character set failed! (Doctor)\n" );
	}
	else{
		printf( "Error character successfully disabled. (Doctor).\n" );
	}

	status = ftdi_set_error_char( ft_con_patient, false, 0 );
	if( status < 0 ){
		printf( "Error character set failed! (Patient)\n" );
	}
	else{
		printf( "Error character successfully disabled. (Patient)\n" );
	}

	status = ftdi_set_latency_timer( ft_con_doctor, 32 );
	if ( status < 0 ){
		printf( "Unable to set latency timer! (Doctor)\n" );
	}	
	else{
		printf( "Latency timer set. (Doctor)\n" );
	}

	status = ftdi_set_latency_timer( ft_con_patient, 32 );
	if ( status < 0 ){
		printf( "Unable to set latency timer! (Patient)\n" );
	}	
	else{
		printf( "Latency timer set. (Patient)\n" );
	}

	status = ftdi_set_bitmode ( ft_con_doctor, 0x00, BITMODE_MPSSE );
	if( status < 0 ){
		printf( "Bitmode set failed! (Doctor)\n" );
	}	
	else{
		printf( "Bitmode successfully set. (Doctor)\n" );
	}

	status = ftdi_set_bitmode ( ft_con_patient, 0x00, BITMODE_MPSSE );
	if( status < 0 ){
		printf( "Bitmode set failed! (Patient)\n" );
	}	
	else{
		printf( "Bitmode successfully set. (Patient)\n" );
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
	
	if(writeFTDI(msg, mp, ft_con_doctor))
		return -1;

	if(writeFTDI(msg, mp, ft_con_patient))
		return -1;
	
	printf( "Open complete!\n" );
	return 0;
}

int closeDevice( void ){
	if( ft_con_doctor != NULL ){
		int status = ftdi_usb_close( ft_con_doctor );
		if ( status < 0 ){
			printf( "Closing doctor device failed!" );
		}
		else{
			printf( "Doctor device successfully closed" );
		}
	}

	if( ft_con_patient != NULL ){
		int status = ftdi_usb_close( ft_con_patient );
		if ( status < 0 ){
			printf( "Closing patient device failed!" );
		}
		else{
			printf( "Patient device successfully closed" );
		}
	}
	return 0;
}

/* write command/data to FTDI device */
int writeFTDI(unsigned char* data, int length, struct ftdi_context *ft_con) {
	if( ft_con == NULL)
		return -1;

	int written = ftdi_write_data( ft_con, data, length );		

	if( written < 0 )
		return -1;

	if(written != length)
		return -1;

	return 0;
}

int resetSensor( void ) {
	unsigned char cmd[3];

	cmd[0] = FPC_SET_HIGH_PINS;
	cmd[1] = 0; // set reset low
	cmd[2] = FPC_ACBUS6_RESET;
	
	if(writeFTDI(cmd, 3, ft_con_patient)){
	    printf("Failed to reset patient sensor! (1)");
		return -1;
	}
	
	usleep(50*1000);
	
	cmd[1] = FPC_ACBUS6_RESET; // set reset high
	if(writeFTDI(cmd, 3, ft_con_patient)){
	    printf("Failed to reset patient sensor! (2)");
		return -1;
	}

	//Reset of doctor sensor fails, returns 0 to avoid.
	printf("Reset of doctor sensor fails, returns 0 before to avoid.\n");
	return 0;


	cmd[0] = FPC_SET_HIGH_PINS;
	cmd[1] = 0; // set reset low
	cmd[2] = FPC_ACBUS6_RESET;
	
	if(writeFTDI(cmd, 3, ft_con_doctor)){
	    printf("Failed to reset doctor sensor! (1)");
		return -1;
	}

	usleep(50*1000);
	
	cmd[1] = FPC_ACBUS6_RESET; // set reset high
	if(writeFTDI(cmd, 3, ft_con_doctor)){
	    printf("Failed to reset doctor sensor! (2)");
		return -1;
	}

	usleep(50*1000);
    printf("Successfully reset sensor\n");
	return 0;
}

/* read back data from FTDI device*/
int readFTDI(unsigned char* data, int length, struct ftdi_context *ft_con) {

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

int captureImage(bool ispatient, unsigned char adcref, unsigned char drivc, unsigned char sensemode, unsigned char* pdata) {
	
	struct ftdi_context *ft_con = ispatient ? ft_con_patient : ft_con_doctor;

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

	if(writeFTDI(cmd, cmdBuilder.getSize(), ft_con))
		return -1;

	if(readFTDI(pdata, 30400, ft_con))
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
