#include <iostream>
#include <vector>
#include <memory>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <powrprof.h>

#include <windows.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "powrprof.lib")

struct EthernetInfo
{
	static const size_t LEN_MAC = 6;
	static const size_t LEN_IPV4 = 4 * 4;

	BYTE* mac;
	char* ipv4;

	EthernetInfo()
	{
		mac = new BYTE[LEN_MAC];
		ipv4 = new char[LEN_IPV4];
	}
	~EthernetInfo()
	{
		delete[] mac;
		delete[] ipv4;
	}
};__attribute__((aligned(16)))

EthernetInfo* GetEthernetInfo()
{
	PIP_ADAPTER_INFO adapter_info;
	PIP_ADAPTER_INFO adapter = nullptr;
	const int IP_ADAPTER_INFO_SIZE = sizeof(IP_ADAPTER_INFO);
	ULONG out_buff_len = IP_ADAPTER_INFO_SIZE;

	adapter_info = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(IP_ADAPTER_INFO_SIZE));
	if (adapter_info == nullptr) 
		return nullptr;

	// if adapters more than 1 - realloc the memory
	if (GetAdaptersInfo(adapter_info, &out_buff_len) == ERROR_BUFFER_OVERFLOW) 
	{
		free(adapter_info);
		adapter_info = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(out_buff_len));
		if (adapter_info == nullptr) 
			return nullptr;
	}

	if (GetAdaptersInfo(adapter_info, &out_buff_len) == NO_ERROR) 
	{
		adapter = adapter_info;
		while (adapter)
		{
			// find the ethernet adapter
			if (adapter->Type == MIB_IF_TYPE_ETHERNET)
			{
				EthernetInfo* ret_val = new EthernetInfo();
				for (size_t i = 0; i < ret_val->LEN_MAC; ++i)
					ret_val->mac[i] = adapter->Address[i];

				for (size_t i = 0; i < ret_val->LEN_IPV4; ++i)
					ret_val->ipv4[i] = adapter->IpAddressList.IpAddress.String[i];
		
				free(adapter_info);
				return ret_val;
			}
			adapter = adapter->Next;
		}
	} 

	if (adapter_info)
		free(adapter_info);
	
	return nullptr;
}

bool Suspend()
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 

	// Get a token for this process. 
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
		return false; 

	// Get the LUID for the shutdown privilege. 
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 

	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

	// Get the shutdown privilege for this process. 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 

	if (GetLastError() != ERROR_SUCCESS) 
		return false; 

	// Suspend the system. 
	if (!SetSuspendState(TRUE, TRUE, TRUE))
		return false; 

	// Suspend was successful
	return true;
}

void Hide()
{
	HWND hwnd = GetConsoleWindow();
	Sleep(1);
	HWND owner = GetWindow(hwnd, GW_OWNER);
	if (owner == NULL) 
		ShowWindow(hwnd, SW_HIDE); // Windows 10
	else
		ShowWindow(owner, SW_HIDE); // Windows 11
}

bool IsMagicPacket(int packet_size, BYTE* data, BYTE* mac)
{
	// 6 bytes of 0xFF and 16 times mac adress (6 bytes) = 102 bytes
	const int MAGIC_PACKET_SIZE = 102;
	if (packet_size != MAGIC_PACKET_SIZE)
		return false;

	size_t i = 0;
	for (; i < 6; ++i)
		if (static_cast<BYTE>(data[i]) != 0xFF)
			return false;

	for (; i < MAGIC_PACKET_SIZE; i += 6)
		for (size_t j = 0; j < 6; ++j)
			if (static_cast<BYTE>(data[i + j]) != mac[j])
				return false;

	return true;
}

int main(int argc, char* argv[])
{
	Hide();

	const int PORT_NUM = 9;
	int error_status = 0;
	
	std::unique_ptr<EthernetInfo> ethernet_info(GetEthernetInfo());
	if (ethernet_info == nullptr)
		return 1;

	in_addr ip_to_num;
	error_status = inet_pton(AF_INET, ethernet_info->ipv4, &ip_to_num);
	if (error_status <= 0)
		return 1;

	// init sockets at all
	WSADATA data;
	error_status = WSAStartup(MAKEWORD(2, 2), &data);
	if (error_status)
		return 1;

	// create server socket for listening
	SOCKET server_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_socket == INVALID_SOCKET)
	{
		closesocket(server_socket);
		WSACleanup();
		return 1;
	}

	// converting info in format
	sockaddr_in server_info;
	ZeroMemory(&server_info, sizeof(server_info));

	server_info.sin_family = AF_INET;
	server_info.sin_addr = ip_to_num;
	server_info.sin_port = htons(PORT_NUM);

	// binding socket to ip and port
	error_status = bind(server_socket, reinterpret_cast<sockaddr*>(&server_info), sizeof(server_info));
	if (error_status)
	{
		closesocket(server_socket);
		WSACleanup();
		return 1;
	}

	const int BUFF_SIZE = 1024;
	std::vector<char> server_buffer(BUFF_SIZE);
	int packet_size = 0;

	for(;;)
	{
		// recive packets (no listen, it is udp)
		packet_size = recvfrom(server_socket, server_buffer.data(), server_buffer.size(), 0, NULL, NULL);
		if (packet_size == SOCKET_ERROR)
		{
			closesocket(server_socket);
			WSACleanup();
			return 1;
		}

		// checking for magic packet
		if (IsMagicPacket(packet_size, reinterpret_cast<BYTE*>(server_buffer.data()), reinterpret_cast<BYTE*>(ethernet_info->mac)))
			Suspend();
	}
	closesocket(server_socket);
	WSACleanup();
	return 0;

}