main: main.cpp imap.o requests.o debug.o
	mkdir -p ../bin
	g++ main.cpp imap.o requests.o debug.o -o ../bin/main `pkg-config libcurl fuse --cflags --libs` -std=c++11 -Wall -Wextra -Wconversion

imap.o: imap.cpp imap.h
	g++ -c imap.cpp -lcurl -std=c++11 -Wall -Wextra -Wconversion

requests.o: requests.cpp imap.h
	g++ -c requests.cpp -lcurl -std=c++11 -Wall -Wextra -Wconversion

debug.o: debug.cpp debug.h
	g++ -c debug.cpp -std=c++11 -Wall -Wextra -Wconversion

ARGS = --help
run:
	../bin/main ${ARGS}

clean:
	rm *.o