/*
** Dillon Bastan, 2016
** This object outputs a variable number of bandpass filters of incoming signal
** A filterbank using biquad filtering similar to the fffb~ object 
*/


#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects
#include <math.h>			// math library


//Constants
const int NUM_COEFS = 4;


//BP filter
struct BPFilter {
	double centerFreq, bandwidth, gain, pan;
	double x1, x2, y1, y2;
	double coefs[NUM_COEFS];
};

//Object Struct
typedef struct _filterbank {
	t_pxobject ob;
	int numFilters;
	struct BPFilter *filterbank;
	double level;
} t_filterbank;



// Standard
void *filterbank_new(t_symbol *s, long argc, t_atom *argv);
void filterbank_free(t_filterbank *x);
void filterbank_assist(t_filterbank *x, void *b, long m, long a, char *s);
//DSP
void filterbank_dsp64(t_filterbank *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void filterbank_perform64(t_filterbank *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
//Additional
void filterbank_freqs(t_filterbank *x, t_symbol *s, long argc, t_atom *argv);
void filterbank_gains(t_filterbank *x, t_symbol *s, long argc, t_atom *argv);
void filterbank_pans(t_filterbank *x, t_symbol *s, long argc, t_atom *argv);
void filterbank_bandwidths(t_filterbank *x, t_symbol *s, long argc, t_atom *argv);
void filterbank_level(t_filterbank *x, double level);
void filterbank_num(t_filterbank *x, int num);
void filterbank_computeCoefs(t_filterbank *x);
void filterbank_initParams(t_filterbank *x);
double *filterbank_filterInput(t_filterbank *x, t_double in);


// global class pointer variable
static t_class *filterbank_class = NULL;



//***********************************************************************************************



void ext_main(void *r)
{
	t_class *c = class_new("db.filterbank~", (method)filterbank_new, (method)dsp_free, (long)sizeof(t_filterbank), 0L, A_GIMME, 0);
	//
	class_addmethod(c, (method)filterbank_dsp64, "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)filterbank_assist, "assist",	A_CANT, 0);
	//
	class_addmethod(c, (method)filterbank_freqs, "freqs", A_GIMME, 0);
	class_addmethod(c, (method)filterbank_gains, "gains", A_GIMME, 0);
	class_addmethod(c, (method)filterbank_pans, "pans", A_GIMME, 0);
	class_addmethod(c, (method)filterbank_bandwidths, "bandwidths", A_GIMME, 0);
	class_addmethod(c, (method)filterbank_level, "level", A_FLOAT, 0);
	class_addmethod(c, (method)filterbank_num, "num", A_LONG, 0);
	//
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	filterbank_class = c;
}



/*	Standard 	*******************************************************************/



//new
void *filterbank_new(t_symbol *s, long argc, t_atom *argv)
{
	t_filterbank *x = (t_filterbank *)object_alloc(filterbank_class);
	if (x) {
		//
		dsp_setup((t_pxobject *)x, 1);
		//2 Signal Outs
		outlet_new((t_pxobject *)x, "signal");
		outlet_new((t_pxobject *)x, "signal");
		//Check for arguments
		if (atom_getlong(argv))
			x->numFilters = atom_getlong(argv);
		else
			x->numFilters = 1;
		if (atom_getlong(++argv))
			x->level = atom_getlong(argv);
		else
			x->level = 1;
		//Allocate memory for filterbank
		x->filterbank = (struct BPFilter *)malloc(x->numFilters * sizeof(struct BPFilter));
		//Initialize parameter values
		filterbank_initParams(x);
	}
	return (x);
}


//free
void filterbank_free(t_filterbank *x)
{
	free(x->filterbank);
}


//Assist
void filterbank_assist(t_filterbank *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) {
		switch (a) {
		case 0:
			sprintf(s,"(Signal) Input");
			break;
		}
	}
	else {
		switch (a) {
		case 0:
			sprintf(s,"(Signal) Filter Bank");
			break;
		}
	}
}



/*	Additional 	*******************************************************************/



//Min and Max functions
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


//Initial parameter values
void filterbank_initParams(t_filterbank *x) 
{
	int size = x->numFilters;
    struct BPFilter *filters = x->filterbank;
    for (int i = 0; i < size; i++, filters++) {
    	if (!filters->centerFreq)
    		filters->centerFreq = 1000;
    	if (!filters->gain)
    		filters->gain = 1;
    	if (!filters->pan)
    		filters->pan = 0.5;
    	if (!filters->bandwidth)
    		filters->bandwidth = 1;
    	post("%ld, %.2f, %.2f, %.2f", i, filters->centerFreq, filters->gain, filters->bandwidth);
    }
    filterbank_computeCoefs(x);
}


//Update filter center frequencies
void filterbank_freqs(t_filterbank *x, t_symbol *s, long argc, t_atom *argv) 
{
	int size = (argc < x->numFilters)? argc : x->numFilters;
	t_atom *args = argv;
    struct BPFilter *filters = x->filterbank;
    for (int i = 0; i < size; i++, args++, filters++) {
    	filters->centerFreq = max( atom_getfloat(args), 20 );
    }
    filterbank_computeCoefs(x);
}


//Update filter gains
void filterbank_gains(t_filterbank *x, t_symbol *s, long argc, t_atom *argv) 
{
	int size = (argc < x->numFilters)? argc : x->numFilters;
	t_atom *args = argv;
    struct BPFilter *filters = x->filterbank;
    for (int i = 0; i < size; i++, args++, filters++) {
    	filters->gain = atom_getfloat(args);
    }
    filterbank_computeCoefs(x);
}


//Update filter pannings
void filterbank_pans(t_filterbank *x, t_symbol *s, long argc, t_atom *argv) 
{
	int size = (argc < x->numFilters)? argc : x->numFilters;
	t_atom *args = argv;
    struct BPFilter *filters = x->filterbank;
    for (int i = 0; i < size; i++, args++, filters++) {
    	filters->pan = max ( min( atom_getfloat(args), 1), 0);
    }
}


//Update filter bandwidths
void filterbank_bandwidths(t_filterbank *x, t_symbol *s, long argc, t_atom *argv) 
{
	int size = (argc < x->numFilters)? argc : x->numFilters;
	t_atom *args = argv;
    struct BPFilter *filters = x->filterbank;
    for (int i = 0; i < size; i++, args++, filters++) {
    	filters->bandwidth = max( atom_getfloat(args), 0.0001 );
    }
    filterbank_computeCoefs(x);
}


//Update the master gain
void filterbank_level(t_filterbank *x, double level) 
{
	x->level = level;
}


//Change the number of filters
void filterbank_num(t_filterbank *x, int num) {
	x->numFilters = num;
	//Reallocate memory for array of bandpass
	x->filterbank = (struct BPFilter *)realloc(x->filterbank, x->numFilters * sizeof(struct BPFilter));
	//Initialize parameter values that are empty
	filterbank_initParams(x);
}


//Convert center frequency and bandwidth to coefficients for each filter
void filterbank_computeCoefs(t_filterbank *x) 
{
	struct BPFilter *filters = x->filterbank;
	for (int i = 0; i < x->numFilters; i++, filters++) {
		//Conversions
		double w = 2 * acos(-1) * filters->centerFreq / 44100;
		double a = sin(w) * sinh( (log(2) / 2) * filters->bandwidth * (w / sin(w)) );
		double a0 = 1 + a;
		double *coefs = filters->coefs;
		*coefs = a / a0;
		*(coefs + 1) = -1 * a / a0;
		*(coefs + 2) = -2 * cos(w) / a0;
		*(coefs + 3) = (1 - a) / a0;
	}
}



/*	DSP 	*******************************************************************/



//Registers a function for the signal chain in Max
void filterbank_dsp64(t_filterbank *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), x, filterbank_perform64, 0, NULL);
}


//Perform
void filterbank_perform64(t_filterbank *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	//Access in and out samples
	t_double *in = ins[0];	
	t_double *outL = outs[0];
	t_double *outR = outs[1];
	double *outs;

	while (sampleframes--) {
		//Filter input
		outs = filterbank_filterInput(x, *in++);
		*outL++ = *outs++;
		*outR++ = *outs;
	}
}


//Implement BP filter on input signal
double *filterbank_filterInput(t_filterbank *x, t_double in) 
{
	//Sum of all bp filters in bank
	static double sums[2];
	double sum = 0;
	//Iterate through each filter
	struct BPFilter *filters = x->filterbank;
	for (int i = 0; i < x->numFilters; i++, filters++) {
		double *coefs = filters->coefs;
		double yTemp = filters->y1;
		//Biquad filter
		filters->y1 = *coefs * in + *(coefs + 1) * filters->x2 - *(coefs + 2) * filters->y1 - *(coefs + 3) * filters->y2;
		filters->y1 *= filters->gain;
		//NaN value correcting?
#ifdef DENORM_WANT_FIX
		if (IS_DENORM_NAN_DOUBLE(filters->y1))
			filters->y1 = yTemp = 0;
#endif
#ifdef DENORM_WANT_FIX
		if (IS_DENORM_NAN_DOUBLE(filters->x1))
			filters->x1 = 0;
#endif
		//Update
		filters->y2 = yTemp;
		filters->x2 = filters->x1;
		filters->x1 = in;
		//Add sample value to sum
		sum += filters->y1;
	}
	//Gain adjustment and Panning L/R
	sum *= (1 / sqrt(x->numFilters)) * x->level;
	sums[1] = sum * filters->pan;
	sums[0] = 1 - sums[1];
	//Return sample value
	return sums;
}