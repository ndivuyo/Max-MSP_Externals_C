/*
** Dillon Bastan, 2016
** Envelope follower on incoming signal
*/


#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects
#include <math.h>			// math library


//Object Struct
typedef struct _envfollow {
	t_pxobject ob;
	long attack, release, samplecount;
	double y, start, xDiff;
	double samplerate;
	int isEnvelope;
} t_envfollow;



// Standard
void *envfollow_new(t_symbol *s, long argc, t_atom *argv);
void envfollow_free(t_envfollow *x);
void envfollow_assist(t_envfollow *x, void *b, long m, long a, char *s);
//DSP
void envfollow_dsp64(t_envfollow *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void envfollow_perform64(t_envfollow *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
//Additional
void envfollow_times(t_envfollow *x, t_symbol *s, long argc, t_atom *argv);
void envfollow_setTarget(t_envfollow *x, double target);
double envfollow_envfollow(t_envfollow *x, t_double in, long attack, long release, long frame);


// global class pointer variable
static t_class *envfollow_class = NULL;



/*  Main   	***********************************************************************************************/



//Main
void ext_main(void *r)
{
	t_class *c = class_new("db.envfollow~", (method)envfollow_new, (method)dsp_free, (long)sizeof(t_envfollow), 0L, A_GIMME, 0);
	//
	class_addmethod(c, (method)envfollow_dsp64, "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)envfollow_assist, "assist",	A_CANT, 0);
	//
	class_addmethod(c, (method)envfollow_times, "times", A_GIMME, 0);
	//
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	envfollow_class = c;
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
void *envfollow_new(t_symbol *s, long argc, t_atom *argv)
{
	t_envfollow *x = (t_envfollow *)object_alloc(envfollow_class);
	if (x) {
		//
		dsp_setup((t_pxobject *)x, 1);
		outlet_new((t_pxobject *)x, "signal");
		//Get samplerate
		x->samplerate = sys_getsr();
		//Check for arguments
		if (atom_getlong(argv))
			x->attack = max( atom_getfloat(argv), 1);
		else
			x->attack = 100;
		if (atom_getlong(++argv))
			x->release = max( atom_getfloat(argv), 1);
		else
			x->release = 100;
		//Initialize Values
		x->isEnvelope = x->start = x->y = x->xDiff = 0;
	}
	return (x);
}


//free
void envfollow_free(t_envfollow *x){;}


//Assist
void envfollow_assist(t_envfollow *x, void *b, long m, long a, char *s)
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
			sprintf(s,"(Signal) Processed");
			break;
		}
	}
}



/*	Additional 	*******************************************************************/



//Set the attack and release times
void envfollow_times(t_envfollow *x, t_symbol *s, long argc, t_atom *argv)
{
	x->attack = max( atom_getlong(argv++), 1);
	x->release = max( atom_getlong(argv), 1);
}


//
void envfollow_setTarget(t_envfollow *x, double target)
{
	x->start = x->y;
	x->xDiff = target - x->start;
	x->samplecount = 0;
	x->isEnvelope = 1;
}



/*	DSP 	*******************************************************************/



//Registers a function for the signal chain in Max
void envfollow_dsp64(t_envfollow *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), x, envfollow_perform64, 0, NULL);
}


//Perform
void envfollow_perform64(t_envfollow *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	//Access in and out samples
	t_double *in = ins[0];	
	t_double *out = outs[0];

	while (sampleframes--) {
		*out++ = envfollow_envfollow(x, *in++, x->attack, x->release, sampleframes);
	}
}


//Envelope following a signal
double envfollow_envfollow(t_envfollow *x, t_double in, long attack, long release, long frame) 
{
	in = fabs(in);
	//Check for new value once every vector of samples
	if (frame == 1 && in != (x->start + x->xDiff))
		envfollow_setTarget(x, in);
	if (x->isEnvelope) {
		double phase = x->samplecount++;
		phase /= (x->xDiff > 0)? attack : release;
		phase = log10(phase * 10 + 1);
		x->y = x->start + x->xDiff * phase;
		if (phase >= 1)
			x->isEnvelope = 0;
	} else
		x->y = in;
	return x->y;
}