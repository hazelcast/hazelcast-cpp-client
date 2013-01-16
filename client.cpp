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


/*
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include "ContextAwareDataOutput.h"

using namespace std;

 namespace hazelcast {
 
 /// Serves stock quote information to any client that connects to it.
 class server
 {
 public:
 /// Constructor opens the acceptor and starts waiting for the first incoming
 /// connection.
 server(boost::asio::io_service& io_service, unsigned short port)
 : mAcceptor(io_service,
 boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
 , mSocket(io_service)
 {
 
 cout << "waiting for socket to accept" << endl;
 mAcceptor.accept(mSocket);
 cout << "connection accepted" << endl;
 }
 template <typename T>
 void send(const T& data){
 ContextAwareDataOutput *dataOutput;
 data.writeData(*dataOutput);
 boost::asio::write(mSocket, boost::asio::buffer(dataOutput->toString().c_str() , 1024));
 }
 private:
 /// The acceptor object used to accept incoming socket connections.
 boost::asio::ip::tcp::acceptor mAcceptor;
 boost::asio::ip::tcp::socket mSocket;
 };
 
 } // namespace hazelcast
 
int main(int argc, char* argv[])
{
    
    //    byte x = 138;
    //    printf("%x\n", 0xff & x);
    //    printf("%x\n", 0x0f & (x >> 4));
    //    printf("%d\n", 0x0f & (x >> 4));
    
    
    
     try
     {
     // Check command line arguments.
     if (argc != 2)
     {
     cerr << "Usage: server <port>" << endl;
     return 1;
     }
     unsigned short port = boost::lexical_cast<unsigned short>(argv[1]);
     
     boost::asio::io_service io_service;
     hazelcast::server server(io_service, port);
     io_service.run();
     
     while(true){
     string mUtf;
     char mChar;
     int mInt,mShort;
     float mFloat;
     bool mBool;
     long mLong;
     double mDouble;
     cout << "utf char int float short bool long double" << endl;
     cin >> mUtf >> mChar >> mInt >> mFloat >> mShort >> mBool >> mLong >> mDouble;
     MyTestObject temp(mUtf,mChar,mInt,mFloat,mShort,mBool,mLong,mDouble);
     
     cout << "Sending " << temp << endl;
     
     server.send(temp);
     
     cout << "Exit ? y/n";
     char exit;
     cin >> exit;
     if(exit == 'y')
     break;
     }
     }
     catch (exception& e)
     {
     cerr << e.what() << endl;
     }
     
}*/
