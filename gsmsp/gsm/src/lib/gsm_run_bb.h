/* -*- c++ -*- */
#ifndef INCLUDED_GR_EXAMPLE_B_SQUARE_BB_H
#define INCLUDED_GR_EXAMPLE_B_SQUARE_BB_H

#include <gr_sync_block.h>

class gsm_run_bb;

/*
 * We use boost::shared_ptr's instead of raw pointers for all access
 * to gr_blocks (and many other data structures).  The shared_ptr gets
 * us transparent reference counting, which greatly simplifies storage
 * management issues.  This is especially helpful in our hybrid
 * C++ / Python system.
 *
 * See http://www.boost.org/libs/smart_ptr/smart_ptr.htm
 *
 * As a convention, the _sptr suffix indicates a boost::shared_ptr
 */
typedef boost::shared_ptr<gsm_run_bb> gsm_run_bb_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of howto_square2_ff.
 *
 * To avoid accidental use of raw pointers, howto_square2_ff's
 * constructor is private.  howto_make_square2_ff is the public
 * interface for creating new instances.
 */
gsm_run_bb_sptr gsm_make_run_bb ();

enum TimeslotConfig { TS_CONFIG_UNKNOWN, TS_CONFIG_CCH, TS_CONFIG_SDCCH8 };
struct timeslot_st
{
	unsigned char data[102][156];
	unsigned char n_max_frames;
	TimeslotConfig config;
};

/*!
 * \brief square2 a stream of floats.
 * \ingroup block
 *
 * This uses the preferred technique: subclassing gr_sync_block.
 */
class gsm_run_bb : public gr_sync_block
{
	private:
		enum TMode { EModeFindFCCH, EModeFindSCH, EModeProcessSCH, EModeProcessL2 };
  // The friend declaration allows howto_make_square2_ff to
  // access the private constructor.

  friend gsm_run_bb_sptr gsm_make_run_bb ();

  gsm_run_bb ();  	// private constructor
	int find_sch();
	int process_sch();
	int find_fch();
	int handle_l2();
	//int find_fch(const unsigned char *start, const unsigned char *end);
	//int FillWorkingBuffer(const unsigned char *start, const unsigned char *end);
	void ChangeSearchMode(TMode new_mode);
 public:
  ~gsm_run_bb ();	// public destructor

   void forecast(int noutput_items, gr_vector_int &ninput_items_required);
  // Where all the action really happens

  int work (int noutput_items,
	    gr_vector_const_void_star &input_items,
	    gr_vector_void_star &output_items);
	private:
		int ts_n;
		unsigned char syncbits_match[64];
		int bits_processed;
		int fcch_zero_count;
		unsigned char *burst_ptr;
//		unsigned char burst[156];
		struct timeslot_st timeslots[8];
		TMode mode;
		//unsigned char mode;
		const unsigned char *iStart, *iPtr, *iEnd;
		unsigned char iFchLastBit;
		unsigned char iPhase;
		int iFN;
};

#endif /* INCLUDED_HOWTO_SQUARE2_FF_H */
