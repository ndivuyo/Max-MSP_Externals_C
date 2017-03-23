/*
*	Dillon Bastan, 2016
*	Receive signal from db.send~
*/


#include "ext.h"		
#include "ext_obex.h"
#include "z_dsp.h"


// Struct
typedef struct _receive {
	t_pxobject ob;
	void *obReg;
	t_symbol *namespaceS;
	t_symbol *namespaceR;
	t_symbol *id;
	t_symbol *alias;
	double val;
} t_receive;



// Standard
void *receive_new(t_symbol *s, long argc, t_atom *argv);
void receive_assist(t_receive *x, void *b, long m, long a, char *s);
void receive_free(t_receive *x);
//DSP
void receive_dsp64(t_receive *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void receive_perform64(t_receive *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void receive_receive(t_receive *x, double in);
//Messages
void receive_set(t_receive *x, t_symbol *name);
void receive_out(t_receive *x, double v);
//Internal
t_symbol *receive_getUniqueID(t_receive *x);
void receive_updateSends(t_symbol *namespaceS);
t_max_err receive_getAlias(t_receive *x, double arg, t_atom *av);



// global class pointer variable
static t_class *receive_class = NULL;



//***********************************************************************************************



void ext_main(void *r)
{
	t_class *c = class_new("db.receive~", (method)receive_new, (method)receive_free, (long)sizeof(t_receive), 0L, A_GIMME, 0);

	class_addmethod(c, (method)receive_dsp64, "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)receive_assist, "assist", A_CANT, 0);
	class_addmethod(c, (method)receive_set, "set", A_SYM, 0);
	class_addmethod(c, (method)receive_out, "out", A_FLOAT, 0);
	class_addmethod(c, (method)receive_getAlias, "getAlias", A_FLOAT, 0);
	//
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	receive_class = c;
}


/*	Standard 	*******************************************************************/



//new
void *receive_new(t_symbol *s, long argc, t_atom *argv)
{
	t_receive *x = (t_receive *)object_alloc(receive_class);

	if (x) {
		dsp_setup((t_pxobject *)x, 1);
		outlet_new((t_pxobject *)x, "signal");
		//Init and args
		srand(time(NULL));
		x->namespaceS = gensym("db.send~");
		x->namespaceR = gensym("db.receive~");
		x->id = receive_getUniqueID(x);
		post("receive ID: %s\n", x->id->s_name);
		x->obReg = object_register(x->namespaceR, x->id, x);


		t_symbol *qnamespace;
		t_symbol *qname;
		object_findregisteredbyptr(&qnamespace, &qname, x);
		post("this object is reg as %s, %s", qnamespace->s_name, qname->s_name);


		atom_arg_getsym(&x->alias, 0, argc, argv);
		x->val = 0;
		//receive_updateSends(x->namespaceS);	
	}
	return (x);
}


//assist
void receive_assist(t_receive *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
		sprintf(s,"(Messages)");
	else
		sprintf(s,"(Signal) Signal sent by connect send~ object");
}


//free
void receive_free(t_receive *x)
{
	dsp_free((t_pxobject *)x);
	object_unregister(x->obReg);
	receive_updateSends(x->namespaceS);
}



/*	Messages 	*******************************************************************/



//Set name of receive
void receive_set(t_receive *x, t_symbol *name)
{
	x->alias = name;
	receive_updateSends(x->namespaceS);
}


//Set value receive outputs, used by db.send~
void receive_out(t_receive *x, double v)
{
	x->val = v;
}



/*	Internal 	*******************************************************************/



//Get a unique id based on existing objects in a namespace
t_symbol *receive_getUniqueID(t_receive *x)
{
	int peak = -1;
	long nameLen;
	t_symbol **names;
	t_symbol *ns = gensym("db.receive~");
	object_register_getnames(ns, &nameLen, &names);
	post("Receive names found: %d\n", nameLen);
	if (nameLen > 0) {/*
		for (int i = 0; i < nameLen; i++) {
			post("Searching receives: %s\n", names[i]->s_name);
			char *name = names[i]->s_name;
			char *id = malloc(sizeof(name));
			short index = strcspn(name, '_');
			memcpy(id, &name[index + 1], strlen(name) - index);
			id[strlen(name) - index] = '\0';
			char *strp;
			int val = strtol(id, &strp, 10);
			if (val >= peak)
				peak = val;
			free(id);
		}
	*/}
	peak = rand() % 90;

	char newName[7];//7 is max amount of characters ID can have
   	sprintf(newName, "obj_%d", ++peak);
    free(names);
	return gensym(newName);
}


//Update each send object to find or release release object
void receive_updateSends(t_symbol *namespaceS)
{
	//Loop through list of all db.send~ objects
	long nameLen;
	t_symbol **names;
	object_register_getnames(&namespaceS, &nameLen, &names);
	if (nameLen > 0) {
		for (int i = 0; i < nameLen; i++) {
			//Update each object
			t_pxobject *s = object_findregistered(namespaceS, names[i]);
			if (s) object_method(s, gensym("update"));
		}
	}
	free(names);
}


//Get value of alias, for queries by other send objects
t_max_err receive_getAlias(t_receive *x, double arg, t_atom *av)
{
	if (av) atom_setsym(av, x->alias);
	return MAX_ERR_NONE;
}



/*	DSP 	*******************************************************************/



// registers a function for the signal chain
void receive_dsp64(t_receive *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), x, receive_perform64, 0, NULL);
}


//perform
void receive_perform64(t_receive *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	double *out = outs[0];
	while (sampleframes--)
		*out++ = x->val;
}
