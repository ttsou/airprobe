//
// C++ Implementation: gsm_receiver_config
//
// Description:
// This file contains classes which define gsm_receiver configuration
// and the burst_counter which is used to store internal state of the receiver
// when it's synchronized
//
// Author: Piotr Krysik <perper@o2.pl>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef INCLUDED_GSM_RECEIVER_CONFIG_H
#define INCLUDED_GSM_RECEIVER_CONFIG_H

#include <vector>
#include <algorithm>
#include <math.h>
#include <stdint.h>
#include <gsm_constants.h>

typedef enum {empty, fcch_burst, sch_burst, normal_burst, rach_burst, dummy} burst_type;
typedef enum {unknown, multiframe_26, multiframe_51} multiframe_type;

class multiframe_configuration
{
  private:
    multiframe_type d_type;
    std::vector<burst_type> d_burst_types;
  public:
    multiframe_configuration() {
      d_type = unknown;
      fill(d_burst_types.begin(), d_burst_types.end(), empty);
    }

    ~multiframe_configuration() {}

    void set_type(multiframe_type type) {
      if (type == multiframe_26) {
        d_burst_types.resize(26);
      } else {
        d_burst_types.resize(51);
      }

      d_type = type;
    }

    void set_burst_type(int nr, burst_type type) {
      d_burst_types[nr] = type;
    }

    multiframe_type get_type() {
      return d_type;
    }

    burst_type get_burst_type(int nr) {
      return d_burst_types[nr];
    }
};

class burst_counter
{
  private:
    const int d_OSR;
    uint32_t d_t1, d_t2, d_t3, d_timeslot_nr;
    double d_offset_fractional;
    double d_offset_integer;
  public:
    burst_counter(int osr):
        d_OSR(osr),
        d_t1(0),
        d_t2(0),
        d_t3(0),
        d_timeslot_nr(0),
        d_offset_fractional(0.0),
        d_offset_integer(0.0) {
    }

    burst_counter(int osr, uint32_t t1, uint32_t t2, uint32_t t3, uint32_t timeslot_nr):
        d_OSR(osr),
        d_t1(t1),
        d_t2(t2),
        d_t3(t3),
        d_timeslot_nr(timeslot_nr),
        d_offset_fractional(0.0),
        d_offset_integer(0.0) {
      double first_sample_position = (get_frame_nr() * 8 + timeslot_nr) * TS_BITS;
      d_offset_integer = floor(first_sample_position);
      d_offset_fractional = first_sample_position - floor(first_sample_position);
    }

    burst_counter & operator++(int);
    void set(uint32_t t1, uint32_t t2, uint32_t t3, uint32_t timeslot_nr);

    uint32_t get_t1() {
      return d_t1;
    }

    uint32_t get_t2() {
      return d_t2;
    }

    uint32_t get_t3() {
      return d_t3;
    }

    uint32_t get_timeslot_nr() {
      return d_timeslot_nr;
    }

    uint32_t get_frame_nr() {
      return (51 * 26 * d_t1) + (51 * (((d_t3 + 26) - d_t2) % 26)) + d_t3;
    }

    unsigned get_offset() {
      return (unsigned)d_offset_integer;
    }
};

class channel_configuration
{
  private:
    multiframe_configuration d_timeslots_descriptions[TS_PER_FRAME];
  public:
    channel_configuration() {
      for (int i = 0; i < TS_PER_FRAME; i++) {
        d_timeslots_descriptions[i].set_type(unknown);
      }
    }

    void set_multiframe_type(int timeslot_nr, multiframe_type type) {
      d_timeslots_descriptions[timeslot_nr].set_type(type);
    }

    void set_burst_types(int timeslot_nr, const unsigned mapping[], unsigned mapping_size, burst_type b_type) {
      unsigned i;
      for (i = 0; i < mapping_size; i++) {
        d_timeslots_descriptions[timeslot_nr].set_burst_type(mapping[i], b_type);
      }
    }

    void set_single_burst_type(int timeslot_nr, int burst_nr, burst_type b_type) {
      d_timeslots_descriptions[timeslot_nr].set_burst_type(burst_nr, b_type);
    }

    burst_type get_burst_type(burst_counter burst_nr);
};

#endif /* INCLUDED_GSM_RECEIVER_CONFIG_H */
