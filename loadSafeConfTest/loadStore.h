#include <esp_littlefs.h>
#include <lfs.h>
#include <lfs_util.h>
#include <LITTLEFS.h>
#include <littlefs_api.h>
#define SPIFFS LITTLEFS

#define FORMAT_LITTLEFS_IF_FAILED true


// calculate 8-bit CRC
byte crc8 (const byte *addr, byte len)
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
}  // end of crc8
//
// init storage
// return: true on succsess WARNING - if storage is not formatted, formats it
//         fail on failure
//
int storageSetup() {
//
    logger.println("Inizializing FS...");
    if (SPIFFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        ErrWrite(ERR_OK, "Storage init success\n"); 
        return true;
    }else{
        ErrWrite(ERR_CRITICAL, "Storage init fail\n"); 
        return false;
    }
  
}
//
// close storage
// return: true on succsess WARNING - if storage is not formatted, formats it
//         fail on failure
//
int storageClose() {
	SPIFFS.end();
    ErrWrite(ERR_OK, "Shutting down storage\n"); 
}

void formatStorage() {
    ErrWrite(ERR_WARNING, "Formatting file system\n");
    SPIFFS.format();
}
void printAlarmConfig(byte * cBuf) {
     for(int i=0; i<sizeof(alarmConfig); ) {
          Serial.printf("Index %d content %d %d %d %d %d %d %d %d %d %d \n", i, cBuf[i++], cBuf[i++],cBuf[i++],cBuf[i++],cBuf[i++],cBuf[i++],cBuf[i++],cBuf[i++],cBuf[i++],cBuf[i++]);
          }
}    

int saveConfig(const char cFileName []) {
//
// SPIFFS.remove(F("/file Name"));
//
//  calc 8 bit checksum first
	alarmConfig.version = SW_VERSION;
    alarmConfig.csum = crc8((byte*) &alarmConfig, sizeof(alarmConfig)-1);
//    
    File cFile = SPIFFS.open(cFileName, "w");
    if (cFile){
        ErrWrite(ERR_DEBUG, "Writing config file\n");
        cFile.write((byte*) &alarmConfig, sizeof(alarmConfig));
        cFile.close();
        }
    else{
        ErrWrite(ERR_CRITICAL, "Problem on create config file!\n");
        return false;
        }
    // readback and verify
    cFile = SPIFFS.open(cFileName, "r");
    if (cFile){
        ErrWrite(ERR_DEBUG, "Reading back config file\n");
        if(int rlen = cFile.read((byte*) &tmpConfig, sizeof(tmpConfig)) != sizeof(tmpConfig)) {
          ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong len!\n");
          //Serial.printf("Problem on reading config file! Read %d expected %d\n",rlen, sizeof(tmpConfig) );
          cFile.close();  
          return false;
          }
        cFile.close();
        byte cs8;
        cs8 = crc8((byte*) &tmpConfig, sizeof(tmpConfig)-1);
        //cs8 += 1;                         // intentional error
        if(cs8  != tmpConfig.csum) {
          ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong csum!\n");
          //Serial.printf("Read CS %d differs from calculated %d", tmpConfig.csum, cs8);
          return false;
          }
        //logger.printf("Content of config file written\n");
        //printAlarmConfig((byte*) &tmpConfig);
        return true;
        }
     else {
        ErrWrite(ERR_CRITICAL, "Problem opening config file\n");
        return false;
        }
}

int readConfig(const char cFileName []) {
//
	if(FORCE_NEW_CONFIG)
		return false;
    File cFile = SPIFFS.open(cFileName, "r");
	memset((void*)&tmpConfig, 0, sizeof(tmpConfig));		// clear tmp buffer
    if (cFile){
        ErrWrite(ERR_DEBUG, "Reading config file\n");
	    if(cFile.size() != sizeof(tmpConfig)) {
          ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong len!\n");
          cFile.close();
          return false;
          } 
        if(int rlen = cFile.read((byte*) &tmpConfig, sizeof(tmpConfig)) != sizeof(tmpConfig)) {
          ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong len!\n");
          cFile.close();
          return false;
          }
        cFile.close();
        byte cs8;
        cs8 = crc8((byte*) &tmpConfig, sizeof(tmpConfig)-1);
        //cs8 += 1;                         // intentional error        
        if(cs8  != tmpConfig.csum) {
          ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong csum!\n");
          //Serial.printf("Read CS %d differs from calculated %d", tmpConfig.csum, cs8);
          return false;
          }
		if(SW_VERSION  != tmpConfig.version) {
          ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong SW version!\n");
          //Serial.printf("Read CS %d differs from calculated %d", tmpConfig.csum, cs8);
          return false;
          }
        logger.printf("Content of config file read\n");
        //printAlarmConfig((byte*) &tmpConfig);
        memcpy((byte*) &alarmConfig, (byte*) &tmpConfig, sizeof(alarmConfig));
        logger.printf("Content of configured DB\n");
        //printAlarmConfig((byte*) &alarmConfig);
        ErrWrite(ERR_DEBUG, "Reading config file done\n");
        return true;
        }
     else {
        ErrWrite(ERR_CRITICAL, "Problem opening config file\n");
        return false;
        }
}
