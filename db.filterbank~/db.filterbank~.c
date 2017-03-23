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
	double centerFreq, bandwidth, gain, pan;		//Parameters
	double x1, x2, y1, y2;							//Delay History
	double coefs[NUM_COEFS];						//Coefficients
};

//Object Struct
typedef struct _filterbank {
	t_pxobject ob;
	int numFilters;					//(Attribute: num) Number of filters
	int maxFilters;
	struct BPFilter *filterbank;	//Array of BPFilters
	double level;					//Master gain for filterbank
	double samplerate;
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
void filterbank_clear(t_filterbank *x);
void filterbank_computeCoefs(t_filterbank *x);
void filterbank_initParams(t_filterbank *x);
double *filterbank_bpFilterbankStereo(struct BPFilter *filters, int size, t_double in);


// global class pointer variable
static t_class *filterbank_class = NULL;



/*  Main   	***********************************************************************************************/



//Main
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
	class_addmethod(c, (method)filterbank_num, "num", A_DEFLONG, 0);
	class_addmethod(c, (method)filterbank_clear, "clear", A_GIMME, 0);
	//
	CLASS_ATTR_LONG(c, "num", 0, t_filterbank, numFilters);
	//
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	filterbank_class = c;
}



/*	Standard 	*******************************************************************/



//Min and Max functions
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


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
		//Register object (attach to itsself) so get attr notifications
		object_attach_byptr_register(x, x, CLASS_BOX);
		//Get samplerate
		x->samplerate = sys_getsr();
		//Init Values
		x->maxFilters = 16;
		x->numFilters = 1;
		x->level = 1;
		//Check for arguments
		for (int i = 0; i < argc; i++) {
			switch(i) {
				case 0:
					x->maxFilters = max( atom_getlong(argv), 1);
					break;
				case 1:
					x->numFilters = max( atom_getlong(argv), 1);
					break;
				case 2:
					x->level = max( atom_getfloat(argv), 0);
					break;
			}
		}
		//Allocate memory for filterbank
		x->filterbank = (struct BPFilter *)malloc(x->maxFilters * sizeof(struct BPFilter));
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



//Initialize empty parameter values
void filterbank_initParams(t_filterbank *x) 
{
	int size = x->maxFilters;
    struct BPFilter *filters = x->filterbank;
    for (int i = 0; i < size; i++) {
    	//Determine if value previously set, if not use previous filter's parameters
    	filters[i].centerFreq = 1000;
    	filters[i].gain = 1;
    	filters[i].pan = 0.5;
    	filters[i].bandwidth = 1;
    }
    filterbank_computeCoefs(x);
}


//Number of filters
void filterbank_num(t_filterbank *x, int num) {
	x->numFilters = min( max(num, 1), x->maxFilters);
}


//Update filter center frequencies
void filterbank_freqs(t_filterbank *x, t_symbol *s, long argc, t_atom *argv) 
{
	int size = (argc < x->numFilters)? argc : x->numFilters;
	t_atom *args = argv;
    struct BPFilter *filters = x->filterbank;
    for (int i = 0; i < size; i++, args++, filters++) {
    	filters->centerFreq = max( min(atom_getfloat(args), x->samplerate / 2), 20 );
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


//Convert center frequency and bandwidth to coefficients for each filter
void filterbank_computeCoefs(t_filterbank *x) 
{
	struct BPFilter *filters = x->filterbank;
	for (int i = 0; i < x->numFilters; i++, filters++) {
		//Conversions
		double w = 2 * acos(-1) * filters->centerFreq / x->samplerate;
		double a = sin(w) * sinh( (log(2) / 2) * filters->bandwidth * (w / sin(w)) );
		double a0 = 1 + a;
		double *coefs = filters->coefs;
		*coefs = a / a0;
		*(coefs + 1) = -1 * a / a0;
		*(coefs + 2) = -2 * cos(w) / a0;
		*(coefs + 3) = (1 - a) / a0;
	}
}


//Clear delay history if filter blows up
void filterbank_clear(t_filterbank *x) 
{
	int size = x->numFilters;
    struct BPFilter *filters = x->filterbank;
    for (int i = 0; i < size; i++, filters++) {
    	filters->x1 = filters->x2 = filters->y1 = filters->y2 = 0;
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
	double *outX;

	while (sampleframes--) {
		//Filterbank input
		outX = filterbank_bpFilterbankStereo(x->filterbank, x->numFilters, *in++);
		*outL++ = *outX++ * x->level;
		*outR++ = *outX * x->level;
	}
}


//Implement array of BP filter on input signal
double *filterbank_bpFilterbankStereo(struct BPFilter *filters, int size, t_double in) 
{
	//Sum of all bp filters in bank
	static double sums[2] = {0, 0};
	//Iterate through each filter
	for (int i = 0; i < size; i++, filters++) {
		double *coefs = filters->coefs;
		double yTemp = filters->y1;
		//Biquad filter
		filters->y1 = *coefs * in + *(coefs + 1) * filters->x2 - *(coefs + 2) * filters->y1 - *(coefs + 3) * filters->y2;
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
		//Add sample value to sum with gain and panning
		sums[0] += filters->y1 * filters->gain * (1 - filters->pan);
		sums[1] += filters->y1 * filters->gain * filters->pan;
	}
	//Gain adjustment
	sums[0] *= (1 / sqrt(size));
	sums[1] *= (1 / sqrt(size));
	//add processed sample values
	return sums;
}