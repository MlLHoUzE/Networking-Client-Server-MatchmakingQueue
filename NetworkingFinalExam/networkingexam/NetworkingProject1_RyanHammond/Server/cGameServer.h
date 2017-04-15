#ifndef _cGameServer_HG_
#define _cGameServer_HG_

#include <string>
#include <vector>

class cGameServer
{
public:
	cGameServer(std::string ipAddress, int numPlayers);
	~cGameServer();

	/*void broadcastToRoom(std::string message, int sender);
	void broadcastJoinToRoom(int newMember);
	void broadcastLeaveRoom(int oldMember);
	void addMember(int memberSocket);
	void removeMember(int memberSocket);*/
	std::string getIPAddress();
	int getNumPlayers();

private:
	std::string m_ipAddress;
	int m_numPlayers;
};

#endif