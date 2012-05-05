#include "sampleplayer.h"
using namespace std;

void SampleMemoryBlock::load_sample(int sample, SF_INFO sfinfo, SNDFILE* sndfile, int release)
{
	int sample_space = sfinfo.channels * sfinfo.frames;
	if(get_available_samples() < sample_space)
		return;
	samples.push_back(sample);
	samples_channels.push_back(sfinfo.channels);
	samples_release.push_back(release);
	int startpos = samples_mempos.empty() ? 0 : samples_mempos[samples_mempos.size() - 1].second;
	int endpos = startpos + sample_space;
	samples_mempos.push_back(pair<int,int>(startpos,endpos));
	sf_read_float(sndfile, mem + startpos, sample_space);
}

int SampleMemoryBlock::get_available_samples()
{
	if(samples_mempos.size() == 0)
		return blocksize;
	int last_endpos = samples_mempos[samples_mempos.size() - 1].second;
	return blocksize - last_endpos;
}

list<string> SamplePlayer::initialize()
{
	list<string> not_loaded;
	// memory_blocks.push_back(new SampleMemoryBlock(SAMPLE_PLAYER_MEMBLOCK_SIZE));

	for(map<int,pair<string,int> >::iterator it = pitch_2_sample_file_mapping.begin(); it != pitch_2_sample_file_mapping.end(); it++)
	{
		SF_INFO info;
		int sampleid = (*it).first;
		string samplepath = (*it).second.first;
		int release = (*it).second.second;
		SNDFILE *sndfile = sf_open(samplepath.c_str(),SFM_READ,&info);
		if(sndfile == NULL)
			not_loaded.push_back(samplepath);
		else
		{
			// if necessary, initialize an ad-hoc sized memory block
			if(info.frames * info.channels > SAMPLE_PLAYER_MEMBLOCK_SIZE)
				memory_blocks.push_front(new SampleMemoryBlock(info.frames * info.channels)); 
			// find the first sufficiently big memory block
			SampleMemoryBlock* assigned_memblock = NULL;
			for(list<SampleMemoryBlock*>::iterator mbit = memory_blocks.begin(); mbit != memory_blocks.end(); mbit++)
			{
				SampleMemoryBlock* mb = (*mbit);
				if(info.frames * info.channels <= mb->get_available_samples())
					assigned_memblock = mb;
			}
			// not found? allocate new of default size
			if(assigned_memblock == NULL)
			{
				assigned_memblock = new SampleMemoryBlock(info.frames * info.channels);
				memory_blocks.push_back(assigned_memblock);
			}
			// now sample can be safely added to memblock
			assigned_memblock->load_sample(sampleid, info, sndfile, release);
		}
		sf_close(sndfile);
	}
	return not_loaded;
}

SamplePlayer::~SamplePlayer()
{
	for(list<SampleMemoryBlock*>::iterator it = memory_blocks.begin(); it != memory_blocks.end(); it++)
		delete (*it);
}

void SamplePlayer::voice_on(int voice, int sample_id, float intensity)
{
	
}

