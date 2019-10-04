CXX = g++

default: tracker peer


tracker.o: Tracker/tracker.cpp vaninet.cpp vaninet.hpp filehandling.cpp filehandling.hpp
	$(CXX) -c Tracker/tracker.cpp vaninet.cpp filehandling.cpp
	
tracker: Tracker/tracker.o vaninet.o filehandling.o
	$(CXX) -o Tracker/tracker Tracker/tracker.o vaninet.o filehandling.o

peer: Peer/peer.o vaninet.o filehandling.o
	$(CXX) -o Peer/peer Peer/peer.o vaninet.o filehandling.o -lssl -lcrypto

peer.o: Peer/peer.cpp vaninet.cpp vaninet.hpp filehandling.cpp filehandling.hpp
	$(CXX) -c Peer/peer.cpp

vaninet.o: vaninet.cpp
	$(CXX) -c vaninet.cpp

filehandling.o: filehandling.cpp
	$(CXX) -c filehandling.cpp