#ifndef _NONLINEAR_DIFFUSION_HPP
#define _NONLINEAR_DIFFUSION_HPP


#include <cmath>		// std::abs
#include <csignal>		// sig_atomic_t
#include <exception>	// std::exception
#include <functional>	// std::function
#include <iostream>		// Could be removed
#include <vector>


typedef std::vector<double> vecd;


double mean_square_rel(const vecd &A, const vecd &B, size_t first, size_t last);
double max_dif_rel(const vecd &A, const vecd &B, size_t first, size_t last);


// \frac{dw}{dt}=\frac{d^2y}{dx^2}, y=y(x,t) — ?, w = w (x,y)
void nonlenear_diffusion_nonuniform_1_2 (double tau,
										 double eps, // relative error for w
										 double left_bounder_cond, // y(left_border,Time+tau) = left_bounder_cond
										 double right_bounder_cond, // \frac{y(right_border,Time+tau)}{dx} = right_bounder_cond
										 std::function<vecd (const vecd &, const vecd &, size_t, size_t)> wunc, // first argument is array of x_i, second — array of y(x_i,t); return value — array of w(x_i,y_i)
										 const vecd &x, // array with (non)uniform grid
										 vecd &y// array with initial coundition and for results
									 	);


void nonlenear_diffusion_nonuniform_wind_1_2 (
		double tau,
		double eps, // relative error for w
		double left_bounder_cond, // y(left_border,Time+tau) = left_bounder_cond
		double right_bounder_cond, // \frac{y(right_border,Time+tau)}{dx} = right_bounder_cond
		const vecd &A,
		const vecd &B,
		std::function<vecd (const vecd &, const vecd &, size_t, size_t)> wunc, // first argument is array of x_i, second — array of y(x_i,t); return value — array of w(x_i,y_i)
		const vecd &x, // array with (non)uniform grid
		vecd &y// array with initial condition and for results
);


#endif // _NONLINEAR_DIFFUSION_HPP
