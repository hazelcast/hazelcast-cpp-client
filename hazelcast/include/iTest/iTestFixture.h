//
// Created by sancar koyunlu on 8/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_iTestFixture
#define HAZELCAST_iTestFixture

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#define RUN_TEST(TEST, ...) do{ \
  TEST t(  __VA_ARGS__ );       \
  if(!t.executeTests()){        \
    return 1;               \
  }                         \
}while(0)

namespace iTest {
    static int assertNumber;

    struct HAZELCAST_API iTestException {
        std::string message;
    };

    template<typename T>
    class HAZELCAST_API iTestFixture {
        typedef void (T::*TestFunction)();

    public:
        iTestFixture(const std::string& fixtureName)
        : fixtureName(fixtureName), id(0) {

        }

        virtual ~iTestFixture() {
        }

        virtual void addTests() = 0;

        virtual void beforeClass() = 0;

        virtual void afterClass() = 0;

        virtual void beforeTest() = 0;

        virtual void afterTest() = 0;

        void addTest(TestFunction test, const std::string& name) {
            tests.push_back(test);
            testNames[id++] = name;
        }

        bool executeTests() {
            addTests();
            (std::cout << "<<<<<<< " << fixtureName << " >>>>>>" << std::endl);
            beforeClass();
            T *t = static_cast<T *>(this);
            for (unsigned int i = 0; i < tests.size(); i++) {
                TestFunction test = tests[i];
                (std::cout << "======= " << i << ". " << testNames[i] << " ======= " << std::endl);
                beforeTest();
                bool isOk = true;
                try {
                    assertNumber = 0;
                    ((*t).*(test))();
                } catch (iTestException& e) {
                    (std::cout << e.message << std::endl);
                    isOk = false;
                } catch (...) {
                    isOk = false;
                    std::cout << "========== EXCEPTION ======== " << std::endl;
                    throw;
                }
                afterTest();
                if (isOk) {
                    std::cout << "============OK============== " << std::endl;
                } else {
                    std::cout << "============FAILED============== " << std::endl;
                    return false;
                }
            }
            afterClass();
            return true;
        }


    private:
        std::vector<TestFunction> tests;
        std::map<int, std::string> testNames;
        std::string fixtureName;
        int id;
    };
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_iTestFixture

