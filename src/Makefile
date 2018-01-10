temp: temp.cpp imap.o requests.o filetree.o
	g++ temp.cpp imap.o requests.o filetree.o -o temp -lcurl -std=c++11 -Wall -Wextra -Wconversion

main: main.cpp imap.o requests.o filetree.o debug.o
	g++ main.cpp imap.o requests.o filetree.o debug.o -o main `pkg-config libcurl fuse --cflags --libs` -std=c++11 -Wall -Wextra -Wconversion

imap.o: imap.cpp imap.h
	g++ -c imap.cpp -lcurl -std=c++11 -Wall -Wextra -Wconversion

requests.o: requests.cpp imap.h
	g++ -c requests.cpp -lcurl -std=c++11 -Wall -Wextra -Wconversion

filetree.o: filetree.cpp filetree.h
	g++ -c filetree.cpp -std=c++11 -Wall -Wextra -Wconversion

debug.o: debug.cpp debug.h
	g++ -c debug.cpp -std=c++11 -Wall -Wextra -Wconversion

clean:
	rm *.o