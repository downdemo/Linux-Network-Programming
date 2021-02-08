$(shell if [ -e build ]; then rm -rf build; fi; mkdir build)
$(shell if [ -e debug ]; then rm -rf debug; fi; mkdir debug)

all: udp_server udp_client \
	socket_server socket_client \
	fork_server fork_client \
	select_server select_client \
	poll_server poll_client \
	epoll_server epoll_client \
	socket_server_debug socket_client_debug \

udp_server: src/udp/server.cpp
	g++ -std=c++11 -Wall -O3 src/udp/server.cpp -o build/udp_server

udp_client: src/udp/client.cpp
	g++ -std=c++11 -Wall -O3 src/udp/client.cpp -o build/udp_client

socket_server: src/socket/server.cpp
	g++ -std=c++11 -Wall -O3 src/socket/server.cpp -o build/socket_server

socket_client: src/socket/client.cpp
	g++ -std=c++11 -Wall -O3 src/socket/client.cpp -o build/socket_client

fork_server: src/fork/server.cpp
	g++ -std=c++11 -Wall -O3 src/fork/server.cpp -o build/fork_server

fork_client: src/fork/client.cpp
	g++ -std=c++11 -Wall -O3 src/fork/client.cpp -o build/fork_client

select_server: src/select/server.cpp
	g++ -std=c++11 -Wall -O3 src/select/server.cpp -o build/select_server

select_client: src/select/client.cpp
	g++ -std=c++11 -Wall -O3 src/select/client.cpp -o build/select_client

poll_server: src/poll/server.cpp
	g++ -std=c++11 -Wall -O3 src/poll/server.cpp -o build/poll_server

poll_client: src/poll/client.cpp
	g++ -std=c++11 -Wall -O3 src/poll/client.cpp -o build/poll_client

epoll_server: src/epoll/server.cpp
	g++ -std=c++11 -Wall -O3 src/epoll/server.cpp -o build/epoll_server

epoll_client: src/epoll/client.cpp
	g++ -std=c++11 -Wall -O3 src/epoll/client.cpp -o build/epoll_client

socket_server_debug: src/socket/server.cpp
	g++ -std=c++11 -g src/socket/server.cpp -o debug/socket_server_debug

socket_client_debug: src/socket/client.cpp
	g++ -std=c++11 -g src/socket/client.cpp -o debug/socket_client_debug

clean:
	rm -rf build
	rm -rf debug
