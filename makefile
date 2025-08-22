CXX ?= g++

fileserver: main.cpp ./fileserver/fileserver.cpp ./threadpool/threadpool.cpp \
	./log/log.cpp ./utils/utils.cpp ./message/message.cpp ./event/myevent.cpp
	$(CXX) -std=c++11  $^ -lpthread  -o main

clean:
	rm  -r main