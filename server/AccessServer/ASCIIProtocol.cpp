//Copyright Royce Pipkins 2010
//May be used under the terms of the GPL V3 or higher. http://www.gnu.org/licenses/gpl.html
/*
 * ASCIIProtocol.cpp
 *
 *  Created on: Mar 28, 2010
 *      Author: Royce Pipkins
 */

#include <stdlib.h>
#include "ASCIIProtocol.h"




ASCIIProtocol::ASCIIProtocol(Print& transmitter) :
	xmit(transmitter)
{
	pktTail = 0;
}



void ASCIIProtocol::byteToASCII(uint8_t val, uint8_t* buf)
{
	uint8_t hex[17] = "0123456789ABCDEF";
	buf[0] = hex[val >> 4];
	buf[1] = hex[val & 0x0f];
}

void ASCIIProtocol::wordToASCII(uint16_t val, uint8_t* buf)
{
	uint8_t hex[17] = "0123456789ABCDEF";
	buf[0] = hex[val >> 12];
	buf[1] = hex[(val >> 8) & 0x0f];
	buf[2] = hex[(val >> 4) & 0x0f];
	buf[3] = hex[val & 0x0f];
}

uint8_t ASCIIProtocol::ASCIIToByte(const uint8_t* buf)
{
	uint8_t val_accum = 0;
	uint8_t raw_val = 0;
	uint8_t idx;
	for(idx = 0; idx < 2; idx++)
	{
		if (buf[idx] < 58)
		{
			raw_val = buf[idx] - 48;
		}
		else
		{
			raw_val = (buf[idx] | 32) - 87;
		}

		val_accum |= (raw_val << 4*(1-idx));
	}

	return(val_accum);
}

uint16_t ASCIIProtocol::ASCIIToWord(const uint8_t* buf)
{
	uint16_t val_accum = 0;
	uint16_t raw_val = 0;
	uint8_t idx;
	for(idx = 0; idx < 4; idx++)
	{
		if (buf[idx] < 58)
		{
			raw_val = buf[idx] - 48;
		}
		else
		{
			raw_val = (buf[idx] | 32) - 87;
		}

		val_accum |= (raw_val << 4*(3-idx));
	}

	return(val_accum);
}


void ASCIIProtocol::send(uint8_t address, uint8_t type, const unsigned char* bodyStr)
{
	uint8_t xmitBuf[bufSize];
	uint16_t checksum = 0;
	uint8_t len = strlen((const char*)bodyStr), idx;

	//assemble the packet
	xmitBuf[0] = STX;
	byteToASCII(address, (xmitBuf+1));
	byteToASCII(type, (xmitBuf+3));
	strncpy((char*)(xmitBuf+5), (char*)bodyStr, bufSize - 5);
	xmitBuf[5+len] = 0;

	//compute the checksum
	for(idx = 1; idx < 5 + len; idx++)
	{
		checksum += xmitBuf[idx];
	}

	//finalize the packet
	wordToASCII(checksum, (xmitBuf + 6 + len));
	xmitBuf[10 + len] = ETX;

	//transmit the packet
	xmit.write((uint8_t*)xmitBuf, 11 + len);

}


bool ASCIIProtocol::recv(unsigned char data)
{
	if (data == STX)
	{
		pktBuf[0] = STX;
		pktTail = 1;
	}
	else if (pktTail > 0 && pktTail < bufSize)
	{
		pktBuf[pktTail] = data;
		pktTail++;
	}
	if (data == ETX) etx_recved = true;
	return (data == ETX);
}



bool ASCIIProtocol::isValidPacket()
{
	uint8_t idx, etx_idx = 0;
	uint16_t checksum, pktChecksum;
	if (pktBuf[0] == STX)
	{
		//find end of packet
		for(idx = 1; idx < bufSize; idx++)
		{
			if (pktBuf[idx] == ETX)
			{
				etx_idx = idx;
				break;
			}
		}
		//make sure packet is at least the min length
		if (etx_idx > 9) //STX + 2 Addr + 2 Type + '\0' + 4 checksum
		{
			pktChecksum = ASCIIToWord(&pktBuf[etx_idx - 4]);
			checksum = 0;
			for(idx = 1; idx < etx_idx - 4; idx++)
			{
				checksum += pktBuf[idx];
			}

			if (pktChecksum == checksum) return true;
		}
	}

	return false;
}

void ASCIIProtocol::erasePkt()
{
	int idx;
	for(idx = 0; idx < bufSize; idx++)
		pktBuf[idx] = 0;
}

uint16_t ASCIIProtocol::getChecksum()
{
		uint8_t idx, etx_idx = 0;
		//find end of packet
		for(idx = 1; idx < bufSize; idx++)
		{
			if (pktBuf[idx] == ETX)
			{
				etx_idx = idx;
				break;
			}
		}

		uint16_t checksum = 0;
		for(idx = 1; idx < etx_idx - 4; idx++)
		{
			checksum += pktBuf[idx];
		}

		return(checksum);
		
	
}

uint8_t ASCIIProtocol::getAddress()
{
	return (ASCIIToByte(pktBuf + 1));
}


uint8_t ASCIIProtocol::getType()
{
	return (ASCIIToByte(pktBuf + 3));
}


uint8_t* ASCIIProtocol::getBody()
{
	return pktBuf + 5;
}
