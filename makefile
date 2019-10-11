CXX = g++

default: tracker peer

tracker: Tracker/tracker.o vaninet.o filehandling.o
	$(CXX) -g -Wall -o Tracker/tracker Tracker/tracker.o vaninet.o filehandling.o -lpthread

peer: Peer/peer.o vaninet.o filehandling.o
	$(CXX) -g -Wall -o Peer/peer Peer/peer.o vaninet.o filehandling.o -lssl -lcrypto -lpthread

tracker.o: Tracker/tracker.cpp vaninet.cpp vaninet.hpp filehandling.cpp filehandling.hpp
	$(CXX) -g -Wall -c Tracker/tracker.cpp vaninet.cpp filehandling.cpp 

peer.o: Peer/peer.cpp vaninet.cpp vaninet.hpp filehandling.cpp filehandling.hpp
	$(CXX) -g -c -Wall Peer/peer.cpp

vaninet.o: vaninet.cpp
	$(CXX) -g -c -Wall vaninet.cpp

filehandling.o: filehandling.cpp
	$(CXX) -g -c -Wall filehandling.cpp