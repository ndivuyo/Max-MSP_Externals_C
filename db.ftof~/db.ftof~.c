#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects


// Struct
typedef struct _ftof {
	t_pxobject ob;
} t_ftof;



// Standard
void *ftof_new(t_symbol *s, long argc, t_atom *argv);
void ftof_assist(t_ftof *x, void *b, long m, long a, char *s);
//DSP
void ftof_dsp64(t_ftof *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ftof_perform64(t_ftof *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
double ftof_ftof(double in);

// global class pointer variable
static t_class *ftof_class = NULL;


//***********************************************************************************************

void ext_main(void *r)
{
	t_class *c = class_new("db.ftof~", (method)ftof_new, (method)dsp_free, (long)sizeof(t_ftof), 0L, A_GIMME, 0);

	class_addmethod(c, (method)ftof_dsp64, "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)ftof_assist, "assist",	A_CANT, 0);

	class_dspinit(c);
	class_register(CLASS_BOX, c);
	ftof_class = c;
}


/*	Standard 	*******************************************************************/

//new
void *ftof_new(t_symbol *s, long argc, t_atom *argv)
{
	t_ftof *x = (t_ftof *)object_alloc(ftof_class);

	if (x) {
		dsp_setup((t_pxobject *)x, 1);
		outlet_new((t_pxobject *)x, "signal");
	}
	return (x);
}


//assist
void ftof_assist(t_ftof *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) {
		switch (a) {
		case 0:
			sprintf(s,"(Signal) Input 0-1");
		}
	}
	else {
		switch (a) {
		case 0:
			sprintf(s,"(Signal) Output Cutoff Frequency");
		}
	}
}


/*	DSP 	*******************************************************************/


// registers a function for the signal chain in Max
void ftof_dsp64(t_ftof *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), x, ftof_perform64, 0, NULL);
}


//perform
void ftof_perform64(t_ftof *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	double *in = ins[0];	
	double *out = outs[0];

	while (sampleframes--) {
		*out++ = ftof_ftof(*in++);
	}
}


//ftof
double ftof_ftof(double in) {
	in = (in * 119.589417 + 15.48682 - 69) / 12;
	in = pow(2, in) * 440;
	return in;
}
