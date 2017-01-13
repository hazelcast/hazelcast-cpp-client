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
// Created by sancar koyunlu on 12/11/13.

#ifndef HAZELCAST_DistributedObject
#define HAZELCAST_DistributedObject

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/impl/DistributedObjectInfo.h"
#include "hazelcast/client/IDistributedObject.h"

#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class BaseEventHandler;

            class BaseRemoveListenerRequest;

            class ClientRequest;
        }

        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        /**
        * Base class for all distributed objects.
        *
        * @see IMap
        * @see MultiMap
        * @see IQueue
        * @see IList
        * @see ISet
        * @see ITopic
        * @see ILock
        * @see ISemaphore
        * @see ICountDownLatch
        * @see IdGenerator
        * @see IAtomicLong
        * @see TransactionalMap
        * @see TransactionalQueue
        * @see TransactionalMultiMap
        * @see TransactionalSet
        * @see TransactionalList
        */
        class HAZELCAST_API DistributedObject : public IDistributedObject {
            friend class HazelcastClient;

        public:
            /**
            * Returns the service name for this object.
            */
            const std::string& getServiceName() const;

            /**
            * Returns the unique name for this DistributedObject.
            *
            * @return the unique name for this object.
            */
            const std::string& getName() const;

            /**
            * Destroys this object cluster-wide.
            * Clears and releases all resources for this object.
            */
            virtual void destroy() = 0;

            /**
            * Destructor
            */
            virtual ~DistributedObject();

        protected:
            /**
            * Constructor.
            */
            DistributedObject(const std::string& serviceName, const std::string& objectName);

            /**
            * method to be called when cluster-wide destroy method is called.
            */
            virtual void onDestroy();

            impl::DistributedObjectInfo info;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_DistributedObject

