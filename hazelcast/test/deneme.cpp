//
// Created by sancar koyunlu on 7/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "deneme.h"
#include "AtomicPointer.h"
#include "Member.h"
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <fstream>

namespace  atomicVisibility {
    hazelcast::util::AtomicPointer<int> testptr;

    void update() {
        long i = 1000L * 1000;
        long last;
        while (i--) {
            int *pInt = new int;
            *pInt = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
            testptr.reset(pInt);
            last = *pInt;
        }
        std::cout << "update finished " << last << std::endl;
    };

    void observer() {
        try{
            std::ofstream file;
            boost::thread::id s = boost::this_thread::get_id();
            std::stringstream id;
            id << "t" << s << ".txt";
            file.open(id.str().c_str(), std::ofstream::out);
            long i = 1000L * 1000L;
            while (i--) {
                int *pInt = testptr.get();
                file << *pInt << std::endl;
            }
            std::cout.flush();
            file.flush();
            file.close();
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        } catch(...){
            std::cout << "unknown exception" << std::endl;
        }
        std::cout << "finished" << std::endl;
    }


    void test() {
        int *p = new int;
        *p = 1;
        testptr.reset(p);

        boost::thread o(observer);
        boost::thread o1(observer);
        boost::thread o2(observer);
        boost::thread o3(observer);
        boost::thread o4(observer);
        boost::thread u1(update);
        boost::thread u2(update);
        std::cout << "Joining" << std::endl;
        o.join();
        o1.join();
        o2.join();
        o3.join();
        o4.join();
        u1.join();
        u2.join();

    }
}


int deneme::init() {
    atomicVisibility::test();
    return 0;
}
