#include <Arduino.h>
#include <Wire.h>

#include "global.h"

// Setup RFID CHIP
#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>

/**
 * Verlauf 
 * 29.8.16  Status LED als Konstante, NFC Gain im Main code
 * 
 * 
 * 
 * ----------------------------------------------------------------------------
 *  MFRC522 library  https://github.com/miguelbalboa/rfid
 * 
 * Projekt zu lesen im RCN Microcotroller Bereich
 * 
 * Benötigte Lesechips Miface Classic 1K
 * 
 * 
 * WICHTIGE INFOS !!!!!!
 * WIRD ein BIDCHIP(NFC) mit Jahr 2100 abgelegt kann damit der Chip Entfernt werden!!!!
 * 
 * 
 * ----------------------------------------------------------------------------
 * 
 * Verbindung Arduino /// NFC Leser :
 * -----------------------------------
 *             MFRC522      Arduino    
 *             Reader/PCD   Uno / Nano         
 * Signal      Pin          Pin          
 * ----------------------------------
 * RST/Reset   RST          9          
 * SPI SS      SDA(SS)      10      
 * SPI MOSI    MOSI         11
 * SPI MISO    MISO         12 
 * SPI SCK     SCK          13  
 * 
 * 
 * 
 * Verbindung Arduino /// BID Port :
 * -----------------------------------
 *            Robbe Lader     Arduino    
 *             BID            Uno / Nano         
 * Signal      Kabel          Pin          
 * ----------------------------------
 * Masse       Schwarz         GND         
 * SCL         Weiss           5      
 * SCK         Gruen           4

 * 
 */







#define SS_PIN 10  //slave select pin
#define RST_PIN 9  //reset pin

MFRC522 mfrc522(SS_PIN, RST_PIN);        
MFRC522::MIFARE_Key key;

// Frei Einstellbare Parameter
const int status_led_1_Pin = A1;  // Pin fuer Status LED
int BID_Reconnect_time = 3000;  // Zeit wie lange Pause BEI Bidwechsel ist, bei fehler erhöhen
int I2C_Adresse_BID_Chip = 80;   //Adresse des BID Chips
const int I2C_Adresse_BID_no_Chip = 89;  //Dummy adresse um den BUS zu sperren
#define PCD_RxGain RxGain_max

//Setup ROBBE BID Chip
int t = 0;
byte adresse =0;
int dat=0;
byte last_byte = 0;
int counter =0;
bool err =0;
byte BID_NFC[256];
byte BID_aktiv[256];
bool bid_daten_NFC_syncron =0;


int block=2;
byte blockcontent[16] =  {"               "};
byte Chip_serial[16] =   {"               "};
byte aktiver_Chip_serial[16] =   {"               "};
int chip_status =0;
bool _bid_aktiv =0;
bool daten_syncron =1;
bool Chip_wechsel=1;
bool lesefehler=0;
//byte blockcontent[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//all zeros. This can be used to delete a block.
byte readbackblock[18];//This array is used for reading out a block. The MIFARE_Read method requires a buffer that is at least 18 bytes to hold the 16 bytes of a block.


//#################################################
// funktions
//#################################################

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  //Serial.print("EE");
  t=0;
  while (1 < Wire.available()) { // loop through all but the last
    t++;
    Serial.println("loop");
    adresse = Wire.read(); // receive byte as a character       // print the character
    //Serial.println(adresse); 
    dat=1;
  }
  byte x = Wire.read();    // receive byte as an integer
  last_byte = x;
  ///Serial.println("last_byte");
  //Serial.println(last_byte);         // print the integer
  if (t==1){
    dat=0;
    daten_syncron =0;
    BID_aktiv[adresse]=last_byte;

    //DEBUG EEPROM Schreiben    
    //Serial.print("write to ");
    //Serial.print(adresse);
    //Serial.print("  ");
    //Serial.println(last_byte);
  }
}

void requestEvent() 
{
  //Serial.print("Request from Master. Sending: ");
  //Serial.print(x, HEX);
  //Serial.print("\n");
  //Serial.print("read adrr ");
  //Serial.print(last_byte);
  //Serial.print(" ");
  //Serial.println(BID_aktiv[last_byte]);
  Wire.write(BID_aktiv[last_byte]);
}

int writeBlock(int blockNumber, byte arrayAddress[]) 
{
  //this makes sure that we only write into data blocks. Every 4th block is a trailer block for the access/security info.
  int largestModulo4Number=blockNumber/4*4;
  int trailerBlock=largestModulo4Number+3;//determine trailer block for the sector
  if (blockNumber > 2 && (blockNumber+1)%4 == 0){Serial.print(blockNumber);Serial.println(" is a trailer block:");return 2;}//block number is a trailer block (modulo 4); quit and send error code 2
  Serial.print(blockNumber);
  Serial.println(" is a data block:");
  
  /*****************************************authentication of the desired block for access***********************************************************/
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  //byte PCD_Authenticate(byte command, byte blockAddr, MIFARE_Key *key, Uid *uid);
  //this method is used to authenticate a certain block for writing or reading
  //command: See enumerations above -> PICC_CMD_MF_AUTH_KEY_A  = 0x60 (=1100000),    // this command performs authentication with Key A
  //blockAddr is the number of the block from 0 to 15.
  //MIFARE_Key *key is a pointer to the MIFARE_Key struct defined above, this struct needs to be defined for each block. New cards have all A/B= FF FF FF FF FF FF
  //Uid *uid is a pointer to the UID struct that contains the user ID of the card.
  if (status != MFRC522::STATUS_OK) {
         Serial.print("PCD_Authenticate() failed: ");
         //Serial.println(mfrc522.GetStatusCodeName(status));
         return 3;//return "3" as error message
  }
  //it appears the authentication needs to be made before every block read/write within a specific sector.
  //If a different sector is being authenticated access to the previous one is lost.


  /*****************************************writing the block***********************************************************/
        
  status = mfrc522.MIFARE_Write(blockNumber, arrayAddress, 16);//valueBlockA is the block number, MIFARE_Write(block number (0-15), byte array containing 16 values, number of bytes in block (=16))
  //status = mfrc522.MIFARE_Write(9, value1Block, 16);
  if (status != MFRC522::STATUS_OK) {
           Serial.print("MIFARE_Write() failed: ");
           //Serial.println(mfrc522.GetStatusCodeName(status));
           err =1;
           return 4;//return "4" as error message
  }
  Serial.println("block was written");
  return 0;
}

int readBlock(int blockNumber, byte arrayAddress[]) 
{
  int largestModulo4Number=blockNumber/4*4;
  int trailerBlock=largestModulo4Number+3;//determine trailer block for the sector

  /*****************************************authentication of the desired block for access***********************************************************/
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  //byte PCD_Authenticate(byte command, byte blockAddr, MIFARE_Key *key, Uid *uid);
  //this method is used to authenticate a certain block for writing or reading
  //command: See enumerations above -> PICC_CMD_MF_AUTH_KEY_A = 0x60 (=1100000),    // this command performs authentication with Key A
  //blockAddr is the number of the block from 0 to 15.
  //MIFARE_Key *key is a pointer to the MIFARE_Key struct defined above, this struct needs to be defined for each block. New cards have all A/B= FF FF FF FF FF FF
  //Uid *uid is a pointer to the UID struct that contains the user ID of the card.
  if (status != MFRC522::STATUS_OK) {
         Serial.print("PCD_Authenticate() failed (read): ");
         err =1;
         //Serial.println(mfrc522.GetStatusCodeName(status));
         return 3;//return "3" as error message
  }
  //it appears the authentication needs to be made before every block read/write within a specific sector.
  //If a different sector is being authenticated access to the previous one is lost.


  /*****************************************reading a block***********************************************************/
        
  byte buffersize = 18;//we need to define a variable with the read buffer size, since the MIFARE_Read method below needs a pointer to the variable that contains the size... 
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);//&buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
  if (status != MFRC522::STATUS_OK) {
          Serial.print("MIFARE_read() failed: ");
          //Serial.println(mfrc522.GetStatusCodeName(status));
          err =1;
          return 4;//return "4" as error message
  }
  Serial.println("block was read");
  return 0;
}

/***************************************** Lese / Schreibroutinen NFC // I2C **********************************************************************/

void Write_from_Ram_to_NFC()
{
  block=2;
  counter =0;
  for (int z=0 ; z<16 ; z++)
  {
    for (int j=0 ; j<16 ; j++)//print the block content
    {
      digitalWrite(status_led_1_Pin, !digitalRead(status_led_1_Pin));
      blockcontent[j] = BID_aktiv[counter];
      //Serial.print("Block:");
      //Serial.print(block);
      //Serial.print(" byte no:");
      //Serial.print(j);
      //Serial.print(" BID Byte");
      //Serial.print(counter);
      //Serial.print(" Inhalt:");
      //Serial.println(blockcontent[j],HEX);
      counter++;
    }
    writeBlock(block, blockcontent);//the blockcontent array is written into the card block
    block++;
    if (block==3) {
      block++;
    }
    if (block==7) {
      block++;
    }

    if (block==11) {
    block++;
    }

    if (block==15) {
      block++;
    }

    if (block==19) {
      block++;
    }           
    if (block==23) {
      block++;
    }           
  }

  Serial.println("");
  digitalWrite(status_led_1_Pin, HIGH);
}

void read_from_NFC_to_RAM()
{
  Serial.print("Start lesen");
  block=2;
  counter =0;
  for (int z=0 ; z<16 ; z++)
  {
    readBlock(block, readbackblock);
    digitalWrite(status_led_1_Pin, !digitalRead(status_led_1_Pin));
    for (int j=0 ; j<16 ; j++)//print the block contents
    {
      //Serial.print("Block:");
      //Serial.print(block);
      //Serial.print(" byte no:");
      //Serial.print(j);
      //Serial.print(" BID Byte");
      //Serial.print(counter);
      //Serial.print(" Inhalt:");
      //Serial.println(readbackblock[j],HEX);

      BID_NFC[counter]=readbackblock[j];
      counter++;
    // Serial.print (readbackblock[j],HEX);//Serial.write() transmits the ASCII numbers as human readable characters to serial monitor
    }
    block++;
    if (block==3) {
      block++;
    }
    if (block==7) {
      block++;
    }

    if (block==11) {
      block++;
    }

    if (block==15) {
      block++;
    }
    if (block==19) {
      block++;
    }           
    if (block==23) {
      block++;
    }           
  }
  Serial.print("ende lesen");
  Serial.println("");
  digitalWrite(status_led_1_Pin, HIGH);
}

//#################################################

void setup() {
  pinMode(status_led_1_Pin, OUTPUT); 
  Serial.begin(115200);        // Initialize serial communications with the PC
  SPI.begin();               // Init SPI bus
  mfrc522.PCD_Init();        // Init MFRC522 card (in case you wonder what PCD means: proximity coupling device)
  Serial.println("Scan a MIFARE Classic card");
  

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;//keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
  }
  // I2c BID
  //Wire.begin(80);                // join i2c bus with address #80
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent);
  //Serial.print(BID[1]);

  //digitalWrite(2, LOW);
}

void loop()
{
  if (lesefehler==1){
    delay(10);
    Serial.println(" read err loop");
    digitalWrite(status_led_1_Pin, !digitalRead(status_led_1_Pin));
  }
  else
  {
    delay(400);
    if (daten_syncron == 0)
    {
      Serial.println("loop");
      Serial.println("Syncronisieren noetig");
      digitalWrite(status_led_1_Pin, !digitalRead(status_led_1_Pin));
    }
  }
  //digitalWrite(status_led_1_Pin, HIGH);

  /***************************************** contact with a tag/card**********************************************************************/
  mfrc522.PCD_Init();      
  
  // Look for new cards (in case you wonder what PICC means: proximity integrated circuit card)
  if ( ! mfrc522.PICC_IsNewCardPresent()) //if PICC_IsNewCardPresent returns 1, a new card has been found and we continue
  {
    return;//if it did not find a new card is returns a '0' and we return to the start of the loop
  }
  
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) //if PICC_ReadCardSerial returns 1, the "uid" struct 
  {
    return; //if it returns a '0' something went wrong and we return to the start of the loop
  }

  
  Serial.println("Chip gefunden!");

  /*****************************************Auslesen der Seriennummer **********************************************************************/
  block=0;
  err =0;
  readBlock(block, Chip_serial);//read the block back
  if (err==1)
  {
   return; 
  }
    
  Serial.print("Lese Seriennummer aus :");
  for (int j=0 ; j<16 ; j++) //print the block contents
  {
    Serial.print (Chip_serial[j],HEX);//Serial.write() transmits the ASCII numbers as human readable characters to serial monitor
  }
  Serial.println("");        

  /***************************************** Erster BID Bei Programmstart  **********************************************************************/ 
  
  // Fuer Programmstart erste Verbindung zum bid  -- NFC
  if (_bid_aktiv == 0)
  {
    err=0;
    
    for (int z=0 ; z<16 ; z++)
    {
      aktiver_Chip_serial[z]=Chip_serial[z];
    }
  
    read_from_NFC_to_RAM();
  
    for (int z=0 ; z<256 ; z++)
    {
      BID_aktiv[z]=BID_NFC[z];
    }   

    Serial.println("read errors:");  
    Serial.println(err);       
    if (err == 1)
    {
      lesefehler=1;  
    }
    else
    {    
      Wire.begin(I2C_Adresse_BID_Chip);
      daten_syncron =1;
      Chip_wechsel=0;
      lesefehler=0;
      _bid_aktiv=1;
      digitalWrite(status_led_1_Pin, HIGH);
    }
  }
  
  /*****************************************Abgleich ob ein Chip mit anderer Seriennumer gesteckt wurde **********************************************************************/ 
  // Abgleich ob ein Chip mit anderer Seriennumer gesteckt wurde
  Chip_wechsel=0;
  for (int z=0 ; z<16 ; z++)
  {
    if (aktiver_Chip_serial[z] != Chip_serial[z])
    {
      Chip_wechsel=1;
      Serial.println(aktiver_Chip_serial[z]);
      Serial.println(" ");
      Serial.println(Chip_serial[z]);
      Serial.println("Chip wechsel");
    }
  }

  /***************************************** Bei ungleichheit NFC / Ram daten Auf NFC Sichen **********************************************************************/
  if (daten_syncron ==0 && Chip_wechsel==0 && _bid_aktiv==1)
  {
    err=0;
    Write_from_Ram_to_NFC();
    //read_from_NFC_to_RAM();
   
    Serial.println("daten syncronisiert");
    digitalWrite(status_led_1_Pin, HIGH);
    if (err==0)
    {    
      daten_syncron =1;
    }   
  }

  /***************************************** Neuen Chip nicht annehmen solange noch ungesicherte daten da sind **********************************************************************/
  if (daten_syncron ==0 && Chip_wechsel==1 && _bid_aktiv==1)
  {
    Serial.println("Daten Nicht Syncron !!! Kein Chipwechsel");
  }

  /***************************************** Chipwechsel wenn alle daten gesichert **********************************************************************/  
  if (daten_syncron ==1 && Chip_wechsel==1 && _bid_aktiv==1)
  {
    Serial.println("led low");
    
    Wire.begin(I2C_Adresse_BID_no_Chip);

    err=0;     
    read_from_NFC_to_RAM();
    
    digitalWrite(status_led_1_Pin, LOW);
    for (int z=0 ; z<256 ; z++)
    {
      BID_aktiv[z]=BID_NFC[z];
    }

    lesefehler=1;           
    delay(BID_Reconnect_time);
    //Serial.println("Chip Gewechselt");
    if (err==0)
    { 
      // Copy versetzt
      for (int z=0 ; z<16 ; z++)
      {
        aktiver_Chip_serial[z]=Chip_serial[z];
      }       
      Wire.begin(I2C_Adresse_BID_Chip);
      lesefehler=0;
      daten_syncron =1;
      Chip_wechsel=0;    
      Serial.println("Chip Gewechselt");
      if (BID_NFC[64] == 100)
      {
        Serial.println("Jahreszahl 2100 erkannt");
        Wire.begin(I2C_Adresse_BID_no_Chip);
        _bid_aktiv =0;
        daten_syncron =1;
        Chip_wechsel=1;
        delay(5000);  
      }
    } 

    if (_bid_aktiv ==1)
    {
      digitalWrite(status_led_1_Pin, HIGH);
    }
  } 
}
