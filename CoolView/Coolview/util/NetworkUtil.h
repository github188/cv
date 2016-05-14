////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file	util\NetworkUtil.h
///
/// @brief ����һЩ����API
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <QStringList>

/**
 * @brief  ��������IP��ȡ���ص�ַ�����浽����pGateway�У�pGateway�ĳ�������Ϊ16.
 * @param pHostIP		����IP��ַ
 * @param pGateway	���ص�ַ����ַ���
 * @return  �����Ƿ�ɹ���ȡ���ص�ַ
 */
extern bool GetGatewayByHostIP(const char* pHostIP, char * pGateway);

/**
 * @brief ��ȡ����IP��ַ�б�
 * @param ��ַ�汾 AF_INET|AF_INET6
 */
extern bool GetIPList(int family , std::vector<std::string>* ipList );

/**
 * @brief �жϵ�ַ�Ƿ�ΪIPv6��ַ
 * @param ip ip��ַ
 */
extern bool IsIpv6Address(const std::string &ip);

/**
 * @brief �жϵ�ַ�Ƿ�ΪIPv4��ַ
 * @param ip ip��ַ
 */
extern bool IsIpv4Address(const std::string &ip);

/**
 * @brief �Ƚ�����IP��ַ����ͬ����true�����򷵻�false
 * @param address1 ip��ַ1
 * @param address2 ip��ַ2
 */
extern bool IsIPAddressEqual(const std::string &address1, const std::string &address2);

/**
 * @brief ��ȡ����ip��ַ������ipv4��ipv6�ĵ�ַ
 * @return ���ص�ַ�б�
 */
extern QStringList GetHostIPList();

/**
 * @brief ��ȡ����ip��ַ������ipv4��ipv6�ĵ�ַ
 * @param ipv4 ipv4��ַ
 * @param ipv6 ipv6��ַ
 */
extern void GetHostIPs(QString &ipv4, QString &ipv6);

/**
 * @brief ��ȡip��ַ����Ӧ��MAC��ַ
 * @return ����MAC��ַ
 */
extern QString GetMacAddress( const QString &ip_address );
