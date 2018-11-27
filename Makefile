TARGET=multitype_queue
all:
	g++ -Wall -Wextra -pedantic -O2 -std=c++14 main.cpp -lpthread -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
