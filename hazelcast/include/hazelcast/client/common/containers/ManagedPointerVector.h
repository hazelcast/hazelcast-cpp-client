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
// Created by İhsan Demir on 25/04/15.
//

#ifndef HAZELCASTCLIENT_MANAGEDPOINTERVECTOR_H
#define HAZELCASTCLIENT_MANAGEDPOINTERVECTOR_H

#include <ostream>

#include <vector>
#include "hazelcast/client/Member.h"
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {
        namespace common {
            namespace containers {
                template<class TYPE>
                class ManagedPointerVector {
                public:
                    typedef std::vector<TYPE * > VECTOR_TYPE;
                    typedef std::vector<const TYPE * > VECTOR_CONST_TYPE;

                    ManagedPointerVector(size_t len) {
                        internalVector.reserve(len);
                    }

                    virtual ~ManagedPointerVector() {
                        for (typename VECTOR_TYPE::iterator it = internalVector.begin();
                             it != internalVector.end(); ++it) {
                            delete *it;
                        }
                    }

                    void push_back(TYPE *value) {
                        internalVector.push_back(value);
                    }

                    TYPE * &operator [](size_t index) {
                        return internalVector[index];
                    }

                    bool empty() const {
                        return internalVector.empty();
                    }

                    typename VECTOR_TYPE::const_iterator begin() const {
                        return internalVector.begin();
                    }

                    typename VECTOR_TYPE::iterator begin() {
                        return internalVector.begin();
                    }

                    typename VECTOR_TYPE::const_iterator end() const {
                        return internalVector.end();
                    }

                    typename VECTOR_TYPE::iterator erase(typename VECTOR_TYPE::iterator it) {
                        // free memory
                        delete *it;
                        return internalVector.erase(it);
                    }

                    typename VECTOR_TYPE::size_type size() const {
                        return internalVector.size();
                    }

                    VECTOR_CONST_TYPE toVector() const {
                        return VECTOR_CONST_TYPE(internalVector.begin(), internalVector.end());
                    }

                private:
                    // Prevent copy
                    ManagedPointerVector(const ManagedPointerVector &rhs);
                    ManagedPointerVector &operator = (const ManagedPointerVector &rhs);

                    VECTOR_TYPE internalVector;
                };

                template <class TYPE>
                std::ostream &operator <<(std::ostream &stream, const ManagedPointerVector<TYPE> &managedVector) {
                    stream << managedVector.size() << "elements:" << std::endl;
                    size_t index = 0;
                    for (typename ManagedPointerVector<TYPE>::VECTOR_TYPE::const_iterator it = managedVector.begin(); it != managedVector.end(); ++it) {
                        TYPE *const &elementPtr = *it;
                        if (NULL == elementPtr) {
                            stream << "item " << ++index << ":" << "NUL" << std::endl;
                        } else {
                            stream << "item " << ++index << ":" << *elementPtr << std::endl;
                        }

                    }

                    return stream;
                }
            }
        }
    }
}

#endif //HAZELCASTCLIENT_MANAGEDPOINTERVECTOR_H
