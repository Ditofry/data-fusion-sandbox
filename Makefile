CC           = clang++
CFLAGS       = -g -std=c++11
DEPENDENCIES = ImageProcessor.cpp Delegator.cpp CVImage.capnp.c++ main.cpp
LINKS        = -lpthread -lstdc++

all:
	$(CC) $(DEPENDENCIES) $(CFLAGS) $(LINKS) -o delegatorTest `pkg-config --cflags --libs opencv`

clean:
	rm -f delegatorTest
	rm -rf delegatorTest.dSYM
