//
//  client.cpp
//  Deneme
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
/*
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <cstdlib>
#include <vector>

using namespace std;
typedef unsigned char byte;
namespace hazelcast {
    
    /// Downloads stock quote information from a server.
    class client
    {
    public:
        /// Constructor starts the asynchronous connect operation.
        client(boost::asio::io_service& io_service,
               const string& host, const string& port)
        : mSocket(io_service)
        {
            // Resolve the host name into an IP address.
            boost::asio::ip::tcp::resolver resolver(io_service);
            boost::asio::ip::tcp::resolver::query query(host, port);
            boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
            cout << "Trying to connect to server " << endl;
            boost::asio::connect(mSocket, endpoint_iterator);
            cout << "Connection established" << endl;

        }
        template<typename T>
        void read(T& data){
            byte localBuffer[1024];
            boost::asio::read(mSocket, boost::asio::buffer(localBuffer,1024));
//            ContextAwareDataInput dataInput(localBuffer);
//            data.readData(dataInput);
        }
        
    private:
        /// The connection to the server.
        boost::asio::ip::tcp::socket mSocket;
        
    };
}

int main2(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if (argc != 3)
        {
            cerr << "Usage: client <host> <port>" << endl;
            return 1;
        }
        
        boost::asio::io_service io_service;
        hazelcast::client client(io_service, argv[1], argv[2]);
        io_service.run();
        
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }
    
    return 0;
}
*/