#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <sys/socket.h>

using socketfd = int;

class ConnectionID
{
public:
    ConnectionID(std::string_view hostName, int port): hostName_(hostName), port_(port) {}
    std::string_view getHostName() const { return hostName_;}
    int getPort() const { return port_; }
private:
    std::string hostName_;
    int port_;
};

class Connection {
public:
    enum class Status {CONNECTING, CONNECTED, NOTCONNECTED};
    enum class IpvType {IPV4, IPV6};
public:
    Connection(std::string_view host, int port, IpvType ipvType = IpvType::IPV4);
    ~Connection();
    Status connect();
    Status checkStatus();
    int send(const char *message, int length);
    int receive(char *message, int length);
    void disconnect();
    const ConnectionID& getID() const { return connId_; }
private:
    ConnectionID connId_;
    IpvType ipvType_;
    Status status_ = Status::NOTCONNECTED;
    socketfd fd_ = -1;
};

#endif