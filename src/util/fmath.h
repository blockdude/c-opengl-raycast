#ifndef FMATH_H
#define FMATH_H

#define PI 3.1415926535897932384626433832795f

float magnitude			( float x, float y );
void normalize			( float x, float y, float *x_norm, float *y_norm );
float clamp				( float val, float min, float max );

float min				( float a, float b );
float max				( float a, float b );

void center_pos			( float x, float y, float *x_out, float *y_out );
float center_x			( float x );
float center_y			( float y );

#endif
