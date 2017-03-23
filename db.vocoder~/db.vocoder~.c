/*
** Dillon Bastan, 2016
** Classic Vocoder
*/



/*  Setup  	***********************************************************************************************/



//Includes
#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include <math.h>


//Constants
const int NUM_COEFS = 4;


//Bandpass Biquad Filter
struct QuadFilter {
	double x1, x2, y1, y2;
	double coefs[NUM_COEFS];
};


//Parameters for each set of band processing (modulator and carrier)
struct BandParams {
	double freq, bw, gain, pan;
	double envY, envStart, envDiff;
	long envCount;
};


//Vocoder Object Struct
typedef struct _vocoder {
	t_pxobject ob;
	double samplerate;
	int gate;
	struct BandParams *bandParams;	//Parameter values for each band (set of mod and carrier filterbanks)
	//Filter variables:
	int maxBands, numBands;
	struct QuadFilter *modFB;
	struct QuadFilter *carrFB;
	//Envelope Follower variables:
	long envAttack, envRelease;
	int sendEnvList, envIsLog;
	t_atom *envList;
	void *envList_outlet;
} t_vocoder;


// Standard
void *vocoder_new(t_symbol *s, long argc, t_atom *argv);
void vocoder_free(t_vocoder *x);
void vocoder_assist(t_vocoder *x, void *b, long m, long a, char *s);
//Messages
void vocoder_numBands(t_vocoder *x, int num);
void vocoder_freqs(t_vocoder *x, t_symbol *s, long argc, t_atom *argv);
void vocoder_bandwidths(t_vocoder *x, t_symbol *s, long argc, t_atom *argv);
void vocoder_gains(t_vocoder *x, t_symbol *s, long argc, t_atom *argv);
void vocoder_pans(t_vocoder *x, t_symbol *s, long argc, t_atom *argv);
void vocoder_envelope(t_vocoder *x, t_symbol *s, long argc, t_atom *argv);
void vocoder_envIsLog(t_vocoder *x, int state);
void vocoder_outputEnvelopes(t_vocoder *x, int state);
void vocoder_clear(t_vocoder *x);
//Internal
double atom_getlfdouble(t_atom *v);
void vocoder_initNullParams(t_vocoder *x);
void vocoder_computeAllBPCoefs(t_vocoder *x);
void vocoder_computeBPCoefs(int bands, double samplerate, struct QuadFilter *fb, struct BandParams *params);
void vocoder_envListOut(void *outlet, int size, t_atom *envList);
//DSP
void vocoder_dsp64(t_vocoder *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void vocoder_perform64(t_vocoder *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
double *vocoder_vocoder(t_vocoder *x, t_double modSig, t_double carrSig, long frame);
double vocoder_biquad(struct QuadFilter *filter, t_double in);
double vocoder_envelopeFollow(double in, long attack, long release, int isLog, struct BandParams *params, long frame);



// global class pointer variable
static t_class *vocoder_class = NULL;



/*  Main   	***********************************************************************************************/



//Init Routine
void ext_main(void *r)
{
	t_class *c = class_new("db.vocoder~", (method)vocoder_new, (method)vocoder_free, (long)sizeof(t_vocoder), 0L, A_GIMME, 0);
	//
	class_addmethod(c, (method)vocoder_dsp64, "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)vocoder_assist, "assist",	A_CANT, 0);
	//
	class_addmethod(c, (method)vocoder_numBands, "num", A_DEFLONG, 0);
	class_addmethod(c, (method)vocoder_freqs, "freqs", A_GIMME, 0);
	class_addmethod(c, (method)vocoder_bandwidths, "bandwidths", A_GIMME, 0);
	class_addmethod(c, (method)vocoder_gains, "gains", A_GIMME, 0);
	class_addmethod(c, (method)vocoder_pans, "pans", A_GIMME, 0);
	class_addmethod(c, (method)vocoder_envelope, "envelope", A_GIMME, 0);
	class_addmethod(c, (method)vocoder_envIsLog, "logenvelopes", A_DEFLONG, 0);
	class_addmethod(c, (method)vocoder_outputEnvelopes, "envelopesout", A_DEFLONG, 0);
	class_addmethod(c, (method)vocoder_clear, "clear", A_GIMME, 0);
	//
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	vocoder_class = c;
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
void *vocoder_new(t_symbol *s, long argc, t_atom *argv)
{
	t_vocoder *x = (t_vocoder *)object_alloc(vocoder_class);
	if (x) {
		//
		dsp_setup((t_pxobject *)x, 2);
		//Outlets
		x->envList_outlet = outlet_new((t_object *)x, NULL);
		outlet_new((t_pxobject *)x, "signal");
		outlet_new((t_pxobject *)x, "signal");
		//Initialize parameters
		x->samplerate = sys_getsr();
		x->gate = 0;
		x->envAttack = 10;
		x->envRelease = 1000;
		x->sendEnvList = 0;
		//args
		atom_arg_getlong(&x->maxBands, 0, argc, argv);
		x->maxBands = max( x->maxBands, 1);
		atom_arg_getlong(&x->numBands, 1, argc, argv);
		x->numBands = min( max(x->numBands, 1), x->maxBands);
	}
	//Initialize empty parameters and pointers
	vocoder_initNullParams(x);
	return (x);
}


//free
void vocoder_free(t_vocoder *x)
{
	dsp_free((t_pxobject *)x);
	free(x->envList);
	free(x->bandParams);
	free(x->modFB);
	free(x->carrFB);
}


//Assist
void vocoder_assist(t_vocoder *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) {
		switch (a) {
		case 0:
			sprintf(s,"(signal) Modulator");
			break;
		case 1:
			sprintf(s,"(signal) Carrier");
		}
	}
	else {
		switch (a) {
		case 0:
			sprintf(s,"(signal) Vocoder Left");
			break;
		case 1:
			sprintf(s,"(signal) Vocoder Right");
			break;
		case 2:
			sprintf(s,"(list) Modulator Envelopes");
		}
	}
}



/*	Messages 	*******************************************************************/



//Update the number of bands
void vocoder_numBands(t_vocoder *x, int num)
{
	x->numBands = min( max(num, 1), x->maxBands);
}


//Update Center frequencies for filterbanks
void vocoder_freqs(t_vocoder *x, t_symbol *s, long argc, t_atom *argv)
{
	int size = (argc < x->numBands)? argc : x->numBands;
    struct BandParams *bands = x->bandParams;
    for (int i = 0; i < size; i++, argv++) {
    	bands[i].freq = max( min(atom_getlfdouble(argv), x->samplerate / 2), 20 );
    }
    vocoder_computeAllBPCoefs(x);
}


//Update bandwidths for filterbanks
void vocoder_bandwidths(t_vocoder *x, t_symbol *s, long argc, t_atom *argv)
{
	int size = (argc < x->numBands)? argc : x->numBands;
    struct BandParams *bands = x->bandParams;
    //If only one argument, then change all values to it
    if (size == 1) {
    	for (int i = 0; i < x->numBands; i++) {
	    	bands[i].bw = max(atom_getlfdouble(argv), 0.0001);
	    }
    } else {
	    for (int i = 0; i < size; i++, argv++) {
	    	bands[i].bw = max(atom_getlfdouble(argv), 0.0001);
	    }
	}
    vocoder_computeAllBPCoefs(x);
}


//Update gains for filterbands
void vocoder_gains(t_vocoder *x, t_symbol *s, long argc, t_atom *argv)
{
	int size = (argc < x->numBands)? argc : x->numBands;
    struct BandParams *bands = x->bandParams;
    //If only one argument, then change all values to it
    if (size == 1) {
    	for (int i = 0; i < x->numBands; i++) {
    		bands[i].gain = atom_getlfdouble(argv);
    	}
    } else {
	    for (int i = 0; i < size; i++, argv++) {
	    	bands[i].gain = atom_getlfdouble(argv);
	    }
	}
}


//Update pans for filterbands
void vocoder_pans(t_vocoder *x, t_symbol *s, long argc, t_atom *argv)
{
	int size = (argc < x->numBands)? argc : x->numBands;
    struct BandParams *bands = x->bandParams;
    for (int i = 0; i < size; i++, argv++) {
    	bands[i].pan = atom_getlfdouble(argv);
    }
}


//Set attack and release times in samples for envelope follower
void vocoder_envelope(t_vocoder *x, t_symbol *s, long argc, t_atom *argv)
{
	x->envAttack = max( atom_getlfdouble(argv++), 1);
	x->envRelease = max( atom_getlfdouble(argv), 1);
}


//Choose envelope following to be logrithmic or not
void vocoder_envIsLog(t_vocoder *x, int state)
{
	x->envIsLog = min( max(state, 0), 1 );
}


//Booelean whether or not to send out envelope values list
void vocoder_outputEnvelopes(t_vocoder *x, int state) 
{
	x->sendEnvList = max( min(state, 1), 0);
}


//Clear bandpass filter delay histories
void vocoder_clear(t_vocoder *x)
{
	int size = x->maxBands;
	struct QuadFilter *mFB = x->modFB;
	struct QuadFilter *cFB = x->carrFB;
    for (int i = 0; i < size; i++) {
    	mFB[i].x1 = mFB[i].x2 = mFB[i].y1 = mFB[i].y2 = 0;
    	cFB[i].x1 = cFB[i].x2 = cFB[i].y1 = cFB[i].y2 = 0;
    }
    //Briefly cut release
    long temp = x->envRelease;
    x->envRelease = 1;
    x->envRelease = temp;
}



/*	Internal 	*******************************************************************/



//Get a double from a long or float atom
double atom_getlfdouble(t_atom *v)
{
	double r = 0;
	switch( atom_gettype(v) ) {
		case A_LONG:
			r = atom_getlong(v);
			break;
		case A_FLOAT:
			r = atom_getfloat(v);
			break;
	}
	return r;
}


//Initialize empty parameters and pointers
void vocoder_initNullParams(t_vocoder *x)
{
	int size = x->maxBands;
	x->bandParams = (struct BandParams *)malloc(size * sizeof(struct BandParams));
	x->modFB = (struct QuadFilter *)malloc(size * sizeof(struct QuadFilter));
	x->carrFB = (struct QuadFilter *)malloc(size * sizeof(struct QuadFilter));
	x->envList = (t_atom *)malloc(size * sizeof(t_atom));
    struct BandParams *params = x->bandParams;
    for (int i = 0; i < size; i++) {
    	params[i].freq = 1000;
    	params[i].gain = 1;
    	params[i].pan = 0.5;
    	params[i].bw = 1;
    	params[i].envY = 0;
    	params[i].envStart = 0;
    	params[i].envCount = 0;
    	params[i].envDiff = 0;
    }
    //Compute filter coefficients
    vocoder_computeAllBPCoefs(x);
    //Open audio gate
    x->gate = 1;
}


//Convert coefficients for all filterbanks
void vocoder_computeAllBPCoefs(t_vocoder *x)
{
	vocoder_computeBPCoefs(x->numBands, x->samplerate, x->modFB, x->bandParams);
	vocoder_computeBPCoefs(x->numBands, x->samplerate, x->carrFB, x->bandParams);
}


//Convert center frequency and bandwidth to coefficients for each filter in a filterbank as bandpass filters
void vocoder_computeBPCoefs(int bands, double samplerate, struct QuadFilter *fb, struct BandParams *params)
{
	for (int i = 0; i < bands; i++) {
		//Conversions
		double w = 2 * PI * params[i].freq / samplerate;
		double a = sin(w) * sinh( (log(2) / 2) * params[i].bw * (w / sin(w)) );
		double a0 = 1 + a;
		double *coefs = fb[i].coefs;
		*coefs = a / a0;
		*(coefs + 1) = -1 * a / a0;
		*(coefs + 2) = -2 * cos(w) / a0;
		*(coefs + 3) = (1 - a) / a0;
	}
}


//Send out a list of the current envelope values
void vocoder_envListOut(void *outlet, int size, t_atom *envList)
{
	outlet_list(outlet, NULL, size, envList);
}



/*	DSP 	*******************************************************************/



//Registers signal function
void vocoder_dsp64(t_vocoder *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), x, vocoder_perform64, 0, NULL);
	//Init sample histories
	vocoder_clear(x);
}


//Perform
void vocoder_perform64(t_vocoder *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	//Access in and out samples
	t_double *modSig = ins[0];		//Modulator
	t_double *carrSig = ins[1];		//Carrier
	t_double *outL = outs[0];		//Left out
	t_double *outR = outs[1];		//Right Out
	double *outX;					//Multi Out Array
	//Gate to prevent audio output at certain times
	if (x->gate) {
		//Iterate through samples
		while (sampleframes--) {
			//Apply vocoder
			outX = vocoder_vocoder(x, *modSig++, *carrSig++, sampleframes);
			//Output samples
			*outL++ = *outX++;
			*outR++ = *outX;
		}
	}
}


//Classic Vocoder
double *vocoder_vocoder(t_vocoder *x, t_double modSig, t_double carrSig, long frame)
{
	double bands = x->numBands;
	struct BandParams *params = x->bandParams;
	//Sum of sample values for stereo channels
	static double sums[2] = {0, 0};
	//Loop through all bands
	for (int i = 0; i < bands; i++) 
	{
		//QuadFilter carrier and modulator
		double carrier = vocoder_biquad(&x->carrFB[i], carrSig);
		double modulator = vocoder_biquad(&x->modFB[i], modSig);
		//Envelope follow modulator
		modulator = vocoder_envelopeFollow(modulator, x->envAttack, x->envRelease, x->envIsLog, &params[i], frame);
		//Apply gain
		modulator *= params[i].gain;
		if (x->sendEnvList)
			atom_setfloat(&x->envList[i], modulator);
		//Apply modulator to carrier
		carrier = carrier * modulator;
		//Panning
		sums[0] += carrier * (1 - params[i].pan);
		sums[1] += carrier * params[i].pan;
	}
	//Optionally Send out list of envelope values
	if (x->sendEnvList)
		vocoder_envListOut(x->envList_outlet, bands, x->envList);
	//Gain adjustment
	sums[0] *= (1 / sqrt(bands));
	sums[1] *= (1 / sqrt(bands));
	//
	return sums;
}


//Biquad BandpassFilter a signal
double vocoder_biquad(struct QuadFilter *filter, t_double in)
{
	double *coefs = filter->coefs;
	double yTemp = filter->y1;
	//Biquad filter
	filter->y1 = *coefs * in + *(coefs + 1) * filter->x2 - *(coefs + 2) * filter->y1 - *(coefs + 3) * filter->y2;
	//NaN value correcting?
#ifdef DENORM_WANT_FIX
	if (IS_DENORM_NAN_DOUBLE(filter->y1))
		filter->y1 = yTemp = 0;
#endif
#ifdef DENORM_WANT_FIX
	if (IS_DENORM_NAN_DOUBLE(filter->x1))
		filter->x1 = 0;
#endif
	//Update
	filter->y2 = yTemp;
	filter->x2 = filter->x1;
	filter->x1 = in;
	//Return filtered sample value
	return filter->y1;
}


//Envelope follow a signal logrithmically
double vocoder_envelopeFollow(double in, long attack, long release, int isLog, struct BandParams *params, long frame)
{
	double *start = &params->envStart;
	double *diff = &params->envDiff;
	double *envY = &params->envY;
	long *count = &params->envCount;
	in = fabs(in);
	//Check if need to set new envelope target once every sample vector
	if (frame == 1 && in != (*start + *diff)) {
		*start = *envY;
		*diff = in - *start;
		*count = 0;
	}
	//Logrithmic ramp to new value
	double phase = (*count)++;
	phase /= (*diff > 0)? attack : release;
	if (isLog)
		phase = log10(phase * 10 + 1);
	*envY = *start + *diff * min(phase, 1);
	return *envY;
}