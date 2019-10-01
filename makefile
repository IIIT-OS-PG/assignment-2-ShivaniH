CXX = g++

default: tracker

tracker: tracker.o vaninet.o 
	$(CXX) -o tracker tracker.o vaninet.o 

tracker.o: tracker.cpp vaninet.cpp vaninet.hpp
	$(CXX) -c tracker.cpp vaninet.cpp

vaninet.o: vaninet.cpp
	$(CXX) -c vaninet.cpp