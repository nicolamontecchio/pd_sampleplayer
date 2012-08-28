#include "sampleplayer.h"
#include <iostream>
using namespace std;

int SampleMemoryBlock::get_sample_index(int sampleid)
{
	for(int i = 0; i < samples.size(); i++)
		if(samples[i] == sampleid)
			return i;
	return -1;
}

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
	for(map<int,pair<string,int> >::iterator it = pitch_2_sample_file_mapping.begin(); it != pitch_2_sample_file_mapping.end(); it++)
	{
		SF_INFO info;
		int sampleid = (*it).first;
		string samplepath = (*it).second.first;
		// cout << "trying to load " << samplepath << endl;
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
			// store compact access descriptor for later use in voiceon()
			sample_to_memblock_and_sampleindex[sampleid] = pair<SampleMemoryBlock*, int>(assigned_memblock, assigned_memblock->get_sample_index(sampleid));
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
	if(sample_to_memblock_and_sampleindex.find(sample_id) == sample_to_memblock_and_sampleindex.end())
		return;
	// enqueue in the active_voices list 
	pair<SampleMemoryBlock*, int> ssii = sample_to_memblock_and_sampleindex[sample_id];
	SampleMemoryBlock* memblock = ssii.first;
	int sample_index_in_memblock = ssii.second;
	Voice v;
	v.voice_id = voice;
	v.sample_id = sample_id;
	v.sample_channels = memblock->samples_channels[sample_index_in_memblock];
	v.memoryblock = memblock;
	v.in_memblock_sample_position = memblock->samples_mempos[sample_index_in_memblock].first;
	v.in_memblock_sample_endpos= memblock->samples_mempos[sample_index_in_memblock].second;
	v.intensity = intensity;
	v.release_remaining_length = memblock->samples_release[sample_index_in_memblock];
	active_voices.push_back(v);
}

void SamplePlayer::voice_off(int voice)
{
	for(list<Voice>::iterator it = active_voices.begin(); it != active_voices.end(); it++)
		if((*it).voice_id == voice)
		{
			Voice v = *it;
			active_voices.erase(it);
			releasing_voices.push_back(v);
			return;
		}
}

inline float release_multiplier(int frame, int tot_frames)
{
	return 1. - ((float) frame) / ((float) tot_frames);
}

void SamplePlayer::tick_voice(Voice *v, float** out, int out_channels, int nsamples, bool releasing)
{
	SampleMemoryBlock* mb = v->memoryblock;
	if(v->sample_channels == out_channels) 		// voice has same number of channels -> just copy
		for(int n = 0; n < nsamples; n++)
		{
			if(v->in_memblock_sample_position >= v->in_memblock_sample_endpos)
				break;	
			float rm = (releasing ? release_multiplier(n,v->release_remaining_length) : 1.f);
			for(int c = 0; c < out_channels; c++)
				out[c][n] += v->intensity * mb->mem[v->in_memblock_sample_position++] * rm;
			if(v->in_memblock_sample_position >= v->in_memblock_sample_endpos)
				break;
		}
	else		                                  // voice has different number of channels -> monoize and copy equally in all channels
		for(int n = 0; n < nsamples; n++)
		{
			if(v->in_memblock_sample_position >= v->in_memblock_sample_endpos)
				break;	
			float o = 0;
			for(int c = 0; c < v->sample_channels; c++)
				o += mb->mem[v->in_memblock_sample_position++];
			o *= (releasing ? release_multiplier(n,v->release_remaining_length) : 1.f);
			for(int c = 0; c < out_channels; c++)
				out[c][n] += v->intensity / out_channels * o;
		}
	if(releasing)
	{
		v->intensity *= release_multiplier(nsamples, v->release_remaining_length);
		v->release_remaining_length -= nsamples;
	}
}

void SamplePlayer::tick(float** out, int out_channels, int nsamples)
{
	for(int c = 0; c < out_channels; c++)
		for(int n = 0; n < nsamples; n++)
			out[c][n] = 0.;
	for(list<Voice>::iterator it = active_voices.begin(); it != active_voices.end(); it++)
	{
		tick_voice(&(*it), out, out_channels, nsamples, false);
		if((*it).in_memblock_sample_position >= (*it).in_memblock_sample_endpos)
			active_voices.erase(it);
	}
	//	cout << "active voices: " << active_voices.size() << endl;
	for(list<Voice>::iterator it = releasing_voices.begin(); it != releasing_voices.end(); it++)
	{
		tick_voice(&(*it), out, out_channels, nsamples, true);
		if((*it).intensity <= 0.01 || (*it).in_memblock_sample_position >= (*it).in_memblock_sample_endpos)
			releasing_voices.erase(it);
	}
	// cout << "releasing voices: " << releasing_voices.size() << endl;
}

int SamplePlayer::get_sample_memory()
{
	int memtot = 0;
	for(list<SampleMemoryBlock*>::iterator it = memory_blocks.begin(); it != memory_blocks.end(); it++)
		memtot += (*it)->blocksize * sizeof(float);
	return memtot;
}

