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
#ifndef HAZELCAST_CLIENT_IDGENERATOR_H_
#define HAZELCAST_CLIENT_IDGENERATOR_H_

#include <stdint.h>
#include <memory>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/impl/IdGeneratorInterface.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class HazelcastClientInstanceImpl;
        }

        /**
         * @deprecated The implementation can produce duplicate IDs in case of network split, even with split-brain
         * protection enabled (during short window while split-brain is detected). Use {@link
         * HazelcastClient#getFlakeIdGenerator(const std::string &)} for an alternative implementation which does not
         * suffer from this problem.
         *
         * Cluster-wide unique id generator.
         */
        class HAZELCAST_API IdGenerator : public impl::IdGeneratorInterface {
            friend class impl::HazelcastClientInstanceImpl;

            friend class FlakeIdGenerator;

        public:
            virtual ~IdGenerator();

            virtual bool init(int64_t id);

            virtual int64_t newId();

        private:
            IdGenerator(const std::shared_ptr<impl::IdGeneratorInterface> &impl);

            std::shared_ptr<impl::IdGeneratorInterface> impl;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_IDGENERATOR_H_ */
