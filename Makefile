CC           = gcc
CFLAGS       = -g
DEPENDENCIES = ImageProcessor.cpp Delegator.cpp main.cpp
LINKS        = -lpthread -lstdc++

all:
	$(CC) $(DEPENDENCIES) $(CFLAGS) $(LINKS) -o delegatorTest

cv:
	$(CC) $(DEPENDENCIES) $(CFLAGS) $(LINKS) -o delegatorTest `pkg-config --cflags --libs opencv`

clean:
	rm -f delegatorTest
	rm -rf delegatorTest.dSYM
