////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file	network\NetworkUtil.h
///
/// @brief ����һЩ����API
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
using namespace std;

class NetworkUtil
{
public:
	/**
	 * @brief  ��������IP��ȡ���ص�ַ�����浽����pGateway�У�pGateway�ĳ�������Ϊ16.
	 * @param pHostIP		����IP��ַ
	 * @param pGateway	���ص�ַ����ַ���
	 * @return  �����Ƿ�ɹ���ȡ���ص�ַ
	 */
	static bool getGatewayByHostIP(const char* pHostIP, char * pGateway);

	/**
	 * @brief ��ȡ����IP��ַ�б�
	 * @param ��ַ�汾 AF_INET|AF_INET6
	 */
	static bool getIPList(int family , vector<string>* ipList );

	/**
	 * @brief ��ȡ����������IP��ַ
	 */
	static vector<string> getHostIPList();
};
