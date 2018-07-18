/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_UTIL_COLLECTION_H_
#define HAZELCAST_UTIL_COLLECTION_H_

#include "hazelcast/util/Iterator.h"

namespace hazelcast {
    namespace util {
        template <typename E>
        class Collection {
        public:
            virtual ~Collection() {
            }

            /**
             * Returns an iterator over the elements in this collection.  There are no
             * guarantees concerning the order in which the elements are returned
             * (unless this collection is an instance of some class that provides a
             * guarantee).
             *
             * @return an <tt>Iterator</tt> over the elements in this collection
             */
            virtual boost::shared_ptr<Iterator<E> > iterator() = 0;

            /**
             * Returns the number of elements in this collection.
             *
             * @return the number of elements in this collection
             */
            virtual int size() const = 0;
        };
    }
}

#endif //HAZELCAST_UTIL_COLLECTION_H_
