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
#include "hazelcast/util/AtomicBoolean.h"

#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace util {
        AtomicBoolean::AtomicBoolean(): v(false){
        }

        AtomicBoolean::AtomicBoolean(bool i) : v(i) {
        }

        bool AtomicBoolean::operator!(){
            LockGuard lockGuard(mutex);
            return !v;
        }

        bool AtomicBoolean::operator ==(bool i){
            LockGuard lockGuard(mutex);
            return v == i;
        }

        bool AtomicBoolean::operator !=(bool i){
            LockGuard lockGuard(mutex);
            return v != i;
        }

        void AtomicBoolean::operator =(bool i){
            LockGuard lockGuard(mutex);
            v = i;
        }

        AtomicBoolean::operator bool(){
            LockGuard lockGuard(mutex);
            return v;
        }

        bool AtomicBoolean::compareAndSet(bool compareValue, bool setValue){
            LockGuard lockGuard(mutex);
            if(compareValue == v){
                v = setValue;
                return true;
            }
            return false;
        }
    }
}
