// plain C re-implementation
#include "sampleplayer.h"
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

const int N_MAX_SAMPLES = 8192;
const int N_VOICES = 256;

SamplePlayer * sampleplayer_new(int n_channels)
{
  SamplePlayer *sp = (SamplePlayer *) malloc(sizeof(SamplePlayer));
  sp->initialized = 0;
  sp->n_samples = 0;
  sp->n_channels = n_channels;
  sp->samples = (Sample *) malloc(sizeof(Sample) * N_MAX_SAMPLES);
  sp->voices = (Voice *) malloc(sizeof(Voice) * N_VOICES);
  return sp;
}

void sampleplayer_free(SamplePlayer *sp)
{
  int n;
  for(n = 0; n < sp->n_samples; n++)
    free(sp->samples[n].file_path);
  if(sp->memblock != 0)
    free(sp->memblock);
  free(sp->voices);
}

int sampleplayer_add_sample(SamplePlayer *sp, Sample s)
{
  if(sp->n_samples < N_MAX_SAMPLES)
  {
    sp->samples[sp->n_samples].pitch = s.pitch;
    sp->samples[sp->n_samples].loop_start_frame = s.loop_start_frame;
    sp->samples[sp->n_samples].loop_end_frame = s.loop_end_frame;
    sp->samples[sp->n_samples].file_path = (char *) malloc(strlen(s.file_path) + 1);
    strcpy(sp->samples[sp->n_samples].file_path, s.file_path);
    sp->n_samples++;
    return SPLR_OK;
  }
  else
    return SPLR_ERROR_TOO_MANY_SAMPLES_ALREADY;
}

int sampleplayer_sample_compare_pitch(const void *a, const void *b)
{
  return (((Sample *) a)->pitch - ((Sample *) b)->pitch);
}

int sampleplayer_initialize(SamplePlayer *sp)
{
  int memblock_size;
  int memblock_pos;
  int n;
  int v;

  for(v = 0; v < N_VOICES; v++)
    sp->voices[v].active = 0;

  // sort samples by pitch
  qsort((void *) sp->samples, sp->n_samples, sizeof(Sample), sampleplayer_sample_compare_pitch);

  // measure necessary space
  memblock_size = 0;
  for(n = 0; n < sp->n_samples; n++)
  {
    SF_INFO info;
    SNDFILE *sndfile = sf_open(sp->samples[n].file_path, SFM_READ, &info);
    if(sndfile == NULL)
      return SPLR_ERROR_CANNOT_OPEN_SAMPLE_FILE;
    if(info.channels != sp->n_channels)
      return SPLR_ERROR_INVALID_NUMBER_OF_CHANNELS;
    memblock_size += (sp->n_channels * info.frames);
    sf_close(sndfile);
  }
  memblock_size *= sizeof(float);

  // create mem block
  sp->memblock = (float *) malloc(memblock_size);
  if(sp->memblock == 0)
    return SPLR_ERROR_CANNOT_ALLOCATE_MEMORY;

  // copy all samples to mem, storing their position in the mem block
  memblock_pos = 0;
  for(n = 0; n < sp->n_samples; n++)
  {
    SF_INFO info;
    SNDFILE *sndfile = sf_open(sp->samples[n].file_path, SFM_READ, &info);
    Sample *s = sp->samples + n;
    sf_readf_float(sndfile, sp->memblock + memblock_pos, info.frames);
    s->sample_mem_position_start = memblock_pos;
    memblock_pos += info.frames * sp->n_channels;
    s->sample_mem_position_end = memblock_pos;
    sf_close(sndfile);
  }

  sp->initialized = 1;
  return SPLR_OK;
}

int sampleplayer_voice_on(SamplePlayer *sp, int voice, int pitch, float intensity, int release_samples)
{
  int n;
  Voice v;
  Sample *s;

  // find sample (TODO implement binary search since they are ordered, but linear is ok for now)
  for(n = 0; n < sp->n_samples; n++)
    if(sp->samples[n].pitch == pitch)
      break;
  if(n == sp->n_samples)
    return SPLR_ERROR_INVALID_PITCH;

  s = sp->samples + n;

  // make a voice out of it, replace whatever is in sp->voices[voice]
  v.active = 1;
  v.releasing = 0;
  v.pitch = pitch;
  v.sample_mem_position_start = s->sample_mem_position_start;
  v.sample_mem_position_current = v.sample_mem_position_start;
  v.sample_mem_position_end = s->sample_mem_position_end;
  v.sample_mem_loop_start = v.sample_mem_position_start + s->loop_start_frame * sp->n_channels;
  v.sample_mem_loop_end = v.sample_mem_position_start + s->loop_end_frame * sp->n_channels;
  v.intensity = intensity;
  v.release_length = release_samples;
  v.release_remaining_length = release_samples;
  sp->voices[voice] = v;

  return SPLR_OK;
}

void sampleplayer_voice_off(SamplePlayer *sp, int voice)
{
  sp->voices[voice].releasing = 1;
}

static inline float release_multiplier(int remaining, int total)
{
  return remaining > 0 ? ((float) remaining) / ((float) total) : 0;
}

void sampleplayer_tick(SamplePlayer *sp, float** out, int n_frames)
{
  if(sp->initialized)
  {
    int n;
    int voice;
    int channel;

    for(channel = 0; channel < sp->n_channels; channel++)
      for(n = 0; n < n_frames; n++)
	out[channel][n] = 0;

    for(voice = 0; voice < N_VOICES; voice++)
    {
      if(sp->voices[voice].active)
      {
	Voice *v = &sp->voices[voice];
	/* printf("voice %d active -- mempos_current: %d \n", voice, v->sample_mem_position_current); */

	// loop?


	// has more remaining samples than n_frames? otherwise set to inactive

	for(n = 0; n < n_frames; n++)
	{
	  if(v->sample_mem_position_current == v->sample_mem_position_end ||
	     v->release_remaining_length == 0)
	    v->active = 0;

	  if(!v->active)
	    break;

	  float w_n = v->intensity * (v->releasing ? release_multiplier(
					v->release_remaining_length, v->release_length) : 1);
	  for(channel = 0; channel < sp->n_channels; channel++)
	  {
	    out[channel][n] += w_n * sp->memblock[v->sample_mem_position_current];
	    v->sample_mem_position_current++;
	  }
	  if(v->releasing)
	    v->release_remaining_length--;
	}
      }
    }
  }
}
