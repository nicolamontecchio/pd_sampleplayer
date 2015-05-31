// #include "sampleplayer_c_interface.h"
#include "sampleplayer.h"
#include <list>
#include <string>
using namespace std;

void* new_sampleplayer_obj()
{
	return (void *) new SamplePlayer();
}

void free_sampleplayer_obj(void* o)
{
	delete (SamplePlayer*) o;
}

int sampleplayer_initialize(void* o)
{
	list<string> non_loaded =	((SamplePlayer*) o)->initialize();
	return non_loaded.size();
}

void sampleplayer_set_sample(void* o, int pitch, char* file_path, int release)
{
	SamplePlayer* sp = (SamplePlayer*) o;
	sp->add_sample(pitch, string(file_path), release);
}

void sampleplayer_tick(void *o, float** out, int out_channels, int nsamples)
{
	SamplePlayer* sp = (SamplePlayer*) o;
	sp->tick(out, out_channels, nsamples);
}

void sampleplayer_voice_on(void* o, int voice, int sample_id, float intensity)
{
	SamplePlayer* sp = (SamplePlayer*) o;
	sp->voice_on(voice, sample_id, intensity);
}

void sampleplayer_voice_off(void *o, int voice)
{
	SamplePlayer* sp = (SamplePlayer*) o;
	sp->voice_off(voice);
}

int sampleplayer_memoryusage(void *o)
{
	SamplePlayer* sp = (SamplePlayer*) o;
	return sp->get_sample_memory();
}
