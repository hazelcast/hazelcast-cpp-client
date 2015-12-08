/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 8/1/13.


#ifndef HAZELCAST_iTestFixture
#define HAZELCAST_iTestFixture

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "hazelcast/client/exception/IOException.h"

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

#define RUN_TEST_NO_ARGS(TEST) do{ \
  TEST t;       \
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
    class iTestFixture {
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
                } catch (hazelcast::client::exception::IException &e) {
                    (std::cout << e.what() << std::endl);
                    isOk = false;
                }
                catch (...) {
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

