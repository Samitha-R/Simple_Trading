#include "Connection.h"
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <fcntl.h> 
#include <unistd.h>
#include <memory>
 
Connection::Connection(std::string_view host, int port, IpvType ipvType) : connId_(host, port), ipvType_(ipvType)
{

}

 Connection::~Connection()
 {
    disconnect();
 }

 void Connection::disconnect()
 {

    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }

    status_ = Status::NOTCONNECTED;
 }

Connection::Status Connection::connect()
{
    struct addrinfo hints, *addres = nullptr;

    // Prepare hints
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = (ipvType_ == IpvType::IPV4) ? AF_INET : AF_INET6;       // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP

    // Resolve host
    std::string port_str = std::to_string(connId_.getPort());

    if (getaddrinfo(connId_.getHostName().data(), port_str.c_str(), &hints, &addres) != 0) {
        std::cout << "getaddrinfo failed for host: " << connId_.getHostName() <<  " " << connId_.getPort() << std::endl;
        return status_;
    }
    
    std::unique_ptr<addrinfo, void(*)(addrinfo*)> res_ptr(addres, freeaddrinfo);

    // Create socket
    fd_ = socket(addres->ai_family, addres->ai_socktype, addres->ai_protocol);

    if (fd_ < 0) {
        perror("socket");
        return status_;
    }

    // Set non-blocking
    int flags = fcntl(fd_, F_GETFL, 0);

    if (flags < 0 || fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl");
        return status_;
    }

    // Try to connect
    int ret = ::connect(fd_, addres->ai_addr, addres->ai_addrlen);

    if (ret < 0) {
        if (errno == EINPROGRESS) {
            status_ = Status::CONNECTING;
        } else {
            perror("connect");
            close(fd_);
            fd_ = -1;
            return status_;
        }
    } else {
        status_ = Status::CONNECTED;
    }

    return status_;
}

Connection::Status Connection::checkStatus()
{
    if (status_ == Status::CONNECTING) {
        int error = 0;
        socklen_t len = sizeof(error);
        auto retVal = getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &len);

        if (retVal >= 0) {
            if (error == 0) {
                status_ = Status::CONNECTED;
            } else if ( error != EINPROGRESS) {
                disconnect();
                status_ = Status::NOTCONNECTED;
            }
        }
    }
    return status_;
}

int Connection::send(const char *message, int length)
{
    return ::send(fd_, message, length, 0);
}

int Connection::receive(char *message, int length)
{
    return ::recv(fd_, message, length, 0);
}