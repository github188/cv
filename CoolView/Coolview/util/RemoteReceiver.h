#ifndef REMOTE_RECEIVER_H
#define REMOTE_RECEIVER_H

#include <string>
#include <map>
#include <vector>


//��¼����ý������Ŀ���Ա����Ϣ
struct ReceiverInfo
{
	std::string MemberURI;   //��Աuri
	std::string recvIP;		 //��Ա��IP��ַ
	int videoPort;		//��Ƶ���ݴ���˿�
	int audioPort;		//��Ƶ���ݴ���˿�
	bool smallVideo;    //�Ƿ�С��
	int virtualIndex;   //���ط��ͽ��̵�����������

	ReceiverInfo():
		videoPort(0),
		audioPort(0),
		smallVideo(false),
		virtualIndex(0)
	{}
};


class ReceiverInfoManager
{
	typedef std::map<int, ReceiverInfo *> VirtualReceiverMap;
	typedef std::map<std::string, VirtualReceiverMap> RemoteReceiverMap;
	RemoteReceiverMap _map;

public:

	/**
	 * @brief ���һ���µ�Զ�̽��ն���Ϣ
	 * @param virtualIndex ���ط����������ն�����
	 * @param sipUri Զ���ն�Uri
	 * @param remoteIPAddr Զ���ն�IP
	 * @param viedoPort Ŀ����Ƶ�˿�
	 * @param audioPort Ŀ����Ƶ�˿�
	 * @param smallVideo �Ƿ�ΪС��
	 */
	void AddRemoteReceiver( const int &virtualIndex, const std::string& sipURI, const std::string&remoteIPAddr, 
		const int&videoPort, const int&audioPort, const bool smallVideo)
	{
		//���Ȳ����Ƿ���ڶ�Ӧ�Ľ��ն�
		ReceiverInfo* receiverInfo = FindRemoteReceiverByURI( virtualIndex, sipURI );
		if( receiverInfo != NULL )
		{
			//������ڶ�Ӧ�Ľ��նˣ���ô�������Ĳ���
			receiverInfo->recvIP = remoteIPAddr;
			receiverInfo->videoPort = videoPort;
			receiverInfo->audioPort = audioPort;
			receiverInfo->smallVideo = smallVideo;
		}
		else
		{
			//��������ڶ�Ӧ�Ľ��նˣ���ô�½�һ��
			receiverInfo = new ReceiverInfo();
			receiverInfo->MemberURI = sipURI;
			receiverInfo->recvIP = remoteIPAddr;
			receiverInfo->videoPort = videoPort;
			receiverInfo->audioPort = audioPort;
			receiverInfo->smallVideo = smallVideo;
			receiverInfo->virtualIndex = virtualIndex;
			_map[sipURI][virtualIndex] = receiverInfo;
		}
		
	}

	/**
	 * @brief ɾ�����ձ��ն˵�Զ���ն˵���Ϣ
	 */
	void RemoveRemoteReceiverByURI( const int virtualIndex, const std::string&sipURI )
	{
		if (_map.find(sipURI) == _map.end())
		{
			return;
		}
		VirtualReceiverMap::iterator it = _map[sipURI].find(virtualIndex);
		if (it != _map[sipURI].end())
		{
			delete it->second;
			_map[sipURI].erase(it);
		}
	}

	/**
	 * @brief ɾ�����ձ��ն˵�Զ���ն˵���Ϣ��ע�⣬�����ڶ����ͬIP�Ľ�������Ϣʱ��ɾ����Ϊ��ȷ��
	 */
	void RemoveRemoteReceiverByIP( const int virtualIndex, const std::string&ip )
	{
		for(RemoteReceiverMap::iterator it = _map.begin(); it != _map.end(); ++it)
		{
			if (it->second.size() == 0)
			{
				continue;
			}
			VirtualReceiverMap::iterator it2 = it->second.find(virtualIndex);
			if( it2 != it->second.end() && (it2->second)->recvIP == ip)
			{
				delete it2->second;
				it->second.erase(it2);
				return;
			}
		}
	}

	/**
	 * @brief ����sipURI���ҽ��յ�ǰ�ն˵�Զ���ն˵���Ϣ
	 * @param receiverList �����ն��б�
	 * @param sipURI ���ҵ�SIP URI
	 * @return ����ҵ����򷵻���ص���Ϣ���󣬷��򷵻ؿ�ָ��
	 */
	ReceiverInfo* FindRemoteReceiverByURI( const int virtualIndex, const std::string&sipURI )
	{
		if (_map.find(sipURI) == _map.end())
		{
			return NULL;
		}
		VirtualReceiverMap::iterator it = _map[sipURI].find(virtualIndex);
		if (it == _map[sipURI].end())
		{
			return NULL;
		}
		return (it->second);
	}


	/**
	 * @brief ����Զ��IP���ҽ��յ�ǰ�ն˵�Զ���ն˵���Ϣ
	 * @param receiverList �����ն��б�
	 * @param ip ���ҵ�Զ���ն�IP��ַ
	 * @return ����ҵ����򷵻���ص���Ϣ���󣬷��򷵻ؿ�ָ��
	 */
	ReceiverInfo * FindRemoteReceiverByIP( const int virtualIndex, const std::string&ip )
	{
		for(RemoteReceiverMap::iterator it = _map.begin(); it != _map.end(); ++it)
		{
			if (it->second.size() == 0)
			{
				continue;
			}
			VirtualReceiverMap::iterator it2 = it->second.find(virtualIndex);
			if( it2 != it->second.end() && it2->second->recvIP == ip)
			{
				return (it2->second);
			}
		}
		return NULL;
	}

	int ReceiverCount() const
	{
		int count = 0;
		for(RemoteReceiverMap::const_iterator it = _map.cbegin(); it != _map.cend(); ++it)
		{
			if (it->second.size() > 0)
			{
				++count;
			}
		}
		return count;
	}

	typedef std::vector<ReceiverInfo> InfoList;
	InfoList ToInfoList() const
	{
		InfoList list;
		for(RemoteReceiverMap::const_iterator it = _map.cbegin(); it != _map.cend(); ++it)
		{
			for (VirtualReceiverMap::const_iterator it2 = it->second.cbegin();
				 it2 != it->second.cend(); ++it2)
			{
				list.push_back(*(it2->second));
			}
		}
		return list;
	}

};


#endif