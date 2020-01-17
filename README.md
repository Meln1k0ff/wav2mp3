Converting multiple wav files to mp3 in parallel. To build app on Linux, build LAME mp3 codec and link it statically with an application. Then, run sudo make install to put lame.h in /usr/local/lib and run cmake CMakeLists.txt. 
To build under Windows, build LAME via MinGW and put a static library in the same directory where the source is.
