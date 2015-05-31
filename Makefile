all : pdexternal

clean :
	rm -f *.o sampleplayer~.pd_darwin

sampleplayer.o : sampleplayer_c.c sampleplayer_c_interface.h
	cc -c -o sampleplayer.o sampleplayer_c.c

pdext.o : sampleplayer_pdext.c sampleplayer_c_interface.h
	cc -c -o pdext.o sampleplayer_pdext.c

pdexternal : sampleplayer.o sampleplayer_pdext.c pdext.o
	cc -shared -lsndfile -undefined dynamic_lookup -o sampleplayer~.pd_darwin  sampleplayer.o pdext.o

# pdexternal_static : sampleplayer.o sampleplayer_pdext.c sampleplayer_c_interface.o
# 	cc -O3 -c -o sampleplayer_pd.o sampleplayer_pdext.c
