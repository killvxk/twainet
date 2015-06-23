#include <Winsock2.h>
#include "inet_headers.h"

/***********************************************************************************************/
/*                                 EtherNetContainer                                           */
/***********************************************************************************************/
EtherNetContainer::EtherNetContainer()
	: m_child(0){}
EtherNetContainer::EtherNetContainer(const std::string& srcmac, const std::string& dstmac)
	: m_child(0)
{
	for(int i = 0, pos = 0; i < 6; srcmac.size() >= 17 && dstmac.size() >= 17 && i++, pos += 3)
	{
		char mactemp[3] = {0};
		memcpy(mactemp, srcmac.c_str() + pos, 2);
		m_ethHeader.ether_shost[i] = (unsigned char)strtol(mactemp, NULL, 16);
		memcpy(mactemp, dstmac.c_str() + pos, 2);
		m_ethHeader.ether_dhost[i] = (unsigned char)strtol(mactemp, NULL, 16);
	}
}

EtherNetContainer::~EtherNetContainer()
{
	if(m_child)
	{
		delete m_child;
	}
}


bool EtherNetContainer::serialize(char* data, int len)
{
	std::string sdata(data, len);
	return SerializeData(sdata) != 0;
}

bool EtherNetContainer::deserialize(char* data, int& len)
{
	std::string sdata;
	int deserialLen = DeserializeData(sdata);
	if(len < deserialLen)
	{
		len = deserialLen;
		return false;
	}

	memcpy(data, sdata.c_str(), len);
	return true;
}

int EtherNetContainer::SerializeData(const std::string& data)
{
	if(data.size() < sizeof(m_ethHeader))
		return 0;
	memcpy(&m_ethHeader, data.c_str(), sizeof(m_ethHeader));
	m_ethHeader.ether_type = htons(m_ethHeader.ether_type);
	return sizeof(m_ethHeader);
}

int EtherNetContainer::DeserializeData(std::string& data)
{
	data.resize(sizeof(m_ethHeader));
	ether_header ethHeader = m_ethHeader;
	ethHeader.ether_type = htons(ethHeader.ether_type);
	memcpy((void*)data.c_str(), &ethHeader, sizeof(m_ethHeader));
	return sizeof(m_ethHeader);
}

/***********************************************************************************************/
/*                                    PPPoEContainer                                           */
/***********************************************************************************************/
PPPoEContainer::PPPoEContainer()
{
	m_pppoeHeader.version_type = PPPOE_VERTYPE;
	m_pppoeHeader.sessionId = 0;
	m_pppoeHeader.code = 0;
	m_pppoeHeader.payload = 0;
}
PPPoEContainer::PPPoEContainer(const std::string& srcmac, const std::string& dstmac, unsigned char code)
	: EtherNetContainer(srcmac, dstmac)
{
	m_pppoeHeader.version_type = PPPOE_VERTYPE;
	m_pppoeHeader.sessionId = 0;
	m_pppoeHeader.code = code;
	m_pppoeHeader.payload = 0;
}
PPPoEContainer::~PPPoEContainer(){}

int PPPoEContainer::SerializeData(const std::string& data)
{
	int serialLen = EtherNetContainer::SerializeData(data);
	if(serialLen != 0 ||
		data.size() < serialLen + sizeof(m_pppoeHeader))
	{
		return 0;
	}
	memcpy(&m_pppoeHeader, data.c_str() + serialLen, sizeof(m_pppoeHeader));
	m_pppoeHeader.payload = htons(m_pppoeHeader.payload);
	return serialLen + sizeof(m_pppoeHeader);
}

int PPPoEContainer::DeserializeData(std::string& data)
{
	std::string ethdata;
	if(!EtherNetContainer::DeserializeData(ethdata))
	{
		return 0;
	}
	data.resize(ethdata.size() + sizeof(m_pppoeHeader));
	memcpy((void*)data.c_str(), ethdata.c_str(), ethdata.size());
	pppoe_header pppoeheader = m_pppoeHeader;
	pppoeheader.payload = htons(pppoeheader.payload);
	memcpy((void*)(data.c_str() + ethdata.size()), &pppoeheader, sizeof(pppoeheader));
	return data.size();
}

/***********************************************************************************************/
/*                                   PPPoEDContainer                                           */
/***********************************************************************************************/
PPPoEDContainer::PPPoEDContainer()
{
	m_ethHeader.ether_type = ETHERTYPE_PPPOED;
}

PPPoEDContainer::PPPoEDContainer(const std::string& srcmac, const std::string& dstmac, unsigned char code)
	: PPPoEContainer(srcmac, dstmac, code)
{
	m_ethHeader.ether_type = ETHERTYPE_PPPOED;
}

PPPoEDContainer::~PPPoEDContainer()
{
}

int PPPoEDContainer::SerializeData(const std::string& data)
{
	int serialLen = EtherNetContainer::SerializeData(data);
	if(serialLen != 0||
		data.size() < serialLen + m_pppoeHeader.payload)
	{
		return false;
	}
	
	char* sdata = (char*)data.c_str() + serialLen;
	int pos = 0;
	while(pos < m_pppoeHeader.payload)
	{
		unsigned short type;
		unsigned short len;

		if(pos + 2 > m_pppoeHeader.payload)
			break;
		memcpy(&type, sdata + pos, 2);
		pos += 2;

		if(pos + 2 > m_pppoeHeader.payload)
			break;
		memcpy(&len, sdata + pos, 2);
		pos += 2;
		
		type = htons(type);
		len = htons(len);

		if(pos + len > m_pppoeHeader.payload)
			break;
		std::string value(len + 1, 0);
		memcpy((char*)value.c_str(), sdata + pos, len);
		m_tags.insert(std::make_pair(type, value));
	}

	return serialLen + m_pppoeHeader.payload;
}

int PPPoEDContainer::DeserializeData(std::string& data)
{
	m_pppoeHeader.payload = 0;
	for(std::map<int, std::string>::iterator it = m_tags.begin();
		it != m_tags.end(); it++)
	{
		m_pppoeHeader.payload += sizeof(pppoe_tag) + it->second.size();
	}

	std::string pppoedata;
	if(!PPPoEContainer::DeserializeData(pppoedata))
	{
		return 0;
	}
	data.resize(pppoedata.size() + m_pppoeHeader.payload);
	memcpy((void*)data.c_str(), pppoedata.c_str(), pppoedata.size());
	int pos = pppoedata.size();
	for(std::map<int, std::string>::iterator it = m_tags.begin();
		it != m_tags.end(); it++)
	{
		pppoe_tag tag = {htons(it->first), htons(it->second.size())};
		memcpy((void*)(data.c_str() + pos), &tag, sizeof(tag));
		pos += sizeof(tag);
		memcpy((void*)(data.c_str() + pos), it->second.c_str(), it->second.size());
		pos += it->second.size();
	}
	return pppoedata.size() + m_pppoeHeader.payload;
}