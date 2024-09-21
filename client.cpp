// 导入相关的库
#include<iostream>
#include<Winsock2.h>
#include<stdlib.h>
#include<string.h>
#include<WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

int main()
{
    // 初始化WinSock库，版本为2.2
    WORD wdVersion = MAKEWORD(2, 2);
    WSADATA wdSockMeg;
    int nRes = WSAStartup(wdVersion, &wdSockMeg);
    // 检查WinSock是否初始化成功
	if (nRes != 0) // 如果初始化失败
	{
		switch (nRes)
		{
		case WSASYSNOTREADY:
			std::cout << "Try restarting your computer, or check the network library." << std::endl;
			break;
		case WSAVERNOTSUPPORTED:
			std::cout << "Please update the network library." << std::endl;
			break;
		case WSAEINPROGRESS:
			std::cout << "Please restart." << std::endl;
			break;
		case WSAEPROCLIM:
			std::cout << "Please close some programs to free up resources for this application." << std::endl;
			break;
		}
	}
    // 检查WinSock版本是否符合预期
	if (2 != HIBYTE(wdSockMeg.wVersion) || 2 != LOBYTE(wdSockMeg.wVersion)) // High byte for minor version, low byte for major version
	{
		// Indicates version mismatch
		// Clean up the network library
		WSACleanup();
		return 0;
	}

    // 创建套接字
    SOCKET socketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == socketServer)
    {
        int a = WSAGetLastError();
        WSACleanup();
        return 0;
    }

    // 设置本地IP和Port
    sockaddr_in serverMeg;
    serverMeg.sin_family = AF_INET;
    serverMeg.sin_port = htons(12345);
    serverMeg.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    // 客户端尝试与服务器建立连接
    int connectMeg = connect(socketServer, (sockaddr*)&serverMeg, sizeof(serverMeg));
    if (SOCKET_ERROR == connectMeg)
    {
        int a = WSAGetLastError();
        WSACleanup();
        return 0;
    }
    std::cout << "Connection successful" << std::endl;

    // 开始循环
    while (1)
    {
        char buf[1500] = {0};
        std::cout << "Please enter what you want to send: " << std::endl;
        scanf("%s", buf);
        int sres = send(socketServer, buf ,strlen(buf), 0);
        if (SOCKET_ERROR == sres)
        {
            int a = WSAGetLastError();
            std::cout << "Error sending from the client" << std::endl;
        }
        std::cout << "Send successfully, content sent: " << buf << std::endl;
        
        // 连接断开，服务器下线
        int res = recv(socketServer, buf, 1400, 0);
        if (0 == res)
        {
            std::cout << "Connection interrupted, going offline" << std::endl;
        }
        // 连接出错
        else if (SOCKET_ERROR == res)
        {
            int a = WSAGetLastError();
            std::cout << "Connection error occurred " << std::endl;
        }
        else
        {
            std::cout << "Received message from the server, bytes: "
                        << "Content: " << buf << std::endl;
        }
    }
    closesocket(socketServer);
    WSACleanup();
    getchar();
}