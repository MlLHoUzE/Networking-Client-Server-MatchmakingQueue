#include <winsock2.h>


struct sClientInfo
{
	int mySocket;
	std::string playerID;
	int rating;
	bool bIsQueued;
};