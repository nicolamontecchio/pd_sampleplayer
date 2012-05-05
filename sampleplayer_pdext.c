#include "m_pd.h"
#include <string.h>
#include "sampleplayer_c_interface.h"

static t_class *sampleplayer_tilde_class;

typedef struct _sampleplayer_tilde {
	t_object  x_obj;
	void * sample_player_cpp_obj;
	int initialized;
} t_sampleplayer_tilde;


void sampleplayer_control_inlet(t_sampleplayer_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
	if(strcmp(s->s_name, "initialize") == 0)
	{
		int not_loaded_samples;
		if(x->initialized)
		{
			post("already initialized; cannot re-initialize");
			return;
		}
		not_loaded_samples = sampleplayer_initialize(x->sample_player_cpp_obj);
		if(not_loaded_samples > 0)
		{
			post("WARNING: %d samples were not loaded correctly", not_loaded_samples);
		}
		x->initialized = 1;
	} 
	else if(strcmp(s->s_name, "setsample") == 0)
	{
		int pitch;
		float release_time;
		t_symbol* sample_path;
		if(x->initialized)
		{
			post("already initialized, cannot add new samples");
			return;
		}
		if(argc != 3)
		{
			post("wrong number of arguments: must be pitch, release_in_seconds, sample_path");
			return;
		}
		if(argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT)
		{
			post("wrong type of arguments, must be int - float - string");
			return;
		}
		pitch = atom_getint(argv + 0);
		release_time = atom_getfloat(argv + 1) * sys_getsr();
		sample_path = atom_gensym(argv + 2);
		sampleplayer_set_sample(x->sample_player_cpp_obj, pitch, sample_path->s_name, release_time);
	} 
	else if(strcmp(s->s_name, "list") == 0) // a (voice/pitch/intensity) triple
	{
		int voice;
		int pitch;
		int intensity;
		if(argc != 3)
		{
			post("wrong message: must be [voice, pitch, intensity], like output from [poly] obj.");
			return;
		}
		voice = atom_getint(argv + 0);
		pitch = atom_getint(argv + 1);
		intensity = atom_getint(argv + 2);
		if(intensity > 0)
			sampleplayer_voice_on(x->sample_player_cpp_obj, voice, pitch, intensity / 128.f);
		else
			sampleplayer_voice_off(x->sample_player_cpp_obj, voice);
	} 
	else 
	{
		post("control inlet used with unknwon message");
		post("symbol: %s", s->s_name);
		post("argc: %d", argc);
	}
}





t_int *sampleplayer_tilde_perform(t_int *w)
{
	t_sampleplayer_tilde *x = (t_sampleplayer_tilde *)(w[1]);
	int   nsamples =           (int)(w[3]); // number of samples
	t_sample  *out =    (t_sample *)(w[2]); // output buffer (ONE for now)
	float* outpointer[1] = {(float*) out};
	sampleplayer_tick(x->sample_player_cpp_obj, outpointer, 1, nsamples);
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
	x->initialized = 0;
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

