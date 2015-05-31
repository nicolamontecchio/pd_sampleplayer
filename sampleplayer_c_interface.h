
typedef enum
{
  OK,
  ERROR_TOO_MANY_SAMPLES_ALREADY,
  ERROR_CANNOT_OPEN_SAMPLE_FILE,
  ERROR_CANNOT_ALLOCATE_MEMORY
} sampleplayer_ERROR_CODES;

typedef struct
{
  int voice_id;
  int sample_id;
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
} Sample;


typedef struct
{
  int initialized;
  int n_samples;
  Sample *samples;
  float *memblock;
} SamplePlayer;


#ifdef __cplusplus
extern "C"
{
#endif

  SamplePlayer sampleplayer_new();
  void sampleplayer_free(SamplePlayer *sp);
  int sampleplayer_add_sample(SamplePlayer *sp, Sample s);
  int sampleplayer_initialize(SamplePlayer *sp);

  /* void sampleplayer_voice_on(void* o, int voice, int sample_id, float intensity); */
  /* void sampleplayer_voice_off(void *o, int voice); */
  /* void sampleplayer_tick(void *o, float** out, int out_channels, int nsamples); */
  /* int sampleplayer_memoryusage(void *o); */

#ifdef __cplusplus
}
#endif
