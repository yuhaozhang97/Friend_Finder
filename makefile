all: serverA.cpp serverB.cpp servermain.cpp client.cpp
	g++ -std=c++11 -o serverA serverA.cpp
	g++ -std=c++11 -o serverB serverB.cpp
	g++ -std=c++11 -o servermain servermain.cpp
	g++ -std=c++11 -o client client.cpp

.PHONY: serverA serverB mainserver

serverA:
	./serverA

serverB:
	./serverB

mainserver:
	./servermain
