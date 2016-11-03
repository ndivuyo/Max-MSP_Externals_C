#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects
#include <math.h>


//Constants
#define M_PI acos(-1.0);
const int NUM_COEFS = 5;

// Struct
typedef struct _testFilter {
	t_pxobject ob;
	double x1, x2, y1, y2;
	double coefs[NUM_COEFS];
	double resonance, centerHz, bandwidth;
	int bpComputationMode;
} t_testFilter;



// Standard
void *testFilter_new(t_symbol *s, long argc, t_atom *argv);
void testFilter_free(t_testFilter *x);
void testFilter_assist(t_testFilter *x, void *b, long m, long a, char *s);
//DSP
void testFilter_dsp64(t_testFilter *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void testFilter_perform64(t_testFilter *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
//Additional
void testFilter_coefs(t_testFilter *x, t_symbol *s, long argc, t_atom *argv);
void testFilter_cutoff(t_testFilter *x, double hz);
void testFilter_q(t_testFilter *x, double q);
void testFilter_bw(t_testFilter *x, double bw);
void testFilter_bpMode(t_testFilter *x, int mode);
void testFilter_computeBPCoefs(t_testFilter *x);
t_max_err testFilter_notify(t_testFilter *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

// global class pointer variable
static t_class *testFilter_class = NULL;


//***********************************************************************************************

void ext_main(void *r)
{
	t_class *c = class_new("testFilter~", (method)testFilter_new, (method)dsp_free, (long)sizeof(t_testFilter), 0L, A_GIMME, 0);
	//
	class_addmethod(c, (method)testFilter_dsp64, "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)testFilter_assist, "assist",	A_CANT, 0);
	//
	class_addmethod(c, (method)testFilter_coefs, "coefs", A_GIMME, 0);
	class_addmethod(c, (method)testFilter_cutoff, "cutoff", A_FLOAT, 0);
	class_addmethod(c, (method)testFilter_q, "q", A_FLOAT, 0);
	class_addmethod(c, (method)testFilter_bw, "bw", A_FLOAT, 0);
	class_addmethod(c, (method)testFilter_bpMode, "bpMode", A_LONG, 0);
	class_addmethod(c, (method)testFilter_notify, "notify", A_CANT, 0);
	//
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	testFilter_class = c;
}


/*	Standard 	*******************************************************************/

//new
void *testFilter_new(t_symbol *s, long argc, t_atom *argv)
{
	t_testFilter *x = (t_testFilter *)object_alloc(testFilter_class);

	if (x) {
		dsp_setup((t_pxobject *)x, 1);
		outlet_new((t_pxobject *)x, "signal");
		//Register object (attach to itsself) so get attr notifications
		object_attach_byptr_register(x, x, CLASS_BOX);
	}
	return (x);
}


//free
void testFilter_free(t_testFilter *x){;}


//assist
void testFilter_assist(t_testFilter *x, void *b, long m, long a, char *s)
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
			sprintf(s,"(Signal) Filtered");
			break;
		}
	}
}


/*	Additional 	*******************************************************************/

//Notify
t_max_err testFilter_notify(t_testFilter *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    t_symbol *attrname;
    if (msg == gensym("attr_modified")) {   // check notification type
        attrname = (t_symbol *)object_method((t_object *)data, gensym("getname")); // ask attribute object for name
    }
    return 0;
}

//Add filter coefficient values
void testFilter_coefs(t_testFilter *x, t_symbol *s, long argc, t_atom *argv) 
{
	int size = (argc < NUM_COEFS)? argc : NUM_COEFS;
	t_atom *args = argv;
    double *coefs = x->coefs;
    for (int i = 0; i < size; i++, args++, coefs++) {
    	*coefs = atom_getfloat(args);
    }
}

//
void testFilter_cutoff(t_testFilter *x, double hz) {
	x->centerHz = hz;
	testFilter_computeBPCoefs(x);
}

//
void testFilter_bw(t_testFilter *x, double bw) {
	x->bandwidth = bw;
	testFilter_computeBPCoefs(x);
}

//
void testFilter_q(t_testFilter *x, double q) {
	x->resonance = q;
	testFilter_computeBPCoefs(x);
}

//
void testFilter_bpMode(t_testFilter *x, int mode) {
	x->bpComputationMode = mode;
	testFilter_computeBPCoefs(x);
}

//
void testFilter_computeBPCoefs(t_testFilter *x) {
	if (!x->bpComputationMode)
		x->bpComputationMode = 0;
	if (!x->resonance)
		x->resonance = 1;
	if (!x->bandwidth)
		x->bandwidth = 1;
	if (!x->centerHz)
		x->centerHz = 1000;
	//
	double w = 2 * acos(-1) * x->centerHz / 44100;
	double a, ax;
	switch (x->bpComputationMode) {
		case 0:
			a = sin(w) / 2;
			ax = a * x->resonance;
			break;
		case 1:
			a = ax = sin(w) * sinh( (log(2) / 2) * x->bandwidth * (w / sin(w)) );
			break;
	}
	//
	double a0 = 1 + a;
	double *coefs = x->coefs;
	*coefs = ax / a0;
	*(coefs + 1) = 0;
	*(coefs + 2) = -1 * ax / a0;
	*(coefs + 3) = -2 * cos(w) / a0;
	*(coefs + 4) = (1 - a) / a0;
}


/*	DSP 	*******************************************************************/


// registers a function for the signal chain in Max
void testFilter_dsp64(t_testFilter *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), x, testFilter_perform64, 0, NULL);
}


//perform
void testFilter_perform64(t_testFilter *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	t_double *in = ins[0];	
	t_double *out = outs[0];
	double yTemp;
	double y1 = x->y1;
	double y2 = x->y2;
	double x1 = x->x1;
	double x2 = x->x2;
	double *coefs = x->coefs;

	while (sampleframes--) {
		yTemp = y1;

		//IIR 1st order
		//y1 = *coefs * *in++ + *(coefs + 1) * y1;

		//FIR 1st order
		//y1 = *coefs * *in + *(coefs + 1) * x1;

		//Biquad
		y1 = *coefs * *in + *(coefs + 1) * x1 + *(coefs + 2) * x2 - *(coefs + 3) * y1 - *(coefs + 4) * y2;

		//
#ifdef DENORM_WANT_FIX
		if (IS_DENORM_NAN_DOUBLE(y1))
			y1 = yTemp = 0;
#endif
#ifdef DENORM_WANT_FIX
		if (IS_DENORM_NAN_DOUBLE(x1))
			x1 = 0;
#endif

		y2 = yTemp;
		x2 = x1;
		x1 = *in++;

		//out
		*out++ = y1;
	}
	//
	x->x1 = x1;
	x->x2 = x2;
	x->y1 = y1;
	x->y2 = y2;
}