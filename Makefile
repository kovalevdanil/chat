all: server, app

server:
	g++ chatserv.cpp -o chatserv

app:
	g++ application.cpp -pthread -o app
