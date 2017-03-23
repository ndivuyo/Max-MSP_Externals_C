/*
** Dillon Bastan, 2016
** Morphing LFO Bank
*/



/*  Setup  	***********************************************************************************************/



//includes
#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"


//Constants
const int TABLE_SIZE = 512;				//Table size of waveforms
const int RANDOM_PERCISION = 100;		//Noise percision
const short NUM_WAVEFORMS = 5;			//Number of available oscillators to choose from


// NOTE: To add more waveform simply increase NUM_WAVEFORMS and add your equation in the switch statement in lfOsc_getOscPhase()


//Object Struct
typedef struct _lfOsc {
	t_pxobject ob;
	double samplerate;
	double phaseDelta;					//Amount to increment phase index by
	double phaseIndex;					//Wavetable index
	short connected;					//Used to determine if signal is connected to inlet
	short blendWaveform;				//Mode to blend waveforms or not
	short lenOsc;						//Amount of waveforms in order
	short *waveformOrder;				//Order of waveforms, can be any length
	double waveform;					//Current waveform based on order
	double sineTable[TABLE_SIZE];		//Lookup table for sine wave 
	double triangleTable[TABLE_SIZE];	//Lookup table for triangle wave 
	double randVal;						//sah random value for noise
	short sahTrig;						//sah trigger for noise
} t_lfOsc;



// Standard
void *lfOsc_new(t_symbol *s, long argc, t_atom *argv);
void lfOsc_free(t_lfOsc *x);
void lfOsc_assist(t_lfOsc *x, void *b, long m, long a, char *s);
//DSP
void lfOsc_dsp64(t_lfOsc *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void lfOsc_perform64(t_lfOsc *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
double lfOsc_getOscPhase(t_lfOsc *x, short waveform, int phaseIndex);
double lfOsc_osc(t_lfOsc *x);
double lfOsc_oscBlend(t_lfOsc *x);
//Messages
void lfOsc_float(t_lfOsc *x, double freq);
void lfOsc_int(t_lfOsc *x, int freq);
void lfOsc_ft1(t_lfOsc *x, double phase);
void lfOsc_blendWaveform(t_lfOsc *x, short state);
void lfOsc_waveform(t_lfOsc *x, double waveform);
void lfOsc_order(t_lfOsc *x, t_symbol *s, long argc, t_atom *argv);
//Internal
void lfOsc_createWaveformTables(t_lfOsc *x);
double lfOsc_getPhaseDelta(t_lfOsc *x, double freq);



// global class pointer variable
static t_class *lfOsc_class = NULL;



/*********************************************************************************************/



void ext_main(void *r)
{
	t_class *c = class_new("db.lfOsc~", (method)lfOsc_new, (method)lfOsc_free, (long)sizeof(t_lfOsc), 0L, A_GIMME, 0);
	//
	class_addmethod(c, (method)lfOsc_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)lfOsc_assist, "assist", A_CANT, 0);
	//
	class_addmethod(c, (method)lfOsc_float, "float", A_FLOAT, 0);
	class_addmethod(c, (method)lfOsc_int, "int", A_LONG, 0);
	class_addmethod(c, (method)lfOsc_ft1, "ft1", A_FLOAT, 0);
	class_addmethod(c, (method)lfOsc_blendWaveform, "blend", A_LONG, 0);
	class_addmethod(c, (method)lfOsc_waveform, "waveform", A_FLOAT, 0);
	class_addmethod(c, (method)lfOsc_order, "order", A_GIMME, 0);
	//
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	lfOsc_class = c;
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
void *lfOsc_new(t_symbol *s, long argc, t_atom *argv)
{
	t_lfOsc *x = (t_lfOsc *)object_alloc(lfOsc_class);
	//
	if (x) {
		//Setup In/out/dsp
		floatin(x, 1);
		dsp_setup((t_pxobject *)x, 1);
		outlet_new((t_pxobject *)x, "signal");
		//InitValues and args
		x->samplerate = sys_getsr();
		x->lenOsc = NUM_WAVEFORMS;
		x->waveformOrder = (short *)malloc(x->lenOsc * sizeof(short));
		for (short i = 0; i < x->lenOsc; i++) {
			x->waveformOrder[i] = i;
		}
		x->blendWaveform = 0;
		atom_arg_getdouble(&x->waveform, 1, argc, argv);
		x->waveform = min( max(x->waveform, 0), x->lenOsc - 1);
		atom_arg_getdouble(&x->phaseDelta, 0, argc, argv);
		x->phaseDelta = lfOsc_getPhaseDelta(x, x->phaseDelta);
		atom_arg_getdouble(&x->phaseIndex, 2, argc, argv);
		x->phaseIndex = min( max( x->phaseIndex, -1), 1) * (double)TABLE_SIZE;
		srand(time(NULL));
		x->randVal = 0;
		x->sahTrig = 0;
		lfOsc_createWaveformTables(x);
	}
	return (x);
}


//free
void lfOsc_free(t_lfOsc *x)
{
	dsp_free((t_pxobject *)x);
	free(x->waveformOrder);
}


//assist
void lfOsc_assist(t_lfOsc *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) {
		switch (a) {
		case 0:
			sprintf(s,"(float/signal) Fundamental Frequency");
			break;
		case 1:
			sprintf(s,"(float) Set Phase");
		}
	}
	else {
		switch (a) {
		case 0:
			sprintf(s,"(signal) LFO Signal");
		}
	}
}



/*	Messages 	*******************************************************************/



//set the fundamental
void lfOsc_float(t_lfOsc *x, double freq) 
{
	x->phaseDelta = lfOsc_getPhaseDelta(x, freq);
}


//set the fundamental
void lfOsc_int(t_lfOsc *x, int freq) 
{
	x->phaseDelta = lfOsc_getPhaseDelta(x, freq);
}



//Set the phase
void lfOsc_ft1(t_lfOsc *x, double phase)
{
	phase = min( max( fabs(phase), 0), 1);
	x->phaseIndex = phase * (double)TABLE_SIZE;
}


//Whether blending waveforms or keeping them individual
void lfOsc_blendWaveform(t_lfOsc *x, short state)
{
	x->blendWaveform = min( max(state, 0), 1);
}


//Set waveform index/blend
void lfOsc_waveform(t_lfOsc *x, double waveform)
{
	x->waveform = min( max(waveform, 0), x->lenOsc - 1);
}


//Set the order of the waveforms according to their index
void lfOsc_order(t_lfOsc *x, t_symbol *s, long argc, t_atom *argv)
{
	if (argc < 1)
		return NULL;
	//Resize order based on new list
	x->lenOsc = argc;
	x->waveformOrder = (short *)realloc(x->waveformOrder, argc * sizeof(short));
    for (int i = 0; i < argc; i++, argv++)
    	x->waveformOrder[i] = min( max( atom_getlong(argv), 0), NUM_WAVEFORMS - 1);
    //Adjust waveform to new range
    x->waveform = min( max(x->waveform, 0), x->lenOsc - 1);
}



/*	Internal 	*******************************************************************/



//Create waveform tables for oscillators that are more efficient as lookup tables
void lfOsc_createWaveformTables(t_lfOsc *x)
{
	for (int i = 0; i < TABLE_SIZE; i++) {
		//Sine
		x->sineTable[i] = sin( (i / (double)TABLE_SIZE) * PI * 2) / 2 + 0.5;
		//Tri
		x->triangleTable[i] = 1 - fabs( i / (double)TABLE_SIZE * 2 - 1);
	}
}


//Get amount to add to phase index during each sampleframe
double lfOsc_getPhaseDelta(t_lfOsc *x, double freq)
{
	return (double)TABLE_SIZE * freq / x->samplerate;
}



/*	DSP 	*******************************************************************/



//Get oscillator phase based on waveform tables
double lfOsc_getOscPhase(t_lfOsc *x, short waveform, int phaseIndex) 
{
	double phase;
	//Allow for bipolar
	phaseIndex = (phaseIndex < 0)? TABLE_SIZE + phaseIndex : phaseIndex;
	//Return phase based on selected waveform
	switch (waveform) {
		case 0:
			phase = x->sineTable[phaseIndex];
			break;
		case 1:	
			phase = x->triangleTable[phaseIndex];
			break;
		case 2:	
			phase = phaseIndex / (double)TABLE_SIZE;
			break;
		case 3:	
			phase = (phaseIndex < TABLE_SIZE / 2)? 0 : 1;
			break;
		case 4:	
			//sah random value for noise
			if (phaseIndex >= TABLE_SIZE / 2 && !x->sahTrig) {
				x->randVal = (double)(rand() % RANDOM_PERCISION) / (double)RANDOM_PERCISION;
				x->sahTrig = 1;
			} else if (phaseIndex < TABLE_SIZE / 2 && x->sahTrig == 1) {
				x->sahTrig = 0;
			}
			phase = x->randVal;
			break;
	}
	return phase;
}


//Implement oscillator without blending
double lfOsc_osc(t_lfOsc *x) 
{
	short index = (short)x->waveform;
	return lfOsc_getOscPhase(x, x->waveformOrder[index], (int)x->phaseIndex);
}


//Implement oscillator with blending
double lfOsc_oscBlend(t_lfOsc *x) 
{
	//Array for oscillators
	double *oscBank = (double *)malloc(x->lenOsc * sizeof(double));
	double oscPhase;
	//Assign phase values for waveforms
	for (int i = 0; i < x->lenOsc; i++) {
		oscBank[i] = lfOsc_getOscPhase(x, x->waveformOrder[i], (int)x->phaseIndex);
	}
	//blend according to waveform
	double ratio = 1 / (double)x->lenOsc;
	double sum = 0;
	for (int i = 0; i < x->lenOsc; i++) {
		double a = fabs(x->waveform - i);
		a = 1 - min( max(a, 0), 1);
		sum += oscBank[i] * a;
	}
	free(oscBank);
	return sum;
}


//Register a function for the signal chain in Max
void lfOsc_dsp64(t_lfOsc *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	//Keep track of count to know if signal is connected to inlet
	x->connected = count[0];
	object_method(dsp64, gensym("dsp_add64"), x, lfOsc_perform64, 0, NULL);
}


//perform
void lfOsc_perform64(t_lfOsc *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    double *out = outs[0];
    //Choose whether to increment phase index by signal value or stored value
    double delta = x->connected ? lfOsc_getPhaseDelta(x, ins[0][0]) : x->phaseDelta;
    //
	while (sampleframes--) {
		//output oscillator with blending or static
		*out++ = (x->blendWaveform)? lfOsc_oscBlend(x) : lfOsc_osc(x);
		//Increment phase Index
		x->phaseIndex += delta;
		if ( fabs(x->phaseIndex) >= TABLE_SIZE )
			x->phaseIndex = 0;
	}
}