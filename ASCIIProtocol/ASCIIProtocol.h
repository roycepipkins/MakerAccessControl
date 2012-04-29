//Copyright Royce Pipkins 2010
//May be used under the terms of the GPL V3 or higher. http://www.gnu.org/licenses/gpl.html
/*
 * ASCIIProtocol.h
 *
 *  Created on: Mar 28, 2010
 *      Author: Royce Pipkins
 *
 *  ASCII protocol driver
 *  Address and Type will be encoded as ascii-hex in the
 *  first four bytes after STX. The body string will follow
 *  as-is and will include the zero terminator. The next
 *  four bytes will be the 16 bit check sum of the everything
 *  past the STX up to the checksum expressed as ascii-hex
 *
 *  [STX][ADDRESS][TYPE][BODYSTR][\0][CHECKSUM][ETX]
 *
 *  The body string mustn't contain an STX or ETX or the receiver
 *  will reject the packet.
 */

#ifndef ASCIIPROTOCOL_H_
#define ASCIIPROTOCOL_H_

#include <stdint.h>
#include <string.h>

#ifdef __AVR__
#include "Print.h"
#else
#include <QIODevice>
#include <QtDebug>
class Print
{
    public:
    Print(QIODevice& qio) : io(qio){}
    void write(const uint8_t *buffer, size_t size)
    { 
        if (-1 == io.write((char*)buffer, size))
        {
            qDebug() << "Failed to write";
            io.close();
        }
    }
    private:
        QIODevice& io;
};
#endif






class ASCIIProtocol 
{

public:
	const static int STX = 2;
	const static int ETX = 3;
	const static int bufSize = 64;

	ASCIIProtocol(Print& transmitter);
	void send(uint8_t address, uint8_t type, const unsigned char* bodyStr);
	bool recv(unsigned char data);
	bool isValidPacket();
	uint8_t getAddress();
	uint8_t getType();
	uint8_t* getBody();
	uint16_t getChecksum();
	void erasePkt();

protected:
	uint8_t pktBuf[bufSize];
	uint8_t pktTail;
	Print& xmit;
	bool etx_recved;
	static void byteToASCII(uint8_t val, uint8_t* buf);
	static void wordToASCII(uint16_t val, uint8_t* buf);
	static uint8_t ASCIIToByte(const uint8_t* buf);
	static uint16_t ASCIIToWord(const uint8_t* buf);
	

};

#endif /* ASCIIPROTOCOL_H_ */
