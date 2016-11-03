#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects

//Constants
const long MAX_SIZE = 5;

// Struct
typedef struct _testIt {
	t_pxobject ob;
	double attr1;
	long size;
	long params[MAX_SIZE];
} t_testIt;



// Standard
void *testIt_new(t_symbol *s, long argc, t_atom *argv);
void testIt_free(t_testIt *x);
void testIt_assist(t_testIt *x, void *b, long m, long a, char *s);
//DSP
void testIt_dsp64(t_testIt *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void testIt_perform64(t_testIt *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
//Additional
void testIt_printargs(t_testIt *x, t_symbol *s, long argc, t_atom *argv);
void testIt_paramList(t_testIt *x, t_symbol *s, long argc, t_atom *argv);
void testIt_printparams(t_testIt *x);
t_max_err testIt_notify(t_testIt *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

// global class pointer variable
static t_class *testIt_class = NULL;


//***********************************************************************************************

void ext_main(void *r)
{
	t_class *c = class_new("testIt~", (method)testIt_new, (method)dsp_free, (long)sizeof(t_testIt), 0L, A_GIMME, 0);

	class_addmethod(c, (method)testIt_dsp64, "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)testIt_assist, "assist",	A_CANT, 0);
	class_addmethod(c, (method)testIt_printargs, "printargs", A_GIMME, 0);
	class_addmethod(c, (method)testIt_notify, "notify", A_CANT, 0);
	class_addmethod(c, (method)testIt_paramList, "paramList", A_GIMME, 0);

	CLASS_ATTR_DOUBLE(c, "attr1", 0, t_testIt, attr1);
	CLASS_ATTR_LONG(c, "size", 0, t_testIt, size);

	class_dspinit(c);
	class_register(CLASS_BOX, c);
	testIt_class = c;
}


/*	Standard 	*******************************************************************/

//new
void *testIt_new(t_symbol *s, long argc, t_atom *argv)
{
	t_testIt *x = (t_testIt *)object_alloc(testIt_class);

	if (x) {
		dsp_setup((t_pxobject *)x, 2);
		outlet_new((t_pxobject *)x, "signal");
		outlet_new((t_pxobject *)x, "signal");
		//Register object (attach to itsself) so get attr notifications
		object_attach_byptr_register(x, x, CLASS_BOX);
	}
	return (x);
}


//free
void testIt_free(t_testIt *x){;}


//assist
void testIt_assist(t_testIt *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) {
		switch (a) {
		case 0:
			sprintf(s,"(Signal) Left Sig");
			break;
		case 1:
			sprintf(s,"(Signal) Right Sig");
			break;
		}
	}
	else {
		switch (a) {
		case 0:
			sprintf(s,"(Signal) Multiplication Result");
			break;
		case 1:
			sprintf(s,"(Signal) Something Else");
			break;
		}
	}
}


/*	Additional 	*******************************************************************/

//Print args
void testIt_printargs(t_testIt *x, t_symbol *s, long argc, t_atom *argv)
{
    long i;
    t_atom *ap = argv;

    post("message selector is %s",s->s_name);
    post("there are %ld arguments",argc);

    for (i = 0; i < argc; i++, ap++) {
        switch (atom_gettype(ap)) {
            case A_LONG:
                post("%ld: %ld",i+1,atom_getlong(ap));
                break;
            case A_FLOAT:
                post("%ld: %.2f",i+1,atom_getfloat(ap));
                break;
            case A_SYM:
                post("%ld: %s",i+1, atom_getsym(ap)->s_name);
                break;
            default:
                post("%ld: unknown atom type (%ld)", i+1, atom_gettype(ap));
                break;
        }
    }
}



//Notify
t_max_err testIt_notify(t_testIt *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    t_symbol *attrname;
    if (msg == gensym("attr_modified")) {   // check notification type
        attrname = (t_symbol *)object_method((t_object *)data, gensym("getname")); // ask attribute object for name
        //Size attr changed
        if (attrname == gensym("size")) {
        	if (x->size > MAX_SIZE)
        		x->size = MAX_SIZE;
        }
        //
    }
    return 0;
}


//Add values to params
void testIt_paramList(t_testIt *x, t_symbol *s, long argc, t_atom *argv) 
{
	long size = (argc < MAX_SIZE)? argc : MAX_SIZE;
	t_atom *args = argv;
    long *p = x->params;
    for (long i = 0; i < size; i++, args++, p++) {
    	*p = atom_getlong(args);
    }
    testIt_printparams(x);
}



//print params
void testIt_printparams(t_testIt *x) 
{
	long *p = x->params;
	long size = x->size;
    for (long i = 0; i < size; i++, p++) {
    	post("%ld", *p );
    }
}


/*	DSP 	*******************************************************************/


// registers a function for the signal chain in Max
void testIt_dsp64(t_testIt *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), x, testIt_perform64, 0, NULL);
}


//perform
void testIt_perform64(t_testIt *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	t_double *in1 = ins[0];	
	t_double *in2 = ins[1];
	t_double *outL = outs[0];
	t_double *outR = outs[1];
	long *offset = x->params;

	int n = sampleframes;

	while (n--) {
		*outL++ = *in1++ / *in2++ + *(offset + 1);
		*outR++ = *offset;
	}
}

