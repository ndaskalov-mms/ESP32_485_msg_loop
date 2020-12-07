/*
 RS485 protocol library - non-blocking.

 Devised and written by Nick Gammon.
 Date: 4 December 2012
 Version: 1.0
 
 modiied by ndaskalov:
 added: support for serial logging from class functions
 added: timeout support 
 added: resetting internal states (haveSTX_, haveETX_) in case of BAD_CRC
 added: test facilities to destroy parts of the messages (STX, ETX, CRC, payload)

 Can send from 1 to 255 bytes from one node to another with:

 * Packet start indicator (STX)
 * Each data byte is doubled and inverted to check validity
 * Packet end indicator (ETX)
 * Packet CRC (checksum)


 To allow flexibility with hardware (eg. Serial, SoftwareSerial, I2C)
 you provide three "callback" functions which send or receive data. Examples are:

 size_t fWrite (const byte what)
   {
   return Serial.write (what);
   }

 int fAvailable ()
   {
   return Serial.available ();
   }

 int fRead ()
   {
   return Serial.read ();
   }


 PERMISSION TO DISTRIBUTE

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.


 LIMITATION OF LIABILITY

 The software is provided "as is", without warranty of any kind, express or implied,
 including but not limited to the warranties of merchantability, fitness for a particular
 purpose and noninfringement. In no event shall the authors or copyright holders be liable
 for any claim, damages or other liability, whether in an action of contract,
 tort or otherwise, arising from, out of or in connection with the software
 or the use or other dealings in the software.

 */


#include "Alarm_RS485.h"

#define PACKET_TIMEOUT 200  //we have to get complete packet withni XXX ms

//#define SCREW_STX
//#define SCREW_ETX
//#define SCREW_CRC
#define SCREW_DATA

// allocate the requested buffer size
void RS485::begin ()
  {
  data_ = (byte *) malloc (bufferSize_);
  reset ();
  fErrCallback_(ERR_DEBUG, " RS485 Begins\n");
  } // end of RS485::begin

// get rid of the buffer
void RS485::stop ()
{
  reset ();
  free (data_);
  data_ = NULL;
  fErrCallback_(ERR_DEBUG, " RS485 Stops\n");
} // end of RS485::stop

// called after an error to return to "not in a packet"
void RS485::reset ()
  {
  haveSTX_ = false;
  available_ = false;
  inputPos_ = 0;
  startTime_ = 0;
  transmitTime_ = 0;
  } // end of RS485::reset

// calculate 8-bit CRC
byte RS485::crc8 (const byte *addr, byte len)
{
  byte crc = 0;
  while (len--)
    {
    byte inbyte = *addr++;
    for (byte i = 8; i; i--)
      {
      byte mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix)
        crc ^= 0x8C;
      inbyte >>= 1;
      }  // end of for
    }  // end of while
  return crc;
}  // end of RS485::crc8



// send a byte complemented, repeated
// only values sent would be (in hex):
//   0F, 1E, 2D, 3C, 4B, 5A, 69, 78, 87, 96, A5, B4, C3, D2, E1, F0
void RS485::sendComplemented (const byte what)
{
byte c;

  // first nibble
  c = what >> 4;
  fWriteCallback_ ((c << 4) | (c ^ 0x0F));

  // second nibble
  c = what & 0x0F;
  fWriteCallback_ ((c << 4) | (c ^ 0x0F));

}  // end of RS485::sendComplemented

// send a message of "length" bytes (max 255) to other end
// put STX at start, ETX at end, and add CRC
bool RS485::sendMsg (const byte * data, const byte length)
{
  static int run = 0;
  byte screw_pattern[] =  {1,1,0,1,0};          //{0,0,1,0,1,1};
  logger.printf("--------run = %d -------------\n", run);
  //fErrCallback_(ERR_DEBUG, "-------------------------Sending message-----------------------------------------\n");
  // no callback? Can't send
  if (fWriteCallback_ == NULL) {
    fErrCallback_(ERR_RS485_NO_CALLBACK, "RS485: no write callback\n");
    return false;
  }
  //fErrCallback_(ERR_DEBUG, "RS485: sendMsg\n");
#ifndef SCREW_STX
  fWriteCallback_ (STX);  // STX
#else
  if (screw_pattern[run])
	  fWriteCallback_ (STX);  // screw only some messages
  else
	  fErrCallback_(ERR_DEBUG, "RS485: Screwing-up (omiting) STX\n"); 
#endif
#ifndef SCREW_DATA   
  for (byte i = 0; i < length; i++) {
    sendComplemented (data [i]);
    //logger.print(data[i], HEX);
  }
#else 
  int rand=random(length);
  if (!screw_pattern[run]) {			// screw-up
    logger.printf("screw_pattern[run] = %d\n", screw_pattern[run]);
  	for (byte i = 0; i < length; i++) {
  		if (i==rand) {
  			sendComplemented ((rand%2==1)?STX:ETX);
  			//fWriteCallback_ (ETX);
  			logger.printf("RS485: Screwing-up data with %s\n",((rand%2==1)?"STX":"ETX"));
  		}
  		else
  			sendComplemented (data [i]);
  	}	// for
  }
  else	
	for (byte i = 0; i < length; i++)	// no screw-up
		sendComplemented (data [i]);
#endif
#ifndef SCREW_ETX
  fWriteCallback_ (ETX);  // ETX
#else
  if (screw_pattern[run]) 
  	  fWriteCallback_ (ETX);  // ETX
  else
  	  fErrCallback_(ERR_DEBUG, "RS485: Screwing-up (omiting) ETX\n"); 
#endif

#ifndef SCREW_CRC
  sendComplemented (crc8 (data, length));
#else
  if (!screw_pattern[run]) {
  	fErrCallback_(ERR_DEBUG, "RS485: Screwing-up CRC\n"); 
	  sendComplemented (~crc8 (data, length));
  }
  else
	  sendComplemented (crc8 (data, length));
#endif
  if (run++ == sizeof(screw_pattern))
    run = 0;
  transmitTime_ = millis ();
  return true;
  
}  // end of RS485::sendMsg

// called periodically from main loop to process data and
// assemble the finished packet in 'data_'
// RS485.update returns 0 (ERR_OK) if no data, 1 (RS485_DATA_PRESENT) if data avail
// or negative int (see errorID enum in helpers.h) if error
int RS485::update ()
{
  // no data? can't go ahead (eg. begin() not called)
  if (data_ == NULL)
    return ERR_OK;

  // no callbacks? Can't read
  if (fAvailableCallback_ == NULL || fReadCallback_ == NULL || fErrCallback_ == NULL) 
  {
    if(fErrCallback_)
      fErrCallback_(ERR_RS485_NO_CALLBACK, "RS485: no read/available callback\n");
    return ERR_RS485_NO_CALLBACK;
  }
  while (fAvailableCallback_ () > 0)
  {
    byte inByte = fReadCallback_ ();

    switch (inByte)
      {

        case STX:   // start of text
		      fErrCallback_(ERR_DEBUG, "RS485: got STX\n");	
          haveSTX_ = true;
          haveETX_ = false;
          inputPos_ = 0;
          firstNibble_ = true;
          startTime_ = millis ();
          break;

        case ETX:   // end of text (now expect the CRC check)
          if (haveSTX_)		// nik
			        haveETX_ = true;
			    fErrCallback_(ERR_DEBUG, "RS485: got ETX\n");	
          break;

        default:
          // wait until packet officially starts
          if (!haveSTX_)
            break;

          // check byte is in valid form (4 bits followed by 4 bits complemented)
          if ((inByte >> 4) != ((inByte & 0x0F) ^ 0x0F) )
            {
            reset ();
            fErrCallback_(ERR_RS485_INV_BYTE_CODE, "RS485: Invalid byte received\n");
			      //break;  // bad character
            return ERR_RS485_INV_BYTE_CODE;
            } // end if bad byte

          // convert back
          inByte >>= 4;
          // high-order nibble?
          if (firstNibble_)
            {
            currentByte_ = inByte;
            firstNibble_ = false;
            break;
            }  // end of first nibble

          // low-order nibble
          currentByte_ <<= 4;
          currentByte_ |= inByte;
          firstNibble_ = true;
          // if we have the ETX this must be the CRC
          if (haveETX_)
            {
            if (crc8 (data_, inputPos_) != currentByte_)
              {   // wrong CRC
              reset ();
			        fErrCallback_(ERR_RS485_BAD_CRC, "RS485:  Bad CRC\n");
              return ERR_RS485_BAD_CRC;
			        }   // end of bad CRC
            available_ = true;
			      fErrCallback_(ERR_DEBUG, "RS485: got MSG\n");	
			      haveETX_ = haveSTX_ = false;		//Nik: to be able to catch timeout
				  //logger.printf((const char *)data_);
            return RS485_DATA_PRESENT;  // show data ready
            }  // end if have ETX already

          // keep adding if not full
          if (inputPos_ < bufferSize_) {      
            data_ [inputPos_++] = currentByte_;
            //fErrCallback_(ERR_DEBUG, (const char *)&currentByte_);
            //logger.print(currentByte_, HEX);
          }
          else
            {
            reset (); // overflow, start again
			      fErrCallback_(ERR_RS485_BUF_OVERFLOW, "RS485: Buffer overflow\n");
            return ERR_RS485_BUF_OVERFLOW; 
            }
          break;    // end of default case
      }  // end of switch
    }  // end of while incoming data
	
	// check for timeout - if we have STX, but not ETX fo a while
	if(isPacketStarted())
	{				
	  if ((unsigned long)(millis() - startTime_) > PACKET_TIMEOUT)
	    {                 // yes, timeout
		  fErrCallback_(ERR_RS485_TIMEOUT, "RS485: packet receive takes too long\n");
		  reset ();
	    }
  } 
	return ERR_OK;  // not ready yet
} // end of RS485::update
