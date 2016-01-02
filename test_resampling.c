#include <stdio.h>
#include <stdlib.h>
#include "sampleplayer.h"
#include <sndfile.h>

void print_usage(char *prog_name)
{
  printf("usage:\n%s input.wav output.wav speedup\n", prog_name);
}

int main(int argc, char **argv)
{

  if(argc != 4)
  {
    print_usage(argv[0]);
    return 1;
  }

  char *path_in = argv[1];
  char *path_out = argv[2];
  double speedup = atof(argv[3]);

  SF_INFO info_in;
  SNDFILE *sndfile_in = sf_open(path_in, SFM_READ, &info_in);
  if(sndfile_in == 0)
  {
    printf("could not open input file %s\n", path_in);
    return 1;
  }
  int n_channels = info_in.channels;
  int n_frames_in = info_in.frames;

  float *input = (float *) malloc(
    sizeof(float) * n_channels * info_in.frames);
  sf_read_float(sndfile_in, input, info_in.frames * n_channels);

  float **input_deinterlaced = (float **) malloc(sizeof(float *) * n_channels);
  for(int c = 0; c < n_channels; c++)
  {
    input_deinterlaced[c] = (float *) malloc(sizeof(float) * n_frames_in);
    for(int n= 0 ; n < n_frames_in; n++)
      input_deinterlaced[c][n] = input[n * n_channels + c];
  }

  int n_frames_out = resampled_signal_length(n_frames_in, speedup);
  float **output_deinterlaced = (float **) malloc(sizeof(float *) * n_channels);
  for(int c = 0; c < n_channels; c++)
    output_deinterlaced[c] = (float *) malloc(n_frames_out);

  resample(input_deinterlaced, n_channels, n_frames_in, output_deinterlaced, speedup);

  float *output_interlaced = (float *) malloc(sizeof(float) * n_frames_out * n_channels);
  for(int c = 0; c < n_channels; c++)
    for(int n = 0; n < n_frames_out; n++)
      output_interlaced[n * n_channels + c] = output_deinterlaced[c][n];

  SF_INFO info_out;
  info_out.channels = n_channels;
  info_out.format = info_in.format;
  info_out.samplerate = info_in.samplerate;
  SNDFILE *sndfile_out = sf_open(path_out, SFM_WRITE, &info_out);
  sf_write_float (sndfile_out, output_interlaced, n_channels * n_frames_out);
  sf_close(sndfile_out);

  for(int c = 0; c < n_channels; c++)
  {
    free(input_deinterlaced[c]);
    free(output_deinterlaced[c]);
  }
  free(input_deinterlaced);
  free(output_deinterlaced);
  free(output_interlaced);
  free(input);
  sf_close(sndfile_in);

}
