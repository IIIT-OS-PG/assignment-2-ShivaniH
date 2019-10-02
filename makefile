CXX = g++

default: tracker peer

tracker: tracker.o vaninet.o 
	$(CXX) -o tracker tracker.o vaninet.o 

tracker.o: Tracker/tracker.cpp vaninet.cpp vaninet.hpp
	$(CXX) -c Tracker/tracker.cpp vaninet.cpp

peer: peer.o vaninet.o
	$(CXX) -o peer peer.o vaninet.o

peer.o: Peer/peer.cpp vaninet.cpp vaninet.hpp
	$(CXX) -c Peer/peer.cpp

vaninet.o: vaninet.cpp
	$(CXX) -c vaninet.cpp