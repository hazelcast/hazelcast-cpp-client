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
// Created by sancar koyunlu on 31/03/14.
//


#ifndef HAZELCAST_AtomicInt
#define HAZELCAST_AtomicInt

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace util {
        class HAZELCAST_API AtomicInt {
        public:
            AtomicInt();

            AtomicInt(int v);

            int operator--(int );

            int operator++(int );

            int operator++();

            void operator =(int i);

            operator int();

            int operator--();

            bool operator <=(int i);

            bool operator ==(int i);

            bool operator !=(int i);

        private:
            Mutex mutex;
            int v;

            AtomicInt(const AtomicInt &rhs);

            void operator =(const AtomicInt &rhs);
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_AtomicInt

