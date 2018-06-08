all: gridserver vehicleclient griddisplay

gridserver: gridserver.cpp
	g++ -o gridserver gridserver.cpp myqueue.h

vehicleclient: vehicleclient.cpp
	g++ -o vehicleclient vehicleclient.cpp myqueue.h

griddisplay: griddisplay.cpp
	g++ -o griddisplay griddisplay.cpp

clean:
	rm gridserver vehicleclient griddisplay
