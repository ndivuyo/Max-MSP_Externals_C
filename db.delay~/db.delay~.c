/*
** Dillon Bastan, 2016
** Envelope follower on incoming signal
*/


#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects
#include <math.h>			// math library


//Object Struct
typedef struct _delay {
	t_pxobject ob;
	double *history;
	long delayTime, samplecount;
} t_delay;



// Standard
void *delay_new(t_symbol *s, long argc, t_atom *argv);
void delay_free(t_delay *x);
void delay_assist(t_delay *x, void *b, long m, long a, char *s);
//DSP
void delay_dsp64(t_delay *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void delay_perform64(t_delay *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
//Additional
void delay_time(t_delay *x, long delayTime);
double delay_delay(t_delay *x, t_double in, long delayTime, double *history);


// global class pointer variable
static t_class *delay_class = NULL;



/*  Main   	***********************************************************************************************/



//Main
void ext_main(void *r)
{
	t_class *c = class_new("db.delay~", (method)delay_new, (method)dsp_free, (long)sizeof(t_delay), 0L, A_GIMME, 0);
	//
	class_addmethod(c, (method)delay_dsp64, "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)delay_assist, "assist",	A_CANT, 0);
	//
	class_addmethod(c, (method)delay_time, "time", A_LONG, 0);
	//
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	delay_class = c;
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
void *delay_new(t_symbol *s, long argc, t_atom *argv)
{
	t_delay *x = (t_delay *)object_alloc(delay_class);
	if (x) {
		//
		dsp_setup((t_pxobject *)x, 1);
		outlet_new((t_pxobject *)x, "signal");
		//Check for arguments
		if (atom_getlong(argv))
			x->delayTime = max( atom_getfloat(argv), 0);
		else
			x->delayTime = 0;
		//Initialize Values
		x->history = (double *)malloc(x->delayTime * sizeof(double));
		x->samplecount = 0;
	}
	return (x);
}


//free
void delay_free(t_delay *x)
{
	free(x->history);
}


//Assist
void delay_assist(t_delay *x, void *b, long m, long a, char *s)
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
void delay_time(t_delay *x, long delayTime)
{
	x->delayTime = delayTime;
	x->history = (double *)realloc(x->history, x->delayTime * sizeof(double));
	x->samplecount = 0;
}



/*	DSP 	*******************************************************************/



//Registers a function for the signal chain in Max
void delay_dsp64(t_delay *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), x, delay_perform64, 0, NULL);
}


//Perform
void delay_perform64(t_delay *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	//Access in and out samples
	t_double *in = ins[0];	
	t_double *out = outs[0];

	while (sampleframes--) {
		*out++ = delay_delay(x, *in++, x->delayTime, x->history);
	}
}


//TRY USING BUFFER?
double delay_delay(t_delay *x, t_double in, long delayTime, double *history)
{
	history[(x->samplecount++)%delayTime] = in;
	return history[x->samplecount%delayTime];
}

