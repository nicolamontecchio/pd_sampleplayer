#include "m_pd.h"
#include <string.h>
#include "sampleplayer_c_interface.h"

static t_class *sampleplayer_tilde_class;

typedef struct _sampleplayer_tilde {
	t_object  x_obj;
	void * sample_player_cpp_obj;
} t_sampleplayer_tilde;


void sampleplayer_control_inlet(t_sampleplayer_tilde *x, t_symbol *s, 
		int argc, t_atom *argv)
{
	if(strcmp(s->s_name, "initialize") == 0)
	{
		// TODO call sampleplayer->initialize();
	} 
	else if(strcmp(s->s_name, "setsample") == 0)
	{
		// TODO call sampleplayer->assign(...)
	} 
	else if(strcmp(s->s_name, "list") == 0)
	{
		// TODO a (voice/pitch/intensity) triple
	} else 
	{
		post("control inlet used with unknwon message");
		post("symbol: %s", s->s_name);
		post("argc: %d", argc);
	}
}





t_int *sampleplayer_tilde_perform(t_int *w)
{
	t_sampleplayer_tilde *x = (t_sampleplayer_tilde *)(w[1]);
	int          n =           (int)(w[3]); // number of samples


	/*
		 t_sample  *in1 =    (t_sample *)(w[2]);
		 t_sample  *in2 =    (t_sample *)(w[3]);
		 t_sample  *out =    (t_sample *)(w[4]);
		 t_sample f_pan = (x->f_pan<0)?0.0:(x->f_pan>1)?1.0:x->f_pan;

		 while (n--) *out++ = (*in1++)*(1-f_pan)+(*in2++)*f_pan;
	 */
	return (w+4);
}

void sampleplayer_tilde_dsp(t_sampleplayer_tilde *x, t_signal **sp)
{
	dsp_add(sampleplayer_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

void *sampleplayer_tilde_new(t_floatarg f)
{
	t_sampleplayer_tilde *x = (t_sampleplayer_tilde *)pd_new(sampleplayer_tilde_class);
	x->sample_player_cpp_obj = new_sampleplayer_obj();
	outlet_new(&x->x_obj, &s_signal);
	return (void *)x;
}

void sampleplayer_tilde_setup(void) {
	sampleplayer_tilde_class = class_new(gensym("sampleplayer~"),
			(t_newmethod)sampleplayer_tilde_new, 0, 
			sizeof(t_sampleplayer_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
	class_addmethod(sampleplayer_tilde_class, (t_method)sampleplayer_tilde_dsp, gensym("dsp"), 0);
	class_addanything(sampleplayer_tilde_class, (t_method) sampleplayer_control_inlet);
}


