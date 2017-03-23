#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects


// Struct
typedef struct _ftof {
	t_object ob;
	void *f_outlet;
} t_ftof;



// Standard
void *ftof_new(t_symbol *s, long argc, t_atom *argv);
void ftof_free(t_ftof *x);
void ftof_assist(t_ftof *x, void *b, long m, long a, char *s);
//Add
void ftof_float(t_ftof *x, double in);


// global class pointer variable
static t_class *ftof_class = NULL;


//***********************************************************************************************

void ext_main(void *r)
{
	t_class *c = class_new("db.ftof", (method)ftof_new, (method)ftof_free, (long)sizeof(t_ftof), 0L, A_GIMME, 0);
	class_addmethod(c, (method)ftof_assist, "assist",	A_CANT, 0);
	class_addmethod(c, (method)ftof_float, "float",	A_FLOAT, 0);

	class_register(CLASS_BOX, c);
	ftof_class = c;
}


/*	Standard 	*******************************************************************/

//new
void *ftof_new(t_symbol *s, long argc, t_atom *argv)
{
	t_ftof *x = (t_ftof *)object_alloc(ftof_class);

	if (x) {
		x->f_outlet = floatout((t_object *)x);
	}
	return (x);
}


//free
void ftof_free(t_ftof *x){;}


//assist
void ftof_assist(t_ftof *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) {
		switch (a) {
		case 0:
			sprintf(s,"(Float) Input 0-1");
		}
	}
	else {
		switch (a) {
		case 0:
			sprintf(s,"(Float) Output Cutoff Frequency");
		}
	}
}



//ftof
void ftof_float(t_ftof *x, double in) {
	in = (in * 119.589417 + 15.48682 - 69) / 12;
	in = pow(2, in) * 440;
	outlet_float(x->f_outlet, in);
}
