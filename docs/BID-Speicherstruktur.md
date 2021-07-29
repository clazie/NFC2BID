
## Struktur

```c
#pragma pack(push, 1)
typedef struct {
  endian16  checksum;         // [0..1]  Checksum, sum up bytes from 2..127
  endian16  act_charge;       // [2..3]
  endian16  max_charged;      // [4..5]
  endian16  chg_cycles;       // [6..7]
  uint8_t   unknown_8[63-7];  // [8..63]
  uint8_t   year;             // [64]
  uint8_t   month;            // [65]
  uint8_t   day;              // [66]
  uint8_t   acctyp;           // [67]
  uint8_t   cells;            // [68]
  endian16  capacity;         // [69..70]
  endian16  chg_current;      // [71..72]
  endian16  dischg_current;   // [73..74]
  endian16  dischg_limit;     // [75..76] discharge limit in mv/Cell
  uint8_t   d_peak;           // [77] delta peak value
  uint8_t   cut_temp;         // [78] cut off temperature
  uint8_t   unknown;
  uint8_t   unknown75[128-80];
}tBID_data;
#pragma pack(pop)
```
Infos gibt|s hier[https://www.rc-network.de/threads/bid-bzw-ams-mit-pc-auslesen486956/](https://www.rc-network.de/threads/bid-bzw-ams-mit-pc-auslesen.486956/) und hier:[https://rchelifan.org/viewtopic.php?f=63&t=59994&hilit=BID+Chip](https://rchelifan.org/viewtopic.php?f=63&t=59994&hilit=BID+Chip).

* Checksumme
* Akkutyp
* Zellenzahl
* Akkukapazität
* Ladestrom
* Entladestrom
* Akkudatum (Tag, Monat, Jahr)
* Anzahl Zyklen
* geladene Kapazität (letzter Ladevorgang)
* entladene Kapazität (letzter Entladevorgang)
* geladene Kapazität (Maximalwert)
* entladene Kapazität (Maximalwert)

Zur Speicherorganisation: Es stehen insgesamt 256 Byte zur Verfügung (8 x 256 Byte = 2 kBit)
Die Daten werden vom Byte 0 bis 127 geschrieben und anschließend ein zweites Mal von Byte 128 bis 255,
also eine 1:1 Spiegelung. Für die obengenannten Daten werden derzeit 23 Bytes genutzt (gespiegelt also 46 Byte von 256 zur Verfügung stehenden Bytes).

+----------------+
|J1     +-+ C1   |
|--+    +-+      |
|  |1   8 7 6 5  |
|  |2   +-----+  |
|  |3   ) IC1 |  |
|  |4   +-----+  |
|--+    1 2 3 4  |
+----------------+

J1:
1 ... or --> VCC --> Arduino 5V
2 ... ws --> SCL --> Arduino Pin 5
3 ... gn --> SDA --> Arduino Pin 4
4 ... sw --> GND --> Arduino GND


IC1: A21SC (=> 24C02 ?)
1 ... E0  --> GND
2 ... E1  --> GND
3 ... E2  --> GND
4 ... GND --> J1-4
5 ... SDA --> J1-3
6 ... SCL --> J1-2
7 ... /WC --> GND  --> C1-1
8 ... VCC --> J1-1 --> C1-2

```c++
#include <Wire.h>


#define BID_ADDRESS (0xA << 3 | 0x0)

int bid_year, bid_month, bid_day;
int bid_cells, bid_acctyp, bid_cap_h, bid_cap_l, bid_chcurr_h, bid_chcurr_l, bid_dchcurr_h, bid_dchcurr_l;
int bid_actc_h, bid_actc_l, bid_actdc_h, bid_actdc_l, bid_maxc_h, bid_maxc_l;
int bid_chcnt_h, bid_chcnt_l;

// EEPROM Routine from http://www.arduino.cc/playground/Code/I2CEEPROM
// adopted to read EEPROMS with 1 Byte address
byte i2c_eeprom_read_byte( int deviceaddress, byte eeaddress ) {
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
//  Wire.send((int)(eeaddress >> 8)); // MSB
//  Wire.send((int)(eeaddress & 0xFF)); // LSB
  Wire.send((int)(eeaddress)); 
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,1);
  if (Wire.available()) rdata = Wire.receive();
  return rdata;
}

void setup() {
  // ...
  Wire.begin();   // start Wire library as I2C-Bus Master
  Serial.begin(9600); // 
  
    int i;
    byte bid_data;
    
    Serial.print("BID-Adresse: ");
    Serial.println(BID_ADDRESS, HEX);    
    
  Serial.println("Start EEPROM_READ");
for (i = 0; i <= 255; i++){
    Serial.print(i, DEC);
    Serial.print(": ");
    bid_data = i2c_eeprom_read_byte( BID_ADDRESS, i );
    Serial.println(bid_data, HEX); 
      switch (i) {
    case 2:
      //Actual Chaged Capacity (mAh) High Byte
      bid_actc_h = bid_data;
      break;
    case 3:
      //Actual Chaged Capacity (mAh) Low Byte
      bid_actc_l = bid_data;
      break;
    case 4:
      //Maximum Chaged Capacity (mAh) High Byte
      bid_maxc_h = bid_data;
      break;
    case 5:
      //Maxiimum Chaged Capacity (mAh) Low Byte
      bid_maxc_l = bid_data;
      break;
    case 6:
      //Charging Cycles High Byte
      bid_chcnt_h = bid_data;
      break;
    case 7:
      //Charging Cycles Low Byte
      bid_chcnt_l = bid_data;
      break;

    case 64:
      //Year
      bid_year = bid_data;
      break;
    case 65:
      //Month
      bid_month = bid_data;
      break;
    case 66:
      //Day
      bid_day = bid_data;
      break;
    case 67:
      //Accu Type
      bid_acctyp = bid_data;
      break;
    case 68:
      //Cells
      bid_cells = bid_data;
      break;
    case 69:
      //Capacaity(mAh) High Byte
      bid_cap_h = bid_data;
      break;
    case 70:
      //Capacaity(mAh) Low Byte
      bid_cap_l = bid_data;
      break;
    case 71:
      //Charg. Current(mA) High Byte
      bid_chcurr_h = bid_data;
      break;
    case 72:
      //Charg. Current(mA) Low Byte
      bid_chcurr_l = bid_data;
      break;
    case 73:
      //Discharg. Current(mA) High Byte
      bid_dchcurr_h = bid_data;
      break;
    case 74:
      //Discharg. Current(mA) Low Byte
      bid_dchcurr_l = bid_data;
      break;
  }

    delay(5);
}
  Serial.println("End EEPROM_READ");


Serial.println("BID Daten:");
Serial.print("Datum: ");
Serial.print(bid_day);
Serial.print(".");
Serial.print(bid_month);
Serial.print(".");
Serial.print(2000 + bid_year);
Serial.println("");

Serial.print("Akku-Typ: ");
      switch (bid_acctyp) {
        case 0:
          Serial.print("NiCd");
          break;
        case 1:
          Serial.print("NiMH");
          break;
        case 2:
          Serial.print("LiPo");
          break;
        default:
          Serial.print(bid_acctyp);
          Serial.print("?");
          break;
      }
Serial.println("");

Serial.print("Zellenzahl: ");
Serial.print(bid_cells);
Serial.print(" -> ");
      switch (bid_acctyp) {
        case 0:
          Serial.print(1.2 * bid_cells);
          break;
        case 1:
          Serial.print(1.2 * bid_cells);
          break;
        case 2:
          Serial.print(3.7 * bid_cells);
          break;
      }
Serial.print(" V");
Serial.println("");

Serial.print("Kapazitaet: ");
Serial.print(bid_cap_h * 256 + bid_cap_l);
Serial.print(" mAh");
Serial.println("");

Serial.print("Ladestrom: ");
Serial.print(bid_chcurr_h * 256 + bid_chcurr_l);
Serial.print(" mA");
Serial.println("");

Serial.print("Entladestrom: ");
Serial.print(bid_dchcurr_h * 256 + bid_dchcurr_l);
Serial.print(" mA");
Serial.println("");


Serial.print("akt. Lad.: ");
Serial.print(bid_actc_h * 256 + bid_actc_l);
Serial.print(" mAh");
Serial.println("");

Serial.print("max. Lad.: ");
Serial.print(bid_maxc_h * 256 + bid_maxc_l);
Serial.print(" mAh");
Serial.println("");

Serial.print("Ladezyklen: ");
Serial.print(bid_chcnt_h * 256 + bid_chcnt_l);
Serial.println("");

  // ...
}

void loop() {
  // ...
  // ...
}

```