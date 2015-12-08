
typedef enum
{
  SPLR_OK,
  SPLR_ERROR_TOO_MANY_SAMPLES_ALREADY,
  SPLR_ERROR_CANNOT_OPEN_SAMPLE_FILE,
  SPLR_ERROR_CANNOT_ALLOCATE_MEMORY,
  SPLR_ERROR_SAMPLER_UNINITIALIZED,
  SPLR_ERROR_INVALID_PITCH,
  SPLR_ERROR_INVALID_NUMBER_OF_CHANNELS
} sampleplayer_error_codes;

typedef struct
{
  int active;
  int releasing;
  int pitch;
  int sample_mem_position_start;
  int sample_mem_position_current;
  int sample_mem_position_end;
  float intensity;
  int release_length;
  int release_remaining_length;
} Voice;


typedef struct
{
  int pitch;
  char *file_path;
  int sample_mem_position_start;
  int sample_mem_position_end;
  int loop_start_frame;  // if < 0, no loop
  int loop_end_frame;
} Sample;


typedef struct
{
  int initialized;
  int n_samples;
  int n_channels;
  Sample *samples;
  float *memblock;
  Voice *voices;
} SamplePlayer;


SamplePlayer * sampleplayer_new(int n_channels);
void sampleplayer_free(SamplePlayer *sp);
int sampleplayer_add_sample(SamplePlayer *sp, Sample s);
int sampleplayer_initialize(SamplePlayer *sp);
int sampleplayer_voice_on(SamplePlayer *sp, int voice, int pitch, float intensity, int release_samples);
void sampleplayer_voice_off(SamplePlayer *sp, int voice);
void sampleplayer_tick(SamplePlayer *sp, float** out, int n_frames);
/* int sampleplayer_memoryusage(void *o); */
