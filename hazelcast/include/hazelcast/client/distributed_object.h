/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include "hazelcast/util/export.h"

#include <string>
#include <boost/thread/future.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#pragma warning(disable : 4250) // for warning class1' : inherits
                                // 'class2::member' via dominance
#endif

namespace hazelcast {
namespace client {
namespace impl {
class hazelcast_client_instance_impl;
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
 * @see TransactionalMap
 * @see TransactionalQueue
 * @see TransactionalMultiMap
 * @see TransactionalSet
 * @see TransactionalList
 */
class HAZELCAST_API distributed_object
{
    friend class impl::hazelcast_client_instance_impl;

public:
    /**
     * Returns the service name for this object.
     */
    virtual const std::string& get_service_name() const = 0;

    /**
     * Returns the unique name for this DistributedObject.
     *
     * @return the unique name for this object.
     */
    virtual const std::string& get_name() const = 0;

    /**
     * Destroys this object cluster-wide.
     * Clears and releases all resources for this object.
     */
    virtual boost::future<void> destroy() = 0;

    /**
     * Destructor
     */
    virtual ~distributed_object() = default;
};

} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
