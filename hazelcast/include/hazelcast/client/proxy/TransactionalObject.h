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
// Created by sancar koyunlu on 12/11/13.




#ifndef HAZELCAST_TransactionalObject
#define HAZELCAST_TransactionalObject


#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include <string>
#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace txn {
            class BaseTxnRequest;
        }
        namespace proxy {

            class HAZELCAST_API TransactionalObject {
            public:
                TransactionalObject(const std::string& serviceName, const std::string& objectName, txn::TransactionProxy *context);

                virtual ~TransactionalObject();

                const std::string& getServiceName();

                const std::string& getName();

                void destroy();

            protected:
                virtual void onDestroy();

                template<typename T>
                serialization::pimpl::Data toData(const T& object) {
                    return context->getSerializationService().template toData<T>(&object);
                }

                template<typename T>
                boost::shared_ptr<T> toObject(const serialization::pimpl::Data& data) {
                    return context->getSerializationService().template toObject<T>(data);
                }

                template<typename K>
                std::vector<K> toObjectCollection(const std::vector<serialization::pimpl::Data>& keyDataSet) {
                    int size = keyDataSet.size();
                    std::vector<K> keys(size);
                    for (int i = 0; i < size; i++) {
                        boost::shared_ptr<K> v = toObject<K>(keyDataSet[i]);
                        keys[i] = *v;
                    }
                    return keys;
                }

                serialization::pimpl::Data invoke(txn::BaseTxnRequest *request);

            private:
                const std::string serviceName;
                const std::string name;
                txn::TransactionProxy *context;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_TransactionalObject

