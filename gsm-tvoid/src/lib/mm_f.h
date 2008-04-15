#ifndef INCLUDED_MM_F_H
#define INCLUDED_MM_F_H

#include <gr_math.h>

class mm_f {
private:
public:

	float			d_mm;
	
	float			d_omega;		//relative sample rate
	float			d_mu;

	float			d_gain_mu;
	float			d_gain_omega;

	//delay taps
	float		d_x_1;	//last input sample
	float		d_a_1;	//last decision

	mm_f (float omega);  

	float slicer(float x);

	float update(float sample, float decision);	//return mu

	float update(float sample);  //use built in decision

};



#endif
