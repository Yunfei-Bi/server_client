// 导入相关的库
#include<iostream>
#include<WinSock2.h>
#include<string>
#include<set>
#pragma comment(lib, "Ws2_32.lib")

// 检查套接字是否在集合中
bool checkSock(const FD_SET messSet, SOCKET Now)
{
    for (u_int i=0; i<messSet.fd_count; i++)
    {
        if (Now == messSet.fd_array[i])
            return true;
    }
    return false;
}

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
    if (2 != HIBYTE(wdSockMeg.wVersion) || 2 != LOBYTE(wdSockMeg.wVersion))
    {
        std::cout << "Version number is incorrect." << std::endl;
        WSACleanup();
        return 0;
    }

    // 创建套接字
    SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == ServerSocket)
    {
        int a = WSAGetLastError();
        std::cout << "Error creating server socket" << std::endl;
        WSACleanup();
        return 0;
    }

    // 设置本地IP和Port
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    // 绑定本地IP和Port
    int bindMeg = bind(ServerSocket, (const sockaddr*)&server, sizeof(server));
    if (SOCKET_ERROR == bindMeg)
    {
        int a = WSAGetLastError();
        std::cout << "Binding error" << std::endl;
        WSACleanup();
    }

    // 设置监听
    int listenMeg = listen(ServerSocket, SOMAXCONN);
    if (SOCKET_ERROR == listenMeg)
    {
        int a = WSAGetLastError();
        std::cout << "Listen function error" << std::endl;
        closesocket(ServerSocket);
        WSACleanup();
    }

    // 设置select模型
    FD_SET allSocketSet;
    FD_ZERO(&allSocketSet);
    FD_SET(ServerSocket, &allSocketSet);
    FD_SET haveSendSocketSet;
    FD_ZERO(&haveSendSocketSet);

    // 开始循环
    while (1)
    {
        // 设置slelect模型的参数
        fd_set readSockets = allSocketSet;
        fd_set writeSockets = allSocketSet;
        fd_set errorSockets = allSocketSet;
        // 设置select模型的时间
        timeval serverTime;
        serverTime.tv_sec = 3;
        serverTime.tv_usec = 0;
        int selectMeg = select(0, &readSockets, &writeSockets, &errorSockets, &serverTime);
        // 如果没有就绪Socket
        if (0 == selectMeg)
        {
            continue;
        }
        // 如果有就绪Socket
        else if (selectMeg > 0)
        {
            // 处理错误Socket
            for (u_int i=0; i<errorSockets.fd_count; i++)
            {
                char str[100] = {0};
                int len = 99;
                if (SOCKET_ERROR == getsockopt(errorSockets.fd_array[i], SOL_SOCKET, SO_ERROR, str, &len))
                {
                    std::cout << "Unable to get error information" << std::endl;
                }
            }

            // 处理写入
            for (u_int i=0; i<writeSockets.fd_count; i++)
            {
                char Hello[40] = "You have successfully connected \n";
                if (!checkSock(haveSendSocketSet, writeSockets.fd_array[i]))
                {
                    int sRes = send(writeSockets.fd_array[i], Hello, strlen(Hello), 0);
                    if (SOCKET_ERROR == sRes)
                    {
                        int a = WSAGetLastError();
                    }
                    else
                    {
                        FD_SET(writeSockets.fd_array[i], &haveSendSocketSet);
                    }
                }
            }

            // 处理读取
            for (u_int i=0; i<readSockets.fd_count; i++)
            {
                // 与新的客户端建立连接
                if (readSockets.fd_array[i] == ServerSocket)
                {
                    sockaddr_in addrClient;
                    int len = sizeof(addrClient);
                    SOCKET socketClient = accept(ServerSocket, (sockaddr*)&addrClient, &len);
                    if (INVALID_SOCKET == socketClient)
                    {
                        continue;
                    }
                    std::cout << "Client with IP: " << inet_ntoa(addrClient.sin_addr) 
                                << " and port: " << ntohs(addrClient.sin_port)
                                << " has logged on "<< std::endl;
                    FD_SET(socketClient, &allSocketSet);
                }
                // 处理客户端消息
                else
                {
                    char recvBuf[1500] = {0};
                    int recvMeg = recv(readSockets.fd_array[i], recvBuf, 1500, 0);
                    // 等于0，客户端下线
                    if (0 == recvMeg)
                    {
                        SOCKET socketTemp = readSockets.fd_array[i];
                        FD_CLR(socketTemp, &allSocketSet);
                        closesocket(socketTemp);
                    }
                    // 大于0，接收到来自客户端消息
                    else if (0 < recvMeg) {
                        std::cout << "Received message: " << recvBuf << std::endl;
                    }
                    // 小于0，出错
                    else
                    {
                        int a = WSAGetLastError();
                        switch(a)
                        {
                            case 10054:
                            {
                                SOCKET socketTemp = readSockets.fd_array[i];
                                FD_CLR(socketTemp, &allSocketSet);
                                closesocket(socketTemp);
                            }
                        }
                    }
                }
            }
        }
        // 如果selectMeg < 0，报错
        else
        {
            std::cout << "An error occurred" << std::endl;
            break;
        }
    }

    for (u_int i=0; i<allSocketSet.fd_count; i++)
    {
        closesocket(allSocketSet.fd_array[i]);
    }
    WSACleanup();
    system("pause");
    return 0;
}