/*
*	Dillon Bastan, 2016
*	Send signal to multiple db.receive~ objects
*/


#include "ext.h"		
#include "ext_obex.h"
#include "z_dsp.h"


// Struct
typedef struct _send {
	t_pxobject ob;
	short connected;
	void *obReg;
	t_symbol *namespaceS;
	t_symbol *namespaceR;
	t_symbol *id;
	t_symbol *outSym;
	short numReceives;
	t_symbol **receiveAliases;
	t_pxobject **receives;
	t_atom *rt;
} t_send;



// Standard
void *send_new(t_symbol *s, long argc, t_atom *argv);
void send_assist(t_send *x, void *b, long m, long a, char *s);
void send_free(t_send *x);
//DSP
void send_dsp64(t_send *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void send_perform64(t_send *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void send_send(t_send *x, double in);
//Messages
void send_update(t_send *x);
void send_set(t_send *x, t_symbol *s, long argc, t_atom *argv);
//Internal
t_symbol *send_getUniqueID(t_send *x);
t_symbol *send_concatSym(t_symbol *sym1, t_symbol *sym2);



// global class pointer variable
static t_class *send_class = NULL;



//***********************************************************************************************



void ext_main(void *r)
{
	t_class *c = class_new("db.send~", (method)send_new, (method)send_free, (long)sizeof(t_send), 0L, A_GIMME, 0);

	class_addmethod(c, (method)send_dsp64, "dsp64",	A_CANT, 0);
	class_addmethod(c, (method)send_assist, "assist", A_CANT, 0);
	//
	class_addmethod(c, (method)send_update, "update", A_NOTHING, 0);
	class_addmethod(c, (method)send_set, "set", A_GIMME, 0);
	//
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	send_class = c;
}



/*	Standard 	*******************************************************************/



//new
void *send_new(t_symbol *s, long argc, t_atom *argv)
{
	t_send *x = (t_send *)object_alloc(send_class);
	
	if (x) {
		dsp_setup((t_pxobject *)x, 1);
		//Init and args
		x->namespaceS = gensym("db.send~");
		x->namespaceR = gensym("db.receive~");
		x->id = send_getUniqueID(x);
		post("send ID: %s\n", x->id->s_name);
		x->obReg = object_register(x->namespaceS, x->id, x);
		x->outSym = gensym("out");
		//Args are a list of db.receive~ names to send to
		x->numReceives = argc;
		x->receiveAliases = (t_symbol *)malloc(argc * sizeof(t_symbol));
		x->receives = (t_pxobject *)malloc(argc * sizeof(t_pxobject));
		for (int i = 0; i < argc; i++) {
			atom_arg_getsym(&x->receiveAliases[i], i, argc, argv);
		}
		send_update(x);
	}
	return (x);
}


//assist
void send_assist(t_send *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
		sprintf(s,"(Signal) Input to send to receive~ objects");
}


//free
void send_free(t_send *x)
{
	dsp_free((t_pxobject *)x);
	free(x->receiveAliases);
	free(x->receives);
}



/*	Messages 	*******************************************************************/



//Update receive objects based on aliases
void send_update(t_send *x)
{
	//Loop through list of all db.receive~ objects
	long *nameLen;
	t_symbol **names;
	object_register_getnames(x->namespaceR, nameLen, names);
	for (int i = 0; i < *nameLen; i++) {
		post("I'm a send searching to match: %s\n", names[i]->s_name);
		t_pxobject *r = object_findregistered(x->namespaceR, names[i]);
		//Compare with list of aliases to send to based on object's alias
		for (int j = 0; j < x->numReceives; j++) {
			if (r) {
				t_atom *rAliasAtom;
				object_method_float(r, gensym("getAlias"), 0, rAliasAtom);
				t_symbol *rAliasSym = atom_getsym(rAliasAtom);
				if (rAliasSym == x->receiveAliases[j])//Compare s_name??
					x->receives[j] = r;
				else
					x->receives[i] = NULL;
			} 
		}
	}
	free(names);
}


//Update receive aliases
void send_set(t_send *x, t_symbol *s, long argc, t_atom *argv)
{
	if (argc < 1)
		return NULL;
	x->numReceives = argc;
	x->receiveAliases = (t_symbol *)realloc(x->receiveAliases, argc * sizeof(t_symbol));
	x->receives = (t_pxobject *)realloc(x->receives, argc * sizeof(t_pxobject));
    for (int i = 0; i < argc; i++, argv++) {
    	x->receiveAliases[i] = atom_getsym(argv);
    }
    send_update(x);
}



/*	Internal 	*******************************************************************/



//
t_symbol *send_getUniqueID(t_send *x)
{
	int peak = 0;
	long *nameLen;
	t_symbol **names;
	object_register_getnames(x->namespaceS, nameLen, names);
	if (*nameLen > 0) {
		for (int i = 0; i < *nameLen; i++) {
			char *name = names[i]->s_name;
			char *id = malloc(sizeof(name));
			short index = strcspn(name, '_');
			memcpy(id, &name[index + 1], strlen(name) - index);
			id[strlen(name) - index] = '\0';
			char *strp;
			int val = strtol(id, &strp, 10);
			if (val > peak) peak = val;
			free(id);
		}
	}
	free(names);
	t_symbol *newID = gensym("obj_" + peak);
	return newID;
}


//Concat two symbols by their name
t_symbol *send_concatSym(t_symbol *sym1, t_symbol *sym2)
{
	const char *str1 = sym1->s_name;
	const char *str2 = sym2->s_name;
	char *result = malloc(strlen(str1) + strlen(str2) + 1);
	if (!result)
		return NULL;
    strcpy(result, str1);
    strcat(result, str2);
    t_symbol *resultSym = gensym(result);
    free(result);
    return resultSym;
}



/*	DSP 	*******************************************************************/



// registers a function for the signal chain in Max
void send_dsp64(t_send *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	//Keep track of count to know if signal is connected to inlet
	x->connected = count[0];
	object_method(dsp64, gensym("dsp_add64"), x, send_perform64, 0, NULL);
}


//perform
void send_perform64(t_send *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	double *in = ins[0];
	if (x->connected) {
		while (sampleframes--)
			send_send(x, *in++);
	}
}


//send
void send_send(t_send *x, double in) 
{
	for (short i = 0; i < x->numReceives; i++) {
		if (x->receives[i]) {
			object_method_float(x->receives[i], x->outSym, in, x->rt);
		}
	}
}
