#include "m_pd.h"
#include <string.h>
#include <stdio.h>
#include "sampleplayer.h"

static t_class *sampleplayer_tilde_class;

typedef struct _sampleplayer_tilde
{
  t_object  x_obj;
  SamplePlayer *sp;
  t_symbol* canvas_dir;
} t_sampleplayer_tilde;


void sampleplayer_control_inlet(t_sampleplayer_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  if(strcmp(s->s_name, "initialize") == 0)
  {
    if(x->sp->initialized)
      post("already initialized; cannot re-initialize");
    else
    {
      int status = sampleplayer_initialize(x->sp);
      if(status != SPLR_OK)
	post("ERROR: sample player did not initialize correctly (error %d)", status);
      else
	post("sampleplayer~ initialized");
    }
  }
  else if(strcmp(s->s_name, "addsample") == 0)
  {
    int pitch;
    float release_time;
    t_symbol* sample_path;
    char full_sample_path[2048];
    Sample s;
    if(x->sp->initialized)
    {
      post("already initialized, cannot add new samples");
      return;
    }
    if(argc != 2)
    {
      post("wrong number of arguments: must be pitch, sample_path");
      return;
    }
    if(argv[0].a_type != A_FLOAT)
    {
      post("wrong type of arguments, must be int - string");
      return;
    }
    pitch = atom_getint(argv + 0);
    sample_path = atom_gensym(argv + 1);
    if(sample_path->s_name[0] == '/' || sample_path->s_name[0] == '\\')
      strcpy(full_sample_path, sample_path->s_name);
    else
      snprintf(full_sample_path, 2048, "%s/%s", x->canvas_dir->s_name, sample_path->s_name);
    s.pitch = pitch;
    s.file_path = full_sample_path;
    post("adding %s", s.file_path);
    sampleplayer_add_sample(x->sp, s);
  }
  else
  {
    if(!x->sp->initialized)
      post("ERROR: sampleplayer~ not initialized");
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
	sampleplayer_voice_on(x->sp, voice, pitch, intensity / 128.f, 1024);   // TODO FIXME
      else
	sampleplayer_voice_off(x->sp, voice);
    }
    else
    {
      post("control inlet used with unknwon message");
      post("symbol: %s", s->s_name);
      post("argc: %d", argc);
    }
  }
}


t_int *sampleplayer_tilde_perform(t_int *w)
{
  t_sampleplayer_tilde *x = (t_sampleplayer_tilde *)(w[1]);
  t_sample  *outL =    (t_sample *)(w[2]);
  t_sample  *outR =    (t_sample *)(w[3]);
  int   nsamples =            (int)(w[4]);
  float* outpointer[2] = {(float*) outL, (float*) outR};
  sampleplayer_tick(x->sp, outpointer, nsamples);
  return (w+5);
}

void sampleplayer_tilde_dsp(t_sampleplayer_tilde *x, t_signal **sp)
{
  printf("tilde_dsp begin\n"); fflush(stdout);
  dsp_add(sampleplayer_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
  printf("tilde_dsp end\n"); fflush(stdout);
}

void *sampleplayer_tilde_new()
{
  t_sampleplayer_tilde *x = (t_sampleplayer_tilde *)pd_new(sampleplayer_tilde_class);
  x->sp = sampleplayer_new(2);
  x->canvas_dir = canvas_getcurrentdir();
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  return (void *)x;
}

void sampleplayer_tilde_free(t_sampleplayer_tilde *x)
{
  sampleplayer_free(x->sp);
}

void sampleplayer_tilde_setup(void) {
  sampleplayer_tilde_class = class_new(gensym("sampleplayer~"),
				       (t_newmethod)sampleplayer_tilde_new,
				       (t_method)sampleplayer_tilde_free,
				       sizeof(t_sampleplayer_tilde),
				       CLASS_DEFAULT, 0);
  class_addmethod(sampleplayer_tilde_class, (t_method)sampleplayer_tilde_dsp, gensym("dsp"), A_CANT, 0);
  class_addanything(sampleplayer_tilde_class, (t_method) sampleplayer_control_inlet);
}
