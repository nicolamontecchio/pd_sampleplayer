
typedef enum
{
  SPLR_OK,
  SPLR_ERROR_TOO_MANY_SAMPLES_ALREADY,
  SPLR_ERROR_CANNOT_OPEN_SAMPLE_FILE,
  SPLR_ERROR_CANNOT_ALLOCATE_MEMORY,
  SPLR_ERROR_SAMPLER_UNINITIALIZED,
  SPLR_ERROR_INVALID_PITCH
} sampleplayer_error_codes;

typedef struct
{
  int active;
  int pitch;
  int sample_channels;
  int sample_mem_position_current;
  int sample_mem_position_end;
  float intensity;
  int release_remaining_length;
} Voice;


typedef struct
{
  int pitch;
  char *file_path;
  int sample_mem_position_start;
  int sample_mem_position_end;
  int channels;
} Sample;


typedef struct
{
  int initialized;
  int n_samples;
  Sample *samples;
  float *memblock;
  Voice *voices;
} SamplePlayer;


#ifdef __cplusplus
extern "C"
{
#endif

  SamplePlayer sampleplayer_new();
  void sampleplayer_free(SamplePlayer *sp);
  int sampleplayer_add_sample(SamplePlayer *sp, Sample s);
  int sampleplayer_initialize(SamplePlayer *sp);
  int sampleplayer_voice_on(SamplePlayer *sp, int voice, int pitch, float intensity, int release_samples);
  /* void sampleplayer_voice_off(void *o, int voice); */
  /* void sampleplayer_tick(void *o, float** out, int out_channels, int nsamples); */
  /* int sampleplayer_memoryusage(void *o); */

#ifdef __cplusplus
}
#endif
