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


//Bandpass Filter
struct BPFilter {
	double x1, x2, y1, y2;
	double coefs[NUM_COEFS];
};


//Parameters for each set of band processing (modulator and carrier)
struct BandParams {
	double freq, bw, gain, pan;
	double envY, envStart, envDiff;
	long envCount, delayCount;
	double *delayHistory;
};


//Vocoder Object Struct
typedef struct _vocoder {
	t_pxobject ob;
	double samplerate;
	struct BandParams *bandParams;	//Parameter values for each band (set of mod and carrier filterbanks)
	//Filter variables:
	int numBands;					//ATTR: “nums”
	struct BPFilter *modFB;
	struct BPFilter *carrFB;
	//Envelope Follower variables:
	long envAttack, envRelease;
	int sendEnvList;				//Attr: "envelopeDisplay"
	t_atom *envList;
	void *envList_outlet;
	//Sample Delay variables:
	long delayTime, delayCount, maxDelay;
} t_vocoder;


// Standard
void *vocoder_new(t_symbol *s, long argc, t_atom *argv);
void vocoder_free(t_vocoder *x);
void vocoder_assist(t_vocoder *x, void *b, long m, long a, char *s);
//Messages
void vocoder_freqs(t_vocoder *x, t_symbol *s, long argc, t_atom *argv);
void vocoder_bandwidths(t_vocoder *x, t_symbol *s, long argc, t_atom *argv);
void vocoder_gains(t_vocoder *x, t_symbol *s, long argc, t_atom *argv);
void vocoder_pans(t_vocoder *x, t_symbol *s, long argc, t_atom *argv);
void vocoder_envelope(t_vocoder *x, t_symbol *s, long argc, t_atom *argv);
void vocoder_outputEnvelopes(t_vocoder *x, long state);
void vocoder_delayTime(t_vocoder *x, long delayTime);
void vocoder_clear(t_vocoder *x);
t_max_err vocoder_notify(t_vocoder *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
//Internal
void vocoder_numBands(t_vocoder *x);
void vocoder_initNullParams(t_vocoder *x);
void vocoder_computeAllCoefs(t_vocoder *x);
void vocoder_computeCoefs(int bands, double samplerate, struct BPFilter *fb, struct BandParams *params);
void vocoder_phaseDelays(t_vocoder *x);
void vocoder_envListOut(void *outlet, int size, t_atom *envList);
//DSP
void vocoder_dsp64(t_vocoder *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void vocoder_perform64(t_vocoder *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
double *vocoder_vocoder(t_vocoder *x, t_double modSig, t_double carrSig, long frame);
double vocoder_bpf(struct BPFilter *filter, t_double in);
double vocoder_envelopeFollow(double in, long attack, long release, struct BandParams *params, long frame);
double vocoder_delay(double in, long delayTime, double *history, long delayCount);

// global class pointer variable
static t_class *vocoder_class = NULL;



/*  Main   	***********************************************************************************************/



//Main
void ext_main(void *r)
{
	t_class *c = class_new("db.vocoder~", (method)vocoder_new, (method)dsp_free, (long)sizeof(t_vocoder), 0L, A_GIMME, 0);
	//
	class_addmethod(c, (method)vocoder_dsp64, "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)vocoder_assist, "assist",	A_CANT, 0);
	//
	class_addmethod(c, (method)vocoder_notify, "notify", A_CANT, 0);
	class_addmethod(c, (method)vocoder_freqs, "freqs", A_GIMME, 0);
	class_addmethod(c, (method)vocoder_bandwidths, "bandwidths", A_GIMME, 0);
	class_addmethod(c, (method)vocoder_gains, "gains", A_GIMME, 0);
	class_addmethod(c, (method)vocoder_pans, "pans", A_GIMME, 0);
	class_addmethod(c, (method)vocoder_envelope, "envelope", A_GIMME, 0);
	class_addmethod(c, (method)vocoder_outputEnvelopes, "outputEnvelopes", A_DEFLONG, 0);
	class_addmethod(c, (method)vocoder_delayTime, "delay", A_DEFLONG, 0);
	class_addmethod(c, (method)vocoder_clear, "clear", A_GIMME, 0);
	//
	CLASS_ATTR_LONG(c, "num", 0, t_vocoder, numBands);
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
		//Register object (attach to itsself) so get attr notifications
		object_attach_byptr_register(x, x, CLASS_BOX);
		//Outlets
		x->envList_outlet = outlet_new((t_object *)x, NULL);
		outlet_new((t_pxobject *)x, "signal");
		outlet_new((t_pxobject *)x, "signal");
		//Initialize parameters
		x->samplerate = sys_getsr();
		x->numBands = 4;
		x->envAttack = 10;
		x->envRelease = 1000;
		x->sendEnvList = 0;
		x->maxDelay = x->samplerate;
		x->delayTime = 1;
		//Check for arguments
		for (int i = 0; i < argc; i++) {
			switch(i) {
				case 0:
					x->numBands = max( atom_getlong(argv), 1);
					break;
				case 1:
					x->envAttack = max( atom_getlong(argv), 1);
					break;
				case 2:
					x->envRelease = max( atom_getlong(argv), 1);
					break;
			}
		}
	}
	return (x);
}


//free
void vocoder_free(t_vocoder *x)
{
	free(x->delayHistory);
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
			sprintf(s,"(Signal / Messages) Modulator");
			break;
		case 1:
			sprintf(s,"(Signal) Carrier");
			break;
		}
	}
	else {
		switch (a) {
		case 0:
			sprintf(s,"(Signal) Vocoder Left");
			break;
		case 1:
			sprintf(s,"(Signal) Vocoder Right");
			break;
		}
	}
}



/*	Messages 	*******************************************************************/



//Update Center frequencies for filterbanks
void vocoder_freqs(t_vocoder *x, t_symbol *s, long argc, t_atom *argv)
{
	int size = (argc < x->numBands)? argc : x->numBands;
	t_atom *args = argv;
    struct BandParams *bands = x->bandParams;
    for (int i = 0; i < size; i++, args++) {
    	bands[i].freq = max( min(atom_getfloat(args), x->samplerate / 2), 20 );
    }
    vocoder_computeAllCoefs(x);
}


//Update bandwidths for filterbanks
void vocoder_bandwidths(t_vocoder *x, t_symbol *s, long argc, t_atom *argv)
{
	int size = (argc < x->numBands)? argc : x->numBands;
	t_atom *args = argv;
    struct BandParams *bands = x->bandParams;
    //If only one argument, then change all values to it
    if (size == 1) {
    	for (int i = 0; i < x->numBands; i++) {
	    	bands[i].bw = max(atom_getfloat(args), 0.0001);
	    }
    } else {
	    for (int i = 0; i < size; i++, args++) {
	    	bands[i].bw = max(atom_getfloat(args), 0.0001);
	    }
	}
    vocoder_computeAllCoefs(x);
}


//Update gains for filterbands
void vocoder_gains(t_vocoder *x, t_symbol *s, long argc, t_atom *argv)
{
	int size = (argc < x->numBands)? argc : x->numBands;
	t_atom *args = argv;
    struct BandParams *bands = x->bandParams;
    //If only one argument, then change all values to it
    if (size == 1) {
    	for (int i = 0; i < x->numBands; i++) {
    		bands[i].gain = atom_getfloat(args);
    	}
    } else {
	    for (int i = 0; i < size; i++, args++) {
	    	bands[i].gain = atom_getfloat(args);
	    }
	}
}


//Update pans for filterbands
void vocoder_pans(t_vocoder *x, t_symbol *s, long argc, t_atom *argv)
{
	int size = (argc < x->numBands)? argc : x->numBands;
	t_atom *args = argv;
    struct BandParams *bands = x->bandParams;
    for (int i = 0; i < size; i++, args++) {
    	bands[i].pan = atom_getfloat(args);
    }
}


//Set attack and release times in samples for envelope follower
void vocoder_envelope(t_vocoder *x, t_symbol *s, long argc, t_atom *argv)
{
	x->envAttack = max( atom_getlong(argv++), 1);
	x->envRelease = max( atom_getlong(argv), 1);
}


//Booelean whether or not to send out envelope values list
void vocoder_outputEnvelopes(t_vocoder *x, long state) 
{
	x->sendEnvList = max( min(state, 1), 0);
}


//Set delay time in samples
void vocoder_delayTime(t_vocoder *x, long delayTime)
{
	x->delayTime = max( min(delayTime, x->maxDelay - 1), 0) + 1;
	vocoder_phaseDelays(x);
}


//Clear bandpass filter delay histories
void vocoder_clear(t_vocoder *x)
{

}


//
t_max_err vocoder_notify(t_vocoder *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	t_symbol *attrname;
    if (msg == gensym("attr_modified")) {   // Check notification type
        attrname = (t_symbol *)object_method((t_object *)data, gensym("getname")); // ask attribute object for name
        //Number of filters changed
        if (attrname == gensym("num")) {
			vocoder_numBands(x);
        }
        //
    }
    return 0;
}



/*	Internal 	*******************************************************************/



//Change the number of bands
void vocoder_numBands(t_vocoder *x)
{
	//Value check
	x->numBands = max(x->numBands, 1);
	double size = x->numBands;
	//Initialize or resize
	if (x->bandParams)
		x->bandParams = (struct BandParams *)sysmem_resizeptr((struct BandParams *)x->bandParams, size * sizeof(struct BandParams));
	else
		x->bandParams = (struct BandParams *)sysmem_newptr(size * sizeof(struct BandParams));
	if (x->modFB)
		x->modFB = (struct BPFilter *)sysmem_resizeptr((struct BPFilter *)x->modFB, size * sizeof(struct BPFilter));
	else
		x->modFB = (struct BPFilter *)sysmem_newptr(size * sizeof(struct BPFilter));
	if (x->carrFB)
		x->carrFB = (struct BPFilter *)sysmem_resizeptr((struct BPFilter *)x->carrFB, size * sizeof(struct BPFilter));
	else
		x->carrFB = (struct BPFilter *)sysmem_newptr(size * sizeof(struct BPFilter));
	if (x->envList)
		x->envList = (t_atom *)sysmem_resizeptr((t_atom *)x->envList, size * sizeof(t_atom));
	else
		x->envList = (t_atom *)sysmem_newptr(size * sizeof(t_atom));
	//Initialize parameter values that are empty
	vocoder_initNullParams(x);
}


//Initialize null parameters
void vocoder_initNullParams(t_vocoder *x)
{
	int size = x->numBands;
    struct BandParams *params = x->bandParams;
    for (int i = 0; i < size; i++) {
    	//Determine if value previously set, if not use previous filter's parameters
    	if (!params[i].freq)
    		params[i].freq = !i? 1000 : params[i - 1].freq;
    	if (!params[i].gain)
    		params[i].gain = !i? 1 : params[i - 1].gain;
    	if (!params[i].pan)
    		params[i].pan = !i? 0.5 : params[i - 1].pan;
    	if (!params[i].bw)
    		params[i].bw = !i? 1 : params[i - 1].bw;
    	if (!params[i].envY)
    		params[i].envY = 0;
    	if (!params[i].envStart) {
    		params[i].envStart = 0;
    		params[i].envCount = 0;
    		params[i].envDiff = 0;
    	}
    	if (!params[i].delayHistory)
    		params[i].delayHistory = (double *)sysmem_newptr(x->maxDelay * sizeof(double));
    }
    //Compute filter coefficients
    vocoder_computeAllCoefs(x);
    //Phase individual band delays
    vocoder_phaseDelays(x);
}


//Convert coefficients for all filterbanks
void vocoder_computeAllCoefs(t_vocoder *x)
{
	vocoder_computeCoefs(x->numBands, x->samplerate, x->modFB, x->bandParams);
	vocoder_computeCoefs(x->numBands, x->samplerate, x->carrFB, x->bandParams);
}


//Convert center frequency and bandwidth to coefficients for each filter in a filterbank
void vocoder_computeCoefs(int bands, double samplerate, struct BPFilter *fb, struct BandParams *params)
{
	for (int i = 0; i < bands; i++) {
		//Conversions
		double w = 2 * acos(-1) * params[i].freq / samplerate;
		double a = sin(w) * sinh( (log(2) / 2) * params[i].bw * (w / sin(w)) );
		double a0 = 1 + a;
		double *coefs = fb[i].coefs;
		*coefs = a / a0;
		*(coefs + 1) = -1 * a / a0;
		*(coefs + 2) = -2 * cos(w) / a0;
		*(coefs + 3) = (1 - a) / a0;
	}
}


//Phase delays for bands based on index and delay time
void vocoder_phaseDelays(t_vocoder *x) {
	int size = x->numBands;
	struct BandParams *params = x->bandParams;
	for (int i = 0; i < size; i++) {
		params[i].delayTime = (i + 1) / size * x->delayTime;
	}
	x->delayCount = 0;
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

	//Iterate through samples
	while (sampleframes--) {
		//Apply vocoder
		outX = vocoder_vocoder(x, *modSig++, *carrSig++, sampleframes);
		//Output samples
		*outL++ = *outX++;
		*outR++ = *outX;
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
		//BPFilter carrier and modulator
		double carrier = vocoder_bpf(&x->carrFB[i], carrSig);
		double modulator = vocoder_bpf(&x->modFB[i], modSig);
		//Get a delayed modulator sample
		modulator = vocoder_delay(modulator, params[i].delayTime, params[i].delayHistory, x->delayCount);
		//Envelope follow modulator
		modulator = vocoder_envelopeFollow(modulator, x->envAttack, x->envRelease, &params[i], frame);
		if (x->sendEnvList)
			atom_setfloat(&x->envList[i], modulator);
		//Apply modulator to carrier with gain
		carrier = carrier * modulator * params[i].gain;
		//Panning
		sums[0] += carrier * (1 - params[i].pan);
		sums[1] += carrier * params[i].pan;
	}
	//Increase delay counting
	x->delayCount++;
	//Optionally Send out list of envelope values
	if (x->sendEnvList)
		vocoder_envListOut(x->envList_outlet, bands, x->envList);
	//Gain adjustment
	sums[0] *= (1 / sqrt(bands));
	sums[1] *= (1 / sqrt(bands));
	return sums;
}


//Bandpass Filter a signal
double vocoder_bpf(struct BPFilter *filter, t_double in)
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
double vocoder_envelopeFollow(double in, long attack, long release, struct BandParams *params, long frame)
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
	phase = log10(phase * 10 + 1);
	*envY = *start + *diff * min(phase, 1);
	return *envY;
}


//Store a sample for delay
double vocoder_delay(double in, long delayTime, double *history, long delayCount)
{
	history[delayCount % delayTime] = in;
	double grab = history[delayCount % delayTime + 1];
	return (!grab)? 0 : grab;
}