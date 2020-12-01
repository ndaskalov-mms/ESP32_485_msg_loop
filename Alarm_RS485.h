/*
 RS485 protocol library - non-blocking.

 Devised and written by Nick Gammon.
 Date: 4 December 2012
 Version: 1.0
 
 modiied by ndaskalov:
 added support for serial logging from class functions
 added timeout support 

 Licence: Released for public use.

*/

#include "Arduino.h"


class RS485
  {

  typedef void (*ErrCallback)    (int err, char * what);    // error num and text description
  typedef size_t (*WriteCallback)  (const byte what);    // send a byte to serial port
  typedef int  (*AvailableCallback)  ();    // return number of bytes available
  typedef int  (*ReadCallback)  ();    // read a byte from serial port
  
  enum {
        STX = '\2',   // start of text
        ETX = '\3'    // end of text
  };  // end of enum

  // callback functions to do reading/writing
  ReadCallback fReadCallback_;
  AvailableCallback fAvailableCallback_;
  WriteCallback fWriteCallback_;
  ErrCallback fErrCallback_;

  // where we save incoming stuff
  byte * data_;

  // how much data is in the buffer
  const int bufferSize_;

  // this is true once we have valid data in buf
  bool available_;

  // an STX (start of text) signals a packet start
  bool haveSTX_;


  // variables below are set when we get an STX
  bool haveETX_;
  byte inputPos_;
  byte currentByte_;
  bool firstNibble_;
  unsigned long startTime_;

  // helper private functions
  byte crc8 (const byte *addr, byte len);
  void sendComplemented (const byte what);

  public:

    // constructor
    RS485 (ReadCallback fReadCallback,
           AvailableCallback fAvailableCallback,
           WriteCallback fWriteCallback,
		       ErrCallback fErrCallback,
           const byte bufferSize) :
        fReadCallback_ (fReadCallback),
        fAvailableCallback_ (fAvailableCallback),
        fWriteCallback_ (fWriteCallback),
		    fErrCallback_ (fErrCallback),
        data_ (NULL),
        bufferSize_ (bufferSize)
         {}

    // destructor - frees memory used
    ~RS485 () { stop (); }

    // allocate memory for buf_
    void begin ();

    // free memory in buf_
    void stop ();

    // handle incoming data, return true if packet ready
    int update ();

    // reset to no incoming data (eg. after a timeout)
    void reset ();

    // send data
    bool sendMsg (const byte * data, const byte length);

    // returns true if packet available
    bool available () const { return available_; };

    // once available, returns the address of the current message
    byte * getData ()   const { return data_; }
    byte   getLength () const { return inputPos_; }

    // return when last packet started
    unsigned long getPacketStartTime () const { return startTime_; }

    // return true if a packet has started to be received
    bool isPacketStarted () const { return haveSTX_; }

  }; // end of class RS485
