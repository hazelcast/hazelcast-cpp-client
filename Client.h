#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "hazelcast/client/serialization/Data.h"
#include <iostream>
#include <cstdlib>
#include <vector>


using namespace hazelcast::client::serialization;
typedef unsigned char byte;
    
/// Downloads stock quote information from a server.
class client
{
public:
    /// Constructor starts the asynchronous connect operation.
    client(boost::asio::io_service& io_service,
           const string& host, const string& port,SerializationService* service)
    : mSocket(io_service),service(service)
    {
        // Resolve the host name into an IP address.
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(host, port);
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        std::cout << "Trying to connect to server " << std::endl;
        boost::asio::connect(mSocket, endpoint_iterator);
        std::cout << "Connection established" << std::endl;

    }
    void read(Data& data){

        Array<byte> buffer(1024);
        boost::asio::read(mSocket, boost::asio::buffer(buffer.buffer,1024));
        DataInput input(buffer,service);
        data.readData(input);
    }

private:
    /// The connection to the server.
    boost::asio::ip::tcp::socket mSocket;
    SerializationService* service;
};



#endif