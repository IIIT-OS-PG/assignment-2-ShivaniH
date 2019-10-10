CXX = g++

default: tracker peer


tracker.o: Tracker/tracker.cpp vaninet.cpp vaninet.hpp filehandling.cpp filehandling.hpp
	$(CXX) -g -c Tracker/tracker.cpp vaninet.cpp filehandling.cpp 
	
tracker: Tracker/tracker.o vaninet.o filehandling.o
	$(CXX) -g -o Tracker/tracker Tracker/tracker.o vaninet.o filehandling.o -lpthread

peer: Peer/peer.o vaninet.o filehandling.o
	$(CXX) -g -o Peer/peer Peer/peer.o vaninet.o filehandling.o -lssl -lcrypto -lpthread

peer.o: Peer/peer.cpp vaninet.cpp vaninet.hpp filehandling.cpp filehandling.hpp
	$(CXX) -g -c Peer/peer.cpp

vaninet.o: vaninet.cpp
	$(CXX) -g -c vaninet.cpp

filehandling.o: filehandling.cpp
	$(CXX) -g -c filehandling.cpp