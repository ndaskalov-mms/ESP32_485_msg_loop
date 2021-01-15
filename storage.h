#include <esp_littlefs.h>
#include <lfs.h>
#include <lfs_util.h>
#include <LITTLEFS.h>
#include <littlefs_api.h>
#define SPIFFS LITTLEFS

#define FORMAT_LITTLEFS_IF_FAILED true
//
// init storage
// return: true on succsess
//         fail on failure
//
int storageSetup() {
    logger.println(F("Inizializing FS..."));
    if (SPIFFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println(F("done."));
    }else{
        Serial.println(F("fail."));
    }
 
    // To remove previous test
    // SPIFFS.remove(F("/testCreate.txt"));
 
    File testFile = SPIFFS.open(F("/testCreate.txt"), "w");
 
    if (testFile){
        Serial.println("Write file content!");
        testFile.print("Here the test text!!");
 
        testFile.close();
    }else{
        Serial.println("Problem on create file!");
    } 
}
