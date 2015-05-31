// plain C re-implementation
#include "sampleplayer_c_interface.h"
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

const int N_MAX_SAMPLES = 8192;
const int N_VOICES = 256;

SamplePlayer sampleplayer_new()
{
  SamplePlayer sp;
  sp.initialized = 0;
  sp.n_samples = 0;
  sp.samples = (Sample *) malloc(sizeof(Sample) * N_MAX_SAMPLES);
  return sp;
}

void sampleplayer_free(SamplePlayer *sp)
{
  int n;
  for(n = 0; n < sp->n_samples; n++)
    free(sp->samples[n].file_path);
  if(sp->memblock != 0)
    free(sp->memblock);
}

int sampleplayer_add_sample(SamplePlayer *sp, Sample s)
{
  if(sp->n_samples < N_MAX_SAMPLES)
  {
    sp->samples[sp->n_samples].pitch = s.pitch;
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
    memblock_size += (info.channels * info.frames);
    sf_close(sndfile);
  }
  memblock_size *= sizeof(float);

  // create mem block
  sp->memblock = (float *) malloc(memblock_size);
  if(sp->memblock == 0)
    return SPLR_ERROR_CANNOT_ALLOCATE_MEMORY;

  // copy all samples to mem, storing their position in the mem block
  for(n = 0; n < sp->n_samples; n++)
  {
    SF_INFO info;
    SNDFILE *sndfile = sf_open(sp->samples[n].file_path, SFM_READ, &info);
    sf_readf_float(sndfile, sp->memblock + memblock_pos, info.frames);
    sp->samples[n].sample_mem_position_start = memblock_pos;
    memblock_pos += info.frames * info.channels;
    sp->samples[n].sample_mem_position_end = memblock_pos;
    sp->samples[n].channels = info.channels;
    sf_close(sndfile);
  }

  sp->initialized = 1;
  return SPLR_OK;
}

int sampleplayer_voice_on(SamplePlayer *sp, int voice, int pitch, float intensity, int release_samples)
{
  int n;
  Voice v;

  // find sample (TODO implement binary search since they are ordered, but linear is ok for now)
  for(n = 0; n < sp->n_samples; n++)
    if(sp->samples[n].pitch == pitch)
      break;
  if(n == sp->n_samples)
    return SPLR_ERROR_INVALID_PITCH;

  // make a voice out of it, replace whatever is in sp->voices[voice]
  v.active = 1;
  v.pitch = pitch;
  v.sample_channels = sp->samples[n].channels;
  v.sample_mem_position_current = sp->samples[n].sample_mem_position_start;
  v.sample_mem_position_end = sp->samples[n].sample_mem_position_end;
  v.intensity = intensity;
  v.release_remaining_length = release_samples;
  sp->voices[voice] = v;

  return SPLR_OK;
}
