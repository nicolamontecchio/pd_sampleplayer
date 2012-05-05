#include <map>
#include <list>
#include <string>
#include <vector>
#include <sndfile.h>

const int SAMPLE_PLAYER_MEMBLOCK_SIZE = 4 * 1024*1024; // 4M samples -> 16MB memblocks

class SampleMemoryBlock
{
	private:
		int blocksize;
		float *mem;                                             // memory block
		std::vector<int> samples;                               // samples contained in the block
		std::vector<int> samples_channels;                      // number of channels for each sample
		std::vector<std::pair<int, int> > samples_mempos;       // start/end point of each sample in the memory block (endpoint not inclusive)
		std::vector<int> samples_release;                       // duration, in samples, of release
	public:
		SampleMemoryBlock(int size) { mem = new float[size]; blocksize = size;}
		~SampleMemoryBlock() { delete[] mem; }
		/** load a sample, with a specific release duration in frames, 
		    into the memory block; if not enough space, 
				fails silently (use get_available_samples(...)) */
		void load_sample(int sample, SF_INFO sfinfo, SNDFILE* sndfile, int release);
		/** get the available number of samples left in the memory block*/
		int get_available_samples();
};

class Voice
{
	public:
		int voice_id;
		int sample_id;
		int sample_channels;
		SampleMemoryBlock* memoryblock;
		int in_memblock_sample_position;
		int in_memblock_sample_endpos;
		float intensity;
		int fadeout_remaining_length;
};

class SamplePlayer
{
	private:
		std::map<int, std::pair<std::string, int> > pitch_2_sample_file_mapping; // sample id -> [path,release_in_frames]
		std::list<Voice> active_voices;
		std::list<Voice> fadingout_voices;
		std::list<SampleMemoryBlock*> memory_blocks;
	public:
		~SamplePlayer();
		void add_sample(int pitch, std::string file_path, int release) { pitch_2_sample_file_mapping[pitch] = std::pair<std::string, int>(file_path, release);	}
		/** load all samples; return a list containing the file paths of those which could not be loaded */
		std::list<std::string> initialize();
		/** Voice on. Assumes the particular voice was indeed off before this call. */
		void voice_on(int voice, int sample_id, float intensity);
		/** voice off */
		void voice_off(int voice);
		/** Tick function. TODO write doc. on channel behaviour */
		void tick(float** out, int out_channels, int nsamples);
};

