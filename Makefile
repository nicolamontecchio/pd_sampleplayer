all : pdexternal

clean :
	rm -f *.o sampleplayer~.pd_darwin

sampleplayer.o : sampleplayer.cpp sampleplayer.h
	c++ -O3 -c -o sampleplayer.o sampleplayer.cpp

sampleplayer_c_interface.o : sampleplayer_c_interface.h sampleplayer_c_interface.cpp
	c++ -O3 -c -o sampleplayer_c_interface.o sampleplayer_c_interface.cpp

pdexternal : sampleplayer.o sampleplayer_pdext.c sampleplayer_c_interface.o
	cc -O3 -shared -lsndfile -undefined dynamic_lookup -o sampleplayer~.pd_darwin sampleplayer_pdext.c sampleplayer.o sampleplayer_c_interface.o

pdexternal_static : sampleplayer.o sampleplayer_pdext.c sampleplayer_c_interface.o
	cc -O3 -c -o sampleplayer_pd.o sampleplayer_pdext.c
	ar -cvq libpdsampleplayer.a *.o
