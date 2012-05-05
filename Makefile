all : pdexternal

clean :
	rm -f *.o sampleplayer~.pd_darwin

sampleplayer.o : sampleplayer.cpp sampleplayer.h
	clang -O3 -arch i386 -arch x86_64 -c -o sampleplayer.o sampleplayer.cpp

sampleplayer_c_interface.o : sampleplayer_c_interface.h sampleplayer_c_interface.cpp
	clang -O3 -arch i386 -arch x86_64 -c -o sampleplayer_c_interface.o sampleplayer_c_interface.cpp

pdexternal : sampleplayer.o sampleplayer_pdext.c sampleplayer_c_interface.o 
	clang -O3 -arch i386 -arch x86_64 -shared -lsndfile -undefined dynamic_lookup -o sampleplayer~.pd_darwin sampleplayer_pdext.c sampleplayer.o sampleplayer_c_interface.o

