
#include "mm_c.h"

mm_c::mm_c(float omega):
	d_omega(omega),
	d_mu(0.5),
	d_x_1(0.0,0.0),
	d_x_2(0.0,0.0),
	d_a_1(0.0,0.0),
	d_a_2(0.0,0.0),
	d_gain_mu(0.01),
	d_gain_omega(0.25 * d_gain_mu * d_gain_mu)
{}


gr_complex mm_c::slicer(gr_complex x)
{
	float real=SLICE_0_R, imag=SLICE_0_I;
	
	if(x.real() > 0.0)
		real = SLICE_1_R;
	
	if(x.imag() > 0.0)
		imag = SLICE_1_I;
	
	return gr_complex(real,imag);
}

float mm_c::update(gr_complex x_0, gr_complex a_0)
{
	//mm vars
	gr_complex x,y,u;
	
	x = (a_0 - d_a_2) * conj(d_x_1);
	y = (x_0 - d_x_2) * conj(d_a_1);
	u = y - x;
	d_mm = u.real();		//error signal
	
	//limit d_mm
	if (d_mm > 1.0) d_mm = 1.0;
	else if (d_mm < -1.0) d_mm = -1.0;

	//error feedback
	d_omega = d_omega + d_gain_omega * d_mm;
	
	//limit omega
/*		
	if (d_omega > d_max_omega)
		d_omega = d_max_omega;
	else if (d_omega < d_min_omega)
		d_omega = d_min_omega;
*/
	//update mu		
	d_mu = d_mu + d_omega + d_gain_mu * d_mm;
	//process mu / ii advances after burst processing for burst timing
	
	//update delay taps
	d_x_2 = d_x_1;
	d_x_1 = x_0;
	d_a_2 = d_a_1;
	d_a_1 = a_0;

	return d_mu;
}


float mm_c::update(gr_complex x_0)
{
	return update(x_0, slicer(x_0) );
}



