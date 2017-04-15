#include "cGameServer.h"
#include "winsock2.h"
#include "Buffer.h"
#include <iostream>

cGameServer::cGameServer(std::string ipAddress, int numPlayers)
{
	m_ipAddress = ipAddress;
	m_numPlayers = numPlayers;
}

cGameServer::~cGameServer()
{

}

//void cGameServer::broadcastToRoom(std::string message, int sender)
//{
//	int resultInt = 0;
//	int bytesLeftToSend;
//	Buffer sendBuf(0);
//	int packetLength = 2 + 2  + 2 + 2 + this->mName.length() + 2 + message.length();
//	sendBuf.writeInt16BE(packetLength);	//totalpacketsize = packetLength + messageID + name + roomLength + roomName + messageLength + message
//	sendBuf.writeInt16BE(MESSAGE_ID_MESSAGE);	//messageID
//	sendBuf.writeInt16BE(sender);	//socket of sender
//	sendBuf.writeStringBE(mName);	//the room name itself
//	sendBuf.writeStringBE(message);
//
//	const char* sendArray = sendBuf.toCharArray();
//
//	bytesLeftToSend = packetLength;
//	for (int index = 0; index != mMemberSockets.size(); index++)
//	{
//		while (bytesLeftToSend > 0)
//		{
//			if (mMemberSockets[index] == sender)
//			{
//				break;
//			}
//			resultInt = send(mMemberSockets[index], sendArray, packetLength, 0);
//			if (resultInt < 0)
//			{
//				break;
//			}
//
//			bytesLeftToSend -= resultInt;
//			sendArray += resultInt;
//		}
//		std::cout << packetLength << " Sent to: " << mMemberSockets[index] << std::endl;
//		bytesLeftToSend = packetLength;
//		sendArray = sendBuf.toCharArray();
//		
//	}
//}
//
//void cGameServer::broadcastJoinToRoom(int newMember)
//{
//	int resultInt = 0;
//	int bytesLeftToSend;
//	Buffer sendBuf(0);
//	std::string message = std::to_string(newMember) + " has joined the room.";
//	int packetLength = 2 + 2 + 2 + this->mName.length() + 2 + message.length();
//	sendBuf.writeInt16BE(packetLength);	//totalpacketsize = packetLength + messageID + roomLength + roomName + messageLength + message
//	sendBuf.writeInt16BE(MESSAGE_ID_JOIN);	//messageID
//	sendBuf.writeStringBE(mName);	//the room name itself
//	sendBuf.writeStringBE(message);
//
//	const char* sendArray = sendBuf.toCharArray();
//
//	bytesLeftToSend = packetLength;
//	for (int index = 0; index != mMemberSockets.size(); index++)
//	{
//		while (bytesLeftToSend > 0)
//		{
//			if (mMemberSockets[index] == newMember)
//			{
//				break;
//			}
//			resultInt = send(mMemberSockets[index], sendArray, packetLength, 0);
//			if (resultInt < 0)
//			{
//				break;
//			}
//			
//			bytesLeftToSend -= resultInt;
//			sendArray += resultInt;
//		}
//		bytesLeftToSend = packetLength;
//		sendArray = sendBuf.toCharArray();
//	}
//}
//
//void cGameServer::broadcastLeaveRoom(int oldMember)
//{
//	int resultInt = 0;
//	int bytesLeftToSend;
//	Buffer sendBuf(0);
//	std::string message = std::to_string(oldMember) + " has left the room.";
//	int packetLength = 2 + 2 + 2 + this->mName.length() + 2 + message.length();
//	sendBuf.writeInt16BE(packetLength);	//totalpacketsize = packetLength + messageID + roomLength + roomName + messageLength + message
//	sendBuf.writeInt16BE(MESSAGE_ID_LEAVE);	//messageID
//	sendBuf.writeStringBE(mName);	//the room name itself
//	sendBuf.writeStringBE(message);
//
//	const char* sendArray = sendBuf.toCharArray();
//
//	bytesLeftToSend = packetLength;
//	for (int index = 0; index != mMemberSockets.size(); index++)
//	{
//		while (bytesLeftToSend > 0)
//		{
//			resultInt = send(mMemberSockets[index], sendArray, packetLength, 0);
//			if (resultInt < 0)
//			{
//				break;
//			}
//
//			bytesLeftToSend -= resultInt;
//			sendArray += resultInt;
//		}
//		bytesLeftToSend = packetLength;
//		sendArray = sendBuf.toCharArray();
//	}
//
//	removeMember(oldMember);
//}
//
//void cGameServer::addMember(int memberSocket)
//{
//	mMemberSockets.push_back(memberSocket);
//}
//
//void cGameServer::removeMember(int memberSocket)
//{
//	for (int index = 0; index != mMemberSockets.size(); index++)
//	{
//		if (memberSocket == mMemberSockets[index])
//		{
//			mMemberSockets.erase(mMemberSockets.begin() + index);
//			break;
//		}
//	}
//	mMemberSockets.shrink_to_fit();
//}

std::string cGameServer::getIPAddress()
{
	return m_ipAddress;
}

int cGameServer::getNumPlayers()
{
	return m_numPlayers;
}