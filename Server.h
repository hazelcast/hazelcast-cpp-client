#ifndef SERVER_H
#define SERVER_H

#include "hazelcast/client/serialization/DataOutput.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>

using namespace hazelcast::client::serialization;
 /// Serves stock quote information to any client that connects to it.
 class server
 {
 public:
    /// Constructor opens the acceptor and starts waiting for the first incoming
    /// connection.
    server(boost::asio::io_service& io_service, unsigned short port, SerializationService* service)
    : mAcceptor(io_service,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , mSocket(io_service), service(service)
    {
        std::cout << "waiting for socket to accept" << std::endl;
        mAcceptor.accept(mSocket);
        cout << "connection accepted" << endl;
    }
    
    void send(Data& data){
        DataOutput out(service);
        data.writeData(out);
        std::vector<byte> buffer = out.toByteArray();
//        std::cout << buffer.length() << std::endl;
//        boost::asio::write(mSocket, boost::asio::buffer(buffer.buffer,1024));
    }
    private:
        /// The acceptor object used to accept incoming socket connections.
        boost::asio::ip::tcp::acceptor mAcceptor;
        boost::asio::ip::tcp::socket mSocket;
        SerializationService* service;
 };
 

#endif