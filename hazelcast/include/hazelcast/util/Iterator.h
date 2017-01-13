/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 9 Dec 2016.

#ifndef HAZELCAST_UTIL_ITERATOR_H_
#define HAZELCAST_UTIL_ITERATOR_H_

#include <assert.h>
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace util {
        template <typename E>
        class Iterator {
        public:
            virtual ~Iterator() {
            }

            /**
             * Returns <tt>true</tt> if the iteration has more elements. (In other
             * words, returns <tt>true</tt> if <tt>next</tt> would return an element
             * rather than throwing an exception.)
             *
             * @return <tt>true</tt> if the iterator has more elements.
             */
            virtual bool hasNext() = 0;

            /**
             * Returns the next element in the iteration.
             *
             * @return the next element in the iteration.
             * @exception NoSuchElementException iteration has no more elements.
             */
            virtual boost::shared_ptr<E> next() {
                assert(0);
                return boost::shared_ptr<E>();
            }

            /**
             *
             * Removes from the underlying collection the last element returned by the
             * iterator (optional operation).  This method can be called only once per
             * call to <tt>next</tt>.  The behavior of an iterator is unspecified if
             * the underlying collection is modified while the iteration is in
             * progress in any way other than by calling this method.
             *
             * @exception UnsupportedOperationException if the <tt>remove</tt>
             *		  operation is not supported by this Iterator.

             * @exception IllegalStateException if the <tt>next</tt> method has not
             *		  yet been called, or the <tt>remove</tt> method has already
             *		  been called after the last call to the <tt>next</tt>
             *		  method.
             */
            virtual void remove() {
                assert(0);
            }
        };
    }
}

#endif //HAZELCAST_UTIL_ITERATOR_H_

