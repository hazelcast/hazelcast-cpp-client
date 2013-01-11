//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include "ContextAwareDataOutput.h"
#include "MyTestObject.h"
#include "TypeSerializer.h"
#include "SerializationService.h"
#include "SerializationServiceImpl.h"
#include "TestPortableFactory.h"
#include "Data.h"

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
    SerializationServiceImpl* serializationService = static_cast<SerializationServiceImpl*>(new SerializationServiceImpl(1,new TestPortableFactory()));
    Data data;
    int x = 3,y;
    
    data = serializationService->toData<int>(x);
    
    y =  serializationService->toObject<int>(data);
    
    if(x == y){
        cout << "OK" << endl;
    }else{
        cout << "FAIL" << endl;
    }
    
    TypeSerializer* typeSerializer = serializationService->serializerFor(typeid(x).name());
    
    cout << typeSerializer->getTypeId() << endl;
    
//    byte x = 138;
//    printf("%x\n", 0xff & x);
//    printf("%x\n", 0x0f & (x >> 4));
//    printf("%d\n", 0x0f & (x >> 4));
    
 
    /*
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
     */
    return 0;
}