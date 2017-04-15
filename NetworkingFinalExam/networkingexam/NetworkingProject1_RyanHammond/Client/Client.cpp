#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>

#include "Buffer.h"
#include "Utilities.h"


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

std::string playerID;
bool Connected;
bool bInQueue = false;
int rating;

void sendingThread(LPVOID ConnectSocket)
{
	Buffer buffer(0);

	while (Connected)
	{
		char selection;
		//Ask the user to input a message
		std::cin >> selection;
		std::cin.clear();
		fflush(stdin);
		//std::string message;
		//std::getline(std::cin, message);
		//short messageLength = (short)message.length();
		//eraseLeadingSpaces(message);
		//The Quit / Exit command
		if (selection == 'q' || selection == 'Q')
		{
			Connected = false;

			// shutdown the connection since no more data will be sent
			int resultInt = shutdown((SOCKET)ConnectSocket, SD_SEND);
			if (resultInt == SOCKET_ERROR)
			{
				std::cout << "shutdown failed with error: " << WSAGetLastError() << std::endl;
				closesocket((SOCKET)ConnectSocket);
				WSACleanup();
			}

			break;
		}
		else if (selection == '1' && !bInQueue)
		{

			//send queue message to matchmaking server
			int packetLength = 2 + 2 + 2 + playerID.length() + 2;//(messageLength(2) + messaggeID(2) + playerIDLength(2) + playerID(?) + rating(2)
			buffer.writeInt16BE(packetLength);	//packetsize
			buffer.writeInt16BE((int)buffer.Queue);	//message ID
			buffer.writeInt16BE(playerID.length());	//write the player Id length for reading out correct length
			buffer.writeStringBE(playerID);	//write the playerID string
			buffer.writeInt16BE(rating);	//write the players rating
			bInQueue = true;
			system("cls");
			std::cout << "You have joined the Queue." << std::endl << "Press 2 to leave the Queue." << std::endl;
			customSend(ConnectSocket, buffer, packetLength);
			
		}//end of Queue
		else if (selection = '2' && bInQueue)	//if deQueue command is read
		{
			
			int packetLength = 2 + 2 + 2 + playerID.length();	//packetLength(2) + messageID(2) + playerIDLength(2) + playerID(?)

			buffer.writeInt16BE(packetLength);
			buffer.writeInt16BE((int)buffer.deQueue);
			buffer.writeInt16BE(playerID.length());
			buffer.writeStringBE(playerID);
			customSend(ConnectSocket, buffer, packetLength);
			system("cls");
			std::cout << "You have been removed from the Queue." << std::endl << "Press 1 to join Queue again." << std::endl;				
		}//end of Leave
		buffer.clear();
	}
}

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;

	Buffer buffer(0);

    char recvbuf[DEFAULT_BUFLEN];
    int resultInt;
    int recvbuflen = DEFAULT_BUFLEN;

	std::cout << "Enter player ID: " << std::endl;
	std::cin >> playerID;
	system("cls");
	std::cout << "Enter player Rating: " << std::endl;
	
	std::cin >> rating;

	//Step 1
    // Initialize Winsock
    resultInt = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (resultInt != 0) 
	{
		std::cout << "WinSock Initalization failed" << std::endl;
		return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    resultInt = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if ( resultInt != 0 ) {
        std::cout << "Socket Initalization failed" << std::endl;
		WSACleanup();
        return 1;
    }

	std::cout << "Step 1: WinSock Initalized" << std::endl;

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) 
	{

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (ConnectSocket == INVALID_SOCKET) 
		{
            std::cout << "Socket failed with error: " << WSAGetLastError() <<std::endl;
            WSACleanup();
            return 1;
        }

        // Connect to server.
        resultInt = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

        if (resultInt == SOCKET_ERROR) 
		{
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

	//This is a global variable used in the thread
	Connected = true;

	std::cout << "Step 2: Socket Created" << std::endl; //Put this here so we only see the message once althought we may try multiple sockets
	std::cout << "Step 3: Connected to the server" << std::endl;

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) 
	{
		std::cout << "Unable to connect to server!" << std::endl;
        WSACleanup();
        return 1;
    }

	system("cls");
	std::cout << "Press 1 to begin Queue." << std::endl << "Press 2 to deQueue." << std::endl;

	//Start a thread that takes keyboard input and sends
	std::thread sendThread(sendingThread, (LPVOID)ConnectSocket); //LPVOID is a pointer to any data type. we can use it to pass in whatever data we need.
	
    // Receive until the peer closes the connection
	int currentBytes = 0;
    do 
	{
		
        resultInt = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		currentBytes += resultInt;
		if (resultInt > 0)
		{
			std::cout << "Bytes received: " << resultInt << std::endl;
			buffer.clear();
			buffer.loadBuffer(recvbuf, resultInt);

			//packet length
			int packetLength = buffer.readInt16BE();
			while (currentBytes < packetLength)
			{
				resultInt = recv(ConnectSocket, recvbuf, recvbuflen, 0);
				
				
				buffer.loadBuffer(recvbuf, resultInt);
				currentBytes += resultInt;
			}
			currentBytes = 0;
			//messageID
			int name;
			int messageID = buffer.readInt16BE();
			int stringLength;
			std::string playerID1, playerID2, gameServerIP;
			int messageLength;
			std::string message;
			switch (messageID)
			{
			case buffer.MatchFound: //packetLength + messageID + playerID1Length + playerId1 + playerID2Length + playerID2 + gameServerIPLength + GameServerIP
				//
				stringLength = buffer.readInt16BE();	//length of player1's ID
				playerID1 = buffer.readStringBE(stringLength);	//player1's ID
				stringLength = buffer.readInt16BE();	//length of player2's ID
				playerID2 = buffer.readStringBE(stringLength);	//Player2's ID
				stringLength = buffer.readInt16BE();	//length of gameServerIP
				gameServerIP = buffer.readStringBE(stringLength);	//gameServerIP
				system("cls");
				std::cout << "Match found on gameServer: " << gameServerIP << std::endl;

				break;
			}//end of switch
		}
        else if ( resultInt == 0 )
            std::cout << "Connection closed" << std::endl;

        else
			std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;

    } while( resultInt > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

	std::cout << "Step 5: Disconnect" << std::endl;

	//Keep the window open
	std::cout << "\nwaiting on exit";
	int tempInput;
	std::cin >> tempInput;

    return 0;
}
