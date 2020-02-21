/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_UTIL_CONCURRENTSET_H_
#define HAZELCAST_UTIL_CONCURRENTSET_H_

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/util/Mutex.h"
#include <set>
#include <vector>


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace util {
        template <typename T>
        class ConcurrentSet {
        public:
            ConcurrentSet() {
            }

            /**
             * Adds the specified element to this set if it is not already present
             * (optional operation).  More formally, adds the specified element
             * <tt>e</tt> to this set if the set contains no element <tt>e2</tt>
             * such that
             * <tt>(e==e2)</tt>.
             * If this set already contains the element, the call leaves the set
             * unchanged and returns <tt>false</tt>.  In combination with the
             * restriction on constructors, this ensures that sets never contain
             * duplicate elements.
             *
             *
             * @param e element to be added to this set
             * @return <tt>true</tt> if this set did not already contain the specified
             *         element
             */
            bool add(const T &e) {
                util::LockGuard lg(m);
                return internalSet.insert(e).second;
            }

            /**
             * Returns the number of elements in this set (its cardinality).
             *
             * @return the number of elements in this set (its cardinality)
             */
            size_t size() {
                util::LockGuard lg(m);
                return internalSet.size();
            }

            /**
             * Returns <tt>true</tt> if this set contains no elements.
             *
             * @return <tt>true</tt> if this set contains no elements
             */
            bool isEmpty() {
                util::LockGuard lg(m);
                return internalSet.empty();
            }

            /**
             * Returns an array containing all of the elements in this set.
             * If this set makes any guarantees as to what order its elements
             * are returned by its iterator, this method must return the
             * elements in the same order.
             *
             * <p>The returned array will be "safe" in that no references to it
             * are maintained by this set.  (In other words, this method must
             * allocate a new array even if this set is backed by an array).
             * The caller is thus free to modify the returned array.
             *
             * @return an array containing all the elements in this set
             */
            std::vector<T> toArray() {
                util::LockGuard lg(m);
                std::vector<T> result;
                for (const typename std::set<T>::value_type &value  : internalSet) {
                                result.push_back(value);
                            }
                return result;
            }

            void clear() {
                util::LockGuard lg(m);
                internalSet.clear();
            }
        private:
            util::Mutex m;
            std::set<T> internalSet;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_UTIL_CONCURRENTSET_H_

