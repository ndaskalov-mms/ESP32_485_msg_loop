/*
 Callbacks and helper functions for RS485 non-blocking library
 Licence: Released for public use.
 RS485 non-blocking requires callbacks for read, write, etc ops
 Hear are the definitions for Master and Slave channes
 

*/


#define HEADER_SIZE 1   //STX, 1 byte, no encoding
#define FOOTER_SIZE 1   //ETX, 1 byte  no encoding
#define CRC_SIZE    2   //CRC, 1 byte but due to encoding is send as 2 bytes
#define CMD_SIZE    4   //COMMAND/REPLY + SRC|DESTINATION, 2 bytes but due to encoding is send as 4 bytes
#define PAYLOAD_OFFSET  (CMD_SIZE/2) // payload offset in the received buffer 
#define TxFIFO_SIZE 128 //ESP32 TxFIFO, we expect that whole message get's in FIFO in order not to block loop()
#define MAX_MSG_LENGHT  ((TxFIFO_SIZE - (HEADER_SIZE + FOOTER_SIZE + CRC_SIZE))/2)  //message encodding doubles every nibble except STX and ETX
#define MAX_PAYLOAD_SIZE  (MAX_MSG_LENGHT - CMD_SIZE/2)   // shall be max 60 = ((128-(1+1+1))/2) - 2
#define RxBUF_SIZE 128  //no material limit
#define BROADCAST_ID    0x0F
#define REPLY_OFFSET    0x80
#define DST_BITS		0x0F
#define SRC_BITS		0xF0
#define SRC_SHIFT		4
#define CMD_OFFSET 		0
#define DST_OFFSET		1
#define CMD_HEADER_LEN 	2 			// CMD + SRC|DST

// COMMAND CODES
// commands definition
// high nibble contains command code, while low nibble contains the recipient ID
// Master ID is always 0x0, while 0xF is reserved for broadcast message (not used so far)
// RESULT CODES - the result code is simply the command code with MS bit set:  REPLY_OFFSET = 0x80
// ----------------- PING -----------------------------------
//#define PAYLOAD_OFFSET		  2
#define CMD_OFFSET			  0
#define PING                  0x1                  // ping 
#define PING_PAYLD_LEN        0                    // ping message has no payload
#define PING_RES              (PING|REPLY_OFFSET)  // ping reply code 
#define PING_RES_PAYLD_LEN    4                    // ping payload is 4 bytes, content not defined yet

// ----------------- POLL -----------------------------------
#define POLL_ZONES            0x2 // poll the extenders for zones status
#define POLL_PAYLD_LEN        0   // poll message has no payload
#define POLL_ZONES_RES        (POLL_ZONES|REPLY_OFFSET) // poll the extenders for zones status
#define POLL_RES_PAYLD_LEN    SZONERES_LEN              // poll payload is 4bits per zone
                                                  

// ----------------- SET OUTPUTS -----------------------------
#define SET_OUTS              0x3                       // set output relay
#define SET_OUTS_PAYLD_LEN    1                         // SET_OUTS payload is 1 byte, 1 byte for outputs (8 outputs, LSB is OUT1)
#define SET_OUTS_RES          (SET_OUTS|REPLY_OFFSET)   // set output relay reply
#define SET_OUTS_RES_PAYLD_LEN 2                        // SET_OUTS payload is 2 bytes 1 byte for outputs (8 outputs, LSB is OUT1) and 1 byte status

// ----------------- FREE_CMD -----------------------------
#define FREE_CMD             0x4                        // send free text (can be binary too)
#define FREE_CMD_PAYLD_LEN   (MAX_PAYLOAD_SIZE)       // FREE_CMD payload is up to MAX_PAYLOAD_SIZE
#define FREE_CMD_DATA_LEN   (MAX_PAYLOAD_SIZE-2)       // FREE_CMD payload is up to MAX_PAYLOAD_SIZE - subCmd - payload size
#define FREE_CMD_HDR_LEN    2                         // one byte subcmd + one byte data len
#define FREE_CMD_SUB_CMD_OFFSET		0
#define FREE_CMD_DATA_LEN_OFFSET	1
#define FREE_CMD_DATA_OFFSET		2
#define FREE_CMD_RES         (FREE_CMD  | REPLY_OFFSET)
#define EXTRACT_LEN_FROM_PAYLOAD 0xFF
// free cmd sub-commands
#define	FREE_TEXT_SUB_CMD	0x1
#define	FREE_TEXT_DATA_LEN	FREE_CMD_DATA_LEN
#define SET_ZONE_SUB_CMD	0x2						// send    SLAVE_ZONE_CNT triplets of 3 bytes: [0] = zone.gpio; [1] = zone.mux; [2] = zoneID
//#define SET_ZONE_DATA_LEN	3						
#define GET_ZONE_SUB_CMD  0x3						// returns SLAVE_ZONE_CNT triplets of 3 bytes: [0] = zone.gpio; [1] = zone.mux; [2] = zoneID
#define SET_PGM_SUB_CMD  0x4           // send    SLAVE_ZONE_CNT triplets of 3 bytes: [0] = zone.gpio; [1] = zone.mux; [2] = zoneID
//#define SET_PGM_DATA_LEN 3           
#define GET_PGM_SUB_CMD  0x5           // returns SLAVE_ZONE_CNT triplets of 3 bytes: [0] = zone.gpio; [1] = zone.mux; [2] = zoneID
//
//
#define RS485_DATA_PRESENT    1         // RS485.update returns 0 (ERR_OK) if no data, 1 (RS485_DATA_PRESENT) if data avail or negative if error

// command records structure for cmdDB
struct COMMAND {
  int cmdID;
  byte len;
  unsigned long last_transmitted;
};
//
// commands database to look-up command params and store temporary data (like last transmition time)
// 
struct COMMAND cmdDB[] = {{PING, PING_PAYLD_LEN,  0}, {POLL_ZONES, POLL_PAYLD_LEN, 0}, {SET_OUTS, SET_OUTS_PAYLD_LEN, 0}, \
                          {FREE_CMD, EXTRACT_LEN_FROM_PAYLOAD,  0}} ;  // len 0 means the len of the payload will determine the message len

struct MSG {
  byte cmd;
  byte src;
  byte dst;
  int  len;
  byte payload[MAX_PAYLOAD_SIZE];
  int parse_err;
  byte subCmd;
  int dataLen;
} ;


void LogMsg(char *formatStr, int len, byte cmd, byte src_dst, byte *payload) {
	if(!DEBUG)
		return;
  lprintf(formatStr, len, cmd, src_dst);
  for(int i =0; i< len-CMD_HEADER_LEN; i++)
    lprintf ("%2x ", payload[i]);                // there is one byte cmd|dst
  logger.println();
}

void LogMsg(char *formatStr, int len, byte cmd, byte dst, byte src, byte *payload) {
  if(!DEBUG)
		return;
	lprintf(formatStr, len, cmd, dst, src);
  for(int i =0; i< len-CMD_HEADER_LEN; i++)
    lprintf ("%2x ", payload[i]);                // there is one byte cmd|dst
   logger.println();
}

void LogMsg(char *formatStr, int len, byte cmd, byte src_dst, byte subCmd, int pldLen,  byte *payload) {
  if(!DEBUG)
		return;
	lprintf(formatStr, len, cmd, src_dst, subCmd, pldLen);
  for(int i =0; i< pldLen; i++)
    lprintf ("%2x ", payload[i]);                // there is one byte cmd|dst
   logger.println();
}

void LogMsg(char *formatStr, int len, byte cmd, byte dst, byte src, byte subCmd, int pldLen,  byte *payload) {
  if(!DEBUG)
    return;
  lprintf(formatStr, len, cmd, dst, src, subCmd, pldLen);
  for(int i =0; i< pldLen; i++)
    lprintf ("%2x ", payload[i]);                // there is one byte cmd|dst
   logger.println();
}
int findCmdEntry(byte cmd) {
  //ErrWrite(ERR_DEBUG, "Looking for record for cmd code   %d \n", cmd);
  cmd = cmd & ~REPLY_OFFSET;                      // clear reply flag if any
  //lprintf("Looking at index  %d out of  %d:\n", cmd, sizeof(cmdDB)/sizeof(struct COMMAND)-1);
  for (int i = 0; i < sizeof(cmdDB)/sizeof(struct COMMAND); i++) {
    if(cmdDB[i].cmdID == cmd) {
      //ErrWrite(ERR_DEBUG,"Found cmd at  index %d\n", i);
      return  i;
    }
  }
  ErrWrite(ERR_DEBUG, "Command index not found!!!!!!!\n");
  return ERR_DB_INDEX_NOT_FND;
}

//
byte test_msg [][FREE_CMD_PAYLD_LEN] = {{"5Hello world;6Hello world;7Hello world;8Hello world;9Hel"},\
                                     {"1234567890123456789012345678901234567890123456789012345678"},\
                                     {"~!@#$%^&*()_+~!@#$%^&*()_+~!@#$%^&*()_+~!@#$%^&*()_+~!@#"},
                                     {"zxcvbnm,./';lkjhgfdsaqwertyuiop[]\\][poiuytrewqasdfghjkl"}};
				 
//
// read board address//
byte readOwnAdr() {												// TODO read the address from dDIO switch
return SLAVE_ADDRESS1;	
}
