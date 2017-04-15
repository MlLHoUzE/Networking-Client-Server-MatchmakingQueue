#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "sClientInfo.h"
#include "cGameServer.h"
#include <vector>

#include "Buffer.h"
#include <ctime>

// for windows socket link
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

std::vector<sClientInfo*> g_vecClientInfo;
std::vector<cGameServer*> g_vecGameServers;
std::vector<sClientInfo*> g_vecQueue;


int main() 
{
	Buffer buffer(0);
	time_t timer;
	time(&timer);
	time_t curTime;
	float deltaTime;
	float queueTimer = 0;
	int maxDifference = 100;
	//Step 1
	//Initial winsock
	WSADATA wsaData; //Create a Windows Socket Application Data Object

	int resultInt;

	resultInt = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (resultInt != 0)
	{
		std::cout << "WinSock Initalization failed" << std::endl;
		return 1;
	}

	//Step 2
	//Create a Socket
	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints)); //Fills a block of memory with zeros
	hints.ai_family = AF_INET; //Unspecified so either IPv4 or IPv6 address can be returned
	hints.ai_socktype = SOCK_STREAM; //Stream must be specified for TCP
	hints.ai_protocol = IPPROTO_TCP; //Protocol is TCP
	hints.ai_flags = AI_PASSIVE;


	resultInt = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (resultInt != 0)
	{
		std::cout << "Socket Initalization failed" << std::endl;
		WSACleanup(); //will nicely kill our WinSock instance
		return 1;
	}

	std::cout << "Step 1: WinSock Initalized" << std::endl;

	//Copy the result object pointer
	ptr = result;

	SOCKET listeningSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (listeningSocket == INVALID_SOCKET)
	{
		std::cout << "Socket Initalization failed" << std::endl;
		freeaddrinfo(result); //free memory allocated to provent memory leaks
		WSACleanup(); //will nicely kill our WinSock instance
		return 1;
	}



	/*NEW STUFF OCT 3RD */

	//We need to switch the socket to be non blocking
	// If iMode!=0, non-blocking mode is enabled.
	u_long iMode = 1;
	ioctlsocket(listeningSocket, FIONBIO, &iMode);

	/****/


	std::cout << "Step 2: Socket Created" << std::endl;

	//Step3 
	//Bind the Socket
	
	resultInt = bind(listeningSocket, result->ai_addr, (int)result->ai_addrlen);

	if (listeningSocket == INVALID_SOCKET)
	{
		std::cout << "Socket binding failed" << std::endl;
		freeaddrinfo(result); //free memory allocated to provent memory leaks
		closesocket(listeningSocket); //Close the socket
		WSACleanup(); //will nicely kill our WinSock instance
		return 1;
	}

	freeaddrinfo(result); //Once bind is called the address info is no longer needed so free the memory allocated

	std::cout << "Step 3: Socket Bound" << std::endl;

	//Step 4
	//Listen for a client connection

	if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "Socket listening failed" << std::endl;
		closesocket(listeningSocket); //Close the socket
		WSACleanup(); //will nicely kill our WinSock instance
		return 1;
	}

	std::cout << "Step 4: Listening on Socket" << std::endl;


	/*NEW STUFF OCT 3RD */

	//Set up file descriptors for select statement
	fd_set master;    //Master list
	fd_set read_fds;  //Temp list for select() statement

					  //Clear the master and temp sets
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	//Add the listener to the master set
	FD_SET(listeningSocket, &master);

	//Keep track of the biggest file descriptor
	int newfd;        //Newly accepted socket descriptor
	int fdmax = listeningSocket; //Maximum file descriptor number (to start it's the listener)
	std::cout << "fdmax " << fdmax << std::endl;

	//Set timeout time
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000; // 500 ms

	bool firstMessage = true;
	int currentBytes = 0;
	//Main loop
	while (true) //infinite loop, this could be scary
	{
		time(&curTime);
		deltaTime = difftime(timer, curTime);

		queueTimer += deltaTime;
		//Select function checks to see if any socket has activity
		read_fds = master; // copy the master to a temp
		if (select(fdmax + 1, &read_fds, NULL, NULL, &tv) == -1)
		{
			std::cout << "Select Error" << std::endl;
			exit(4);
		}

		//Loop through existing connections looking for data to read
		for (int i = 0; i <= fdmax; i++)
		{
			//If no flag is raised keep looping
			if (!FD_ISSET(i, &read_fds))
			{
				continue;
			}

			//If the raised flag is on the listening socket accept a new connection then keep looping
			if (i == listeningSocket)
			{

				// handle new connections
				sockaddr_in client;
				socklen_t addrlen = sizeof sockaddr_storage; //this represents the client address size
				newfd = accept(listeningSocket, (struct sockaddr *)&client, &addrlen);
				std::cout << "Connected to: " << client.sin_addr.s_addr << std::endl;
				sClientInfo* temp = new sClientInfo;
				temp->mySocket = newfd;
				g_vecClientInfo.push_back(temp);
				std::cout << "newfd " << newfd << std::endl;
				if (newfd == -1)
				{
					std::cout << "Accept Error" << std::endl;
					continue;
				}

				FD_SET(newfd, &master); // add to master set

				//Keep track of max fd
				if (newfd > fdmax)
					fdmax = newfd;

				std::cout << "New connection on socket " << newfd << std::endl;
				continue;
			}

			/////////////////////////////////
			// Recieve an incoming message //
			/////////////////////////////////

			char recvbuf[DEFAULT_BUFLEN];
			int incomingMessageLength = 0;

			//Recieve the message
			incomingMessageLength = recv(i, recvbuf, sizeof recvbuf, 0);
			currentBytes += incomingMessageLength;
		

			if (incomingMessageLength > 0)
			{
				std::cout << "Bytes received: " << incomingMessageLength << std::endl;

				
				
				buffer.clear();
				buffer.loadBuffer(recvbuf, incomingMessageLength);

				//packet length
				int16_t packetLength = buffer.readInt16BE();
				while (currentBytes < packetLength)
				{
					resultInt = recv(i, recvbuf, sizeof recvbuf, 0);


					buffer.loadBuffer(recvbuf, resultInt);
					currentBytes += resultInt;
				}
				currentBytes = 0;
				//messageID
				int16_t messageID = buffer.readInt16BE();
				short messageLength;
				std::string message;
				int stringLength;
				int playersConnected;
				int rating;
				std::string IpAddress;
				std::string playerID;
				bool bRoomExists = false;
				cGameServer* curGameServer;

				switch (messageID)
				{
				case buffer.RegisterServer:	//packet format = packetLength-messageID-(int)playersConnected-(string)IpAddress
					playersConnected = buffer.readShort16BE();	//players connected			
					stringLength = buffer.readInt16BE();	//length of IPAddress
					IpAddress = buffer.readStringBE(stringLength);	//ipAddress
					curGameServer = new cGameServer(IpAddress, playersConnected);
					g_vecGameServers.push_back(curGameServer);
					break;
				case buffer.Queue:	//packetFormat = (int)packetLength-(int)messageID-(string)PlayerID-(int)Rating
					stringLength = buffer.readInt16BE();
					playerID = buffer.readStringBE(stringLength);
					rating = buffer.readInt16BE();
					for (int index = 0; index < g_vecClientInfo.size(); index++)	//find the client that wishes to queue and update their info.
					{
						if (g_vecClientInfo[index]->mySocket == i)
						{
							g_vecClientInfo[index]->playerID = playerID;
							g_vecClientInfo[index]->rating = rating;
							g_vecClientInfo[index]->bIsQueued = true;
						}
					}
					break;
				case buffer.deQueue:	//packetFormat = (int)packageLength-(int)MessageID-(string)PlayerID
					stringLength = buffer.readInt16BE();
					playerID = buffer.readStringBE(stringLength);
					//look for that player in the queue
					for (int index = 0; index < g_vecClientInfo.size(); index++)
					{
						if (g_vecClientInfo[index]->playerID == playerID)
						{
							//found the player, now remove from queue
							g_vecClientInfo[index]->bIsQueued = false;
						}
					}

					break;
				default:

					break;
				}
			}
			else if (incomingMessageLength == 0)
			{
				std::cout << "Connection closing..." << std::endl;
				closesocket(i); //CLose the socket
				FD_CLR(i, &master); // remove from master set

				continue;
			}
			else
			{
				std::cout << "recv failed: " << WSAGetLastError() << std::endl;
				closesocket(i);
				WSACleanup();

				//You probably don't want to stop the server based on this in the real world
				return 1;
			}
		}
		g_vecQueue.empty();
		//create queue vector
		for (int index = 0; index < g_vecClientInfo.size(); index++)
		{
			if (g_vecClientInfo[index]->bIsQueued)
			{
				g_vecQueue.push_back(g_vecClientInfo[index]);
			}
		}
		if (queueTimer > 30)	//every 30 seconds
		{
			maxDifference += 10;	//increase the maximum difference
			if (maxDifference >= 500)	//if the maximum difference if greater than 500
			{
				maxDifference = 500;	//limit it to 500
			}
			queueTimer = 0.f;	//reset the queueTimer
		}
		

		//*****************QUEUE*************************
		for (int index = 0; index < g_vecQueue.size(); index++)
		{
			for (int innerIndex = 0; innerIndex < g_vecQueue.size(); innerIndex++)
			{
				if (g_vecQueue[index]->rating - g_vecQueue[innerIndex]->rating <= maxDifference)
				{
					//find game server
				}
			}
		}
	}

	/****/


	//Step 7
	//Disconnect and Cleanup

	// shutdown the send half of the connection since no more data will be sent
	resultInt = shutdown(listeningSocket, SD_SEND);
	if (resultInt == SOCKET_ERROR)
	{
		std::cout << "shutdown failed:" << WSAGetLastError << std::endl;
		closesocket(listeningSocket);
		WSACleanup();
		return 1;
	}

	//Final clean up
	closesocket(listeningSocket);
	WSACleanup(); 

	std::cout << "Step 7: Disconnect" << std::endl;

	//Keep the window open
	std::cout << "\nwaiting on exit";
	int tempInput;
	std::cin >> tempInput;

	return 0;
}


std::string FindGameServer()
{
	cGameServer* curServer;
	int lowestPlayers = 0;
	for (int i = 0; i < g_vecGameServers.size(); i++)
	{
		if (g_vecGameServers[i]->getNumPlayers() < lowestPlayers)
		{
			curServer = g_vecGameServers[i];
		}
	}

	if (curServer != NULL)
	{
		return curServer->getIPAddress();
	}
}