#ifdef __cplusplus
extern "C" 
{
#endif 


	void* new_sampleplayer_obj();
	void free_sampleplayer_obj(void* o);
	int sampleplayer_initialize(void* o); // for now just return the number of non-loaded samples
	void sampleplayer_set_sample(void* o, int pitch, char* file_path, int release);
	void sampleplayer_tick(void *o, float** out, int out_channels, int nsamples);


#ifdef __cplusplus
}
#endif

