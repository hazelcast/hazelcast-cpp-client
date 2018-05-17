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
#ifndef HAZELCAST_ID_GENERATOR
#define HAZELCAST_ID_GENERATOR

#include "hazelcast/client/IAtomicLong.h"
#include "hazelcast/util/AtomicInt.h"
#include <boost/shared_ptr.hpp>
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
        }

        namespace impl {
            class IdGeneratorSupport;
        }

        /**
         * Cluster-wide unique id generator.
         */
        class HAZELCAST_API IdGenerator : public proxy::ProxyImpl {
            friend class impl::HazelcastClientInstanceImpl;

        public:
            enum {
                BLOCK_SIZE = 1000
            };

            /**
             * Try to initialize this IdGenerator instance with given id
             *
             * @return true if initialization success
             */
            bool init(long id);

            /**
             * Generates and returns cluster-wide unique id.
             * Generated ids are guaranteed to be unique for the entire cluster
             * as long as the cluster is live. If the cluster restarts then
             * id generation will start from 0.
             *
             * @return cluster-wide new unique id
             */
            long newId();

        private:

            IAtomicLong atomicLong;
            boost::shared_ptr<util::Atomic<int64_t> > local;
            boost::shared_ptr<util::Atomic<int32_t> > residue;
            boost::shared_ptr<util::Mutex> localLock;
            IdGenerator(const std::string &instanceName, spi::ClientContext *context);

            void onDestroy();

        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_ID_GENERATOR */
