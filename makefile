OBJS = /src/main.cpp /src/conversion.h /src/mtrand.h /src/conversion.cpp /src/mtrand.cpp
CC = g++
DEBUG = -g -Wall
VERS = -std=c++11


MarkovMozart: $(OBJS)
	$(CC) $(VERS) $(DEBUG) $(OBJS) -o MarkovMozart
