
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

