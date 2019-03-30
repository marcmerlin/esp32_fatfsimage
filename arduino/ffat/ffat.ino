/* Code gathered by Marc MERLIN <marc_soft@merlins.org> from code found
on gitter (license unknown) and in esp32-arduino's 
libraries/FFat/examples/FFat_Test/FFat_Test.ino */

#include <esp_partition.h>
#include "FFat.h"

void partloop(esp_partition_type_t part_type) {
  esp_partition_iterator_t iterator = NULL;
  const esp_partition_t *next_partition = NULL;
  iterator = esp_partition_find(part_type, ESP_PARTITION_SUBTYPE_ANY, NULL);
  while (iterator) {
     next_partition = esp_partition_get(iterator);
     if (next_partition != NULL) {
        Serial.printf("partiton addr: 0x%06x; size: 0x%06x; label: %s\n", next_partition->address, next_partition->size, next_partition->label);  
     iterator = esp_partition_next(iterator);
    }
  }
}
 
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void setup(){
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    Serial.println("Partition list:");
    partloop(ESP_PARTITION_TYPE_APP);
    partloop(ESP_PARTITION_TYPE_DATA);

    Serial.println("\n\nTrying to mount ffat partition if present");
 
    if(!FFat.begin()){
        Serial.println("FFat Mount Failed");
        return;
    }
 
    Serial.println("File system mounted");
    Serial.printf("Total space: %10lu\n", FFat.totalBytes());
    Serial.printf("Free space:  %10lu\n\n", FFat.freeBytes());
    listDir(FFat, "/gifs64", 5);
}
 
void loop(){}
