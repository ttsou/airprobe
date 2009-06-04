#ifndef INCLUDED_GSM_CONSTANTS_H
#define INCLUDED_GSM_CONSTANTS_H

#define GSM_SYMBOL_RATE  (1625000.0/6.0) //symbols per second
#define GSM_SYMBOL_PERIOD (1.0/GSM_SYMBOL_RATE) //seconds per symbol

//Burst timing
#define TAIL_BITS         3
#define GUARD_BITS        8
#define GUARD_FRACTIONAL  0.25 //fractional part of guard period
#define GUARD_PERIOD      GUARD_BITS + GUARD_FRACTIONAL
#define DATA_BITS         58   //size of 1 data block in normal burst
#define N_TRAIN_BITS      26
#define N_SYNC_BITS       64
#define USEFUL_BITS       142  //(2*DATA_BITS + N_TRAIN_BITS )
#define FCCH_BITS         USEFUL_BITS
#define BURST_SIZE        (USEFUL_BITS+2*TAIL_BITS)

#define SCH_DATA_LEN      39
#define TS_BITS           (TAIL_BITS+USEFUL_BITS+TAIL_BITS+GUARD_BITS)  //a full TS (156 bits)
#define TS_PER_FRAME      8
#define FRAME_BITS        (TS_PER_FRAME * TS_BITS + 2) // 156.25 * 8
#define FCCH_POS          TAIL_BITS
#define SYNC_POS          39
#define TRAIN_POS         58
#define SAFETY_MARGIN     6

#define FCCH_HITS_NEEDED        (USEFUL_BITS - 4) 
#define FCCH_MAX_MISSES         1
#define FCCH_MAX_FREQ_OFFSET    100

#define CHAN_IMP_RESP_LENGTH  5

static const unsigned char SYNC_BITS[] = {
  1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
  0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1,
  0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1
};

const unsigned FCCH_FRAMES[] = {0,10,20,30,40};
const unsigned SCH_FRAMES[] = {1,11,21,31,41};
const unsigned BCCH_FRAMES[] = {2,3,4,5,  12}; //remove 12

// Sync             : .+...++.+..+++.++++++.++++++....++.+..+.+.+++.+.+...+..++++..+..
// Diff Encoded Sync: .++..+.+++.+..++.....++.....+...+.+++.+++++..+++++..++.+...+.++.

#define TSC0  0
#define TSC1  1
#define TSC2  2
#define TSC3  3
#define TSC4  4
#define TSC5  5
#define TSC6  6
#define TSC7  7
#define TS_DUMMY 8

#define TRAIN_SEQ_NUM 9

static const unsigned char train_seq[TRAIN_SEQ_NUM][N_TRAIN_BITS] = {
  {0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},
  {0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},
  {0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0},
  {0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0},
  {0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1},
  {0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0},
  {1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1} // DUMMY
};


//Dummy burst 0xFB 76 0A 4E 09 10 1F 1C 5C 5C 57 4A 33 39 E9 F1 2F A8
static const unsigned char dummy_burst[] = {
  1, 1, 1, 1, 1, 0, 1, 1, 0, 1,
  1, 1, 0, 1, 1, 0, 0, 0, 0, 0,
  1, 0, 1, 0, 0, 1, 0, 0, 1, 1,
  1, 0, 0, 0, 0, 0, 1, 0, 0, 1,
  0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,

  0, 1, 1, 1, 0, 0, 0, 1, 0, 1,
  1, 1, 0, 0, 0, 1, 0, 1, 1, 1,
  0, 0, 0, 1, 0, 1,

  0, 1, 1, 1, 0, 1, 0, 0, 1, 0,
  1, 0, 0, 0, 1, 1, 0, 0, 1, 1,
  0, 0, 1, 1, 1, 0, 0, 1, 1, 1,
  1, 0, 1, 0, 0, 1, 1, 1, 1, 1,
  0, 0, 0, 1, 0, 0, 1, 0, 1, 1,
  1, 1, 1, 0, 1, 0, 1, 0
};

//Diff encoded train_seq
//TSC0: +.++.+++..+...++..++.+++..
//TSC1: +.+++.++..++...+..++.+++..
//TSC2: +++...+..++..+++.++...+..+
//TSC3: +++..+...++.+++..++..+...+
//TSC4: +..+.++++..+.++....+.++++.
//TSC5: +++.+..++++.+....++.+..+++
//TSC6: .+++.+....++.+..++++.+....
//TSC7: ...++...+..++.+++..++...+.
//TSC8: ..........................
//TSC9: ++..+..+++..+..+++..+..+++


#endif /* INCLUDED_GSM_CONSTANTS_H */
