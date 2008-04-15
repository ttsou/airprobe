#ifndef INCLUDED_MM_C_H
#define INCLUDED_MM_C_H

#include <gr_math.h>

#if 0
#define SLICE_0_R	0.0
#define SLICE_0_I	0.0
#define SLICE_1_R	1.0
#define SLICE_1_I	1.0
#else
#define SLICE_0_R	-1.0
#define SLICE_0_I	-1.0
#define SLICE_1_R	1.0
#define SLICE_1_I	1.0
#endif

class mm_c {
private:
public:

	float			d_mm;
	
	float			d_omega;		//relative sample rate
	float			d_mu;

	float			d_gain_mu;
	float			d_gain_omega;

	//delay taps
	gr_complex		d_x_1;	//last input sample
	gr_complex		d_x_2;
	gr_complex		d_a_1;	//last decision
	gr_complex		d_a_2;

	mm_c (float omega);  

	gr_complex slicer(gr_complex x);

	float update(gr_complex sample, gr_complex decision);	//return mu

	float update(gr_complex sample);  //use built in decision

};



#endif
