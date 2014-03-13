//g++ chat.cc -o chat -lws2_32 to compile

#include <winsock2.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
std::string ip_addr = "127.0.0.1";

using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::stringstream;
sockaddr sockAddrClient;
bool startListening(SOCKET &s, unsigned long addy, int port);
bool connectToHost(SOCKET &theSocket, const char* addy, int portnum);
long WINAPI Thread(SOCKET *s);
bool validated(string &s);
int main()
{
	srand(time(NULL));
	SOCKET serv;
	SOCKET c[2];
	if(!startListening(serv, INADDR_ANY, 6500))
		cout << "something went wrong, not listening." << endl;
	else
		cout<< "Listening for a connection..." << endl;
	cout << "What IP are you connecting to? Enter a valid IPv4 address.\n > ";
	cin >> ip_addr;
	/*while(validated(ip_addr)){
		cout << "Invalid IP address, try again. \n > ";
		cin >> ip_addr;
	}*/
	if(!connectToHost(c[0], ip_addr.c_str(), 6500))
		cout << "failed to connect to host!"<< endl;
	else
		cout << "Connected to host!"<<endl;
	SOCKET TempSock=SOCKET_ERROR;
	while(TempSock==SOCKET_ERROR){
		TempSock=accept(serv,NULL,NULL);
		cout << "serv seems to be invalid..." << endl;
	}
	
	cout << "serv is legitimized now." << endl;
	serv = TempSock;
	u_long iMode=1;
	ioctlsocket(serv,FIONBIO,&iMode);
	
	
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread, &serv, 0, 0);
	while(1){
		
		string outbound,temp;
		cout << "You: ";
		getline(cin,outbound);
		send(c[0],outbound.c_str(),outbound.length()+1,0);
		
		int nError=WSAGetLastError();
		if(nError!=WSAEWOULDBLOCK&&nError!=0)
		{
			cout<<"Winsock error code: "<< nError <<endl;
			cout<<"Client disconnected!\r\n";
			// Shutdown our socket
			shutdown(serv,SD_SEND);
			// Close our socket entirely
			closesocket(serv);
			break;
		}
		Sleep(1);
	}
	if (serv == INVALID_SOCKET){
		int nret = WSAGetLastError();
		cout<<"invalid socket";
		WSACleanup();
	}
	std::cin.ignore();
	closesocket(serv);
	WSACleanup();
	return 0;
}

bool startListening(SOCKET &s, unsigned long addy, int port){
	WORD sockVersion;
	WSADATA wsaData;
	int nret;
	sockVersion = MAKEWORD(2, 2);
	// We begin by initializing Winsock
	WSAStartup(sockVersion, &wsaData);
	// Next, create the listening socket
	
	s = socket(AF_INET,		// Go over TCP/IP
			         SOCK_STREAM,   	// This is a stream-oriented socket
				 IPPROTO_TCP);		// Use TCP rather than UDP
	if (s == INVALID_SOCKET){
		cout << "s is an invalid socket, cleaning up now." << endl;
		nret = WSAGetLastError();		// Get a more detailed error
		WSACleanup();				// Shutdown Winsock
		return false;
	}
	// Use a SOCKADDR_IN struct to fill in address information
	SOCKADDR_IN serverInfo;
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = addy /*inet_addr("ip_addr");INADDR_ANY*/;
	serverInfo.sin_port = htons(port);
	// Bind the socket to our local server address
	nret = bind(s, (SOCKADDR*)&serverInfo, sizeof(struct sockaddr));
	if (nret == SOCKET_ERROR){
		nret = WSAGetLastError();
		cout << "socket error, cleaning up now." << endl;
		WSACleanup();
		return false;
	}
	// Make the socket listen
	u_long iMode=1;
	//ioctlsocket(s,FIONBIO,&iMode);	
	nret = listen(s, 10);		// Up to 10 connections 
	
	if (nret == SOCKET_ERROR){
		nret = WSAGetLastError();
		cout << "socket error, cleaning up now." << endl;
		WSACleanup();
		return false;
	}
	return true;
}
bool connectToHost(SOCKET &theSocket, const char* addy, int portnum){
	WORD sockVersion;
	WSADATA wsaData;
	int nret = SOCKET_ERROR;
	sockVersion = MAKEWORD(2, 2);
	
	WSAStartup(sockVersion, &wsaData);
	
	LPHOSTENT hostEntry;
	hostEntry = gethostbyname(addy);
	
	if (!hostEntry){
		nret = WSAGetLastError();
		cout << "no host entry, cleaning up now"<<endl;
		WSACleanup();
		return false;
	}
	
	theSocket = socket(AF_INET,			// Go over TCP/IP
			   SOCK_STREAM,			// stream-oriented socket
			   IPPROTO_TCP);		// TCP
	if (theSocket == INVALID_SOCKET){
		nret = WSAGetLastError();
		cout << "invalid socket, cleaning up now"<<endl;
		WSACleanup();
		return false;
	}
	
	//fill in socket info
	SOCKADDR_IN serverInfo;
	serverInfo.sin_family = AF_INET;
	
	serverInfo.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);
	serverInfo.sin_port = htons(portnum);		
	
	//actually do the connect stuff
	while(nret == SOCKET_ERROR){
	nret = connect(theSocket,
		       (LPSOCKADDR)&serverInfo,
		       sizeof(struct sockaddr));
	}
	if (nret == SOCKET_ERROR){
		nret = WSAGetLastError();
		cout << "socket error, cleaning up now"<<endl;
		WSACleanup();
		return false;
	}

	return true;
}
long WINAPI Thread(SOCKET *s){
	while(1){
		char buffer[1000];
		memset(buffer,'\0',1000);
		int msglength1 = recv(*s, buffer, 1000, 0);
		if(msglength1 > 0){
			cout << "\rThem: "<<buffer<<endl << "You: ";
		}
	}
}