#pragma once

//#include "windows.h"
#include <ftdi.h>

/* these are the FTDI MPSSE commands and pins used
   for SPI communication with FPC1011F            */

#define VENDOR_ID 0x0403
#define PRODUCT_ID 0x6014
#define PRODUCT_ID_2 0x6010

#define FPC_MPSSE								0x02

#define FPC_DISABLE_CLOCKDIV_5					0x8A
#define FPC_ENABLE_CLOCKDIV_5					0x8B
#define FPC_ENABLE_3_PHASE						0x8C
#define FPC_DISABLE_3_PHASE						0x8D
#define FPC_DISABLE_ADATPIVE_CLOCKING			0x97
#define FPC_SET_LOW_PINS						0x80
#define FPC_SET_HIGH_PINS						0x82
#define FPC_SET_CLOCK_DIV						0x86
#define FPC_DISABLE_LOOPBACK					0x85
#define FPC_SEND_IMMEDIATE						0x87

/* clock data out negative clock edge, MSB first (SPI mode 0)*/
#define FPC_OUT_NEG								0x11

/* sample data on positive clock edge, MSB first (SPI mode 0)*/
#define FPC_IN_POS								0x20

/* macro for length parameter to follow MPSSE data clocking command */
#define FPC_TRANSFER_SIZE_L(x) (((x) - 1) & 0xFF)
#define FPC_TRANSFER_SIZE_H(x) ((((x) - 1) >> 8) & 0xFF)

/* pins */
#define FPC_ADBUS0_CLK						(1 << 0)
#define FPC_ADBUS1_MOSI						(1 << 1)
#define FPC_ADBUS2_MISO						(1 << 2)
#define FPC_ADBUS3_CS						(1 << 3)
#define FPC_ACBUS6_RESET					(1 << 6)


/* FPC1011 instructions */
#define FPC_RD_SENSOR		0x11
#define FPC_RD_SPIDATA		0x20
#define FPC_RD_SPISTAT		0x21
#define FPC_RD_REGS			0x50
#define FPC_WR_DRIVC		0x75
#define FPC_WR_ADCREF		0x76
#define FPC_WR_SENSEM		0x77
#define FPC_WR_FIFO_TH		0x7C
#define FPC_WR_XSENSE		0x7F
#define FPC_WR_YSENSE		0x81
#define FPC_WR_XSHIFT		0x82
#define FPC_WR_YSHIFT		0x83
#define FPC_WR_XREADS		0x84


/* functions for opening USB device and communicating with FPC1011 sensor */
extern "C" {
	int openDevice(void);
	int closeDevice(void);
	int writeFTDI(unsigned char* data, int length, struct ftdi_context *ft_con);
	int readFTDI(unsigned char* data, int length, struct ftdi_context *ft_con);
	int captureImage(bool ispatient, unsigned char adcref, unsigned char drivc, unsigned char sensemode, unsigned char* pdata);
	int readStatus(unsigned char* status);
	int resetSensor();
}

/* convenience class for filling an array with MPSSE commands/data */
class FpcMpsseCommandBuilder {
private:
	unsigned char* m_command;
	int m_index;
public:
	FpcMpsseCommandBuilder(unsigned char* commandArray);
	void init(unsigned char* commandArray);


	void addSetSS(bool high);
	void addSend(int length);
	void addRead(int length);
	void addByte(unsigned char byte);

public:
	void addSendImmmediate();
	void addWaitIrq();
	void addWriteRegister(unsigned char addr, unsigned char value);
	void addReadRegister(unsigned char addr);
	void addSendRead(unsigned char command, int dummy_count = 0, int read_count = 0);
	int getSize(void);
};
