/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/member.h"

namespace hazelcast {
namespace client {

class cluster;

namespace connection {
class ClientConnectionManagerImpl;
}

/**
 *
 * load_balancer allows you to send operations to one of a number of
 * endpoints(Members). It is up to the implementation to use different load
 * balancing policies. If Client is configured as smart, only the operations
 * that are not key based will be router to the endpoint returned by the Load
 * Balancer. If it is not smart, load_balancer will not be used.
 * **Note:** Client is smart by default.
 *
 */
class HAZELCAST_API load_balancer final
{
public:
    /**
     * Sets the function to be called when load balancer is initialized.
     *
     * @param cluster Cluster contains current membership information for
     * initialization. And one can add membership through this class for future
     * notifications.
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    load_balancer& init(Handler&& h) &
    {
        init_ = std::forward<Handler>(h);
        return *this;
    }

    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    load_balancer&& init(Handler&& h) &&
    {
        init_ = std::forward<Handler>(h);
        return std::move(*this);
    }

    /**
     * The function returns the next member to route to.
     *
     * @param h The function to be used for finding the next member.
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    load_balancer& next(Handler&& h) &
    {
        next_ = std::forward<Handler>(h);
        return *this;
    }

    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    load_balancer&& next(Handler&& h) &&
    {
        next_ = std::forward<Handler>(h);
        return std::move(*this);
    }

private:
    /**
     * Cluster contains current membership information for initialization. And
     * one can add membership through this class for future notifications.
     */
    std::function<void(cluster&)> init_ = util::noop<cluster&>;
    std::function<boost::optional<member>(cluster&)> next_ = [](cluster&) {
        return boost::none;
    };

    friend class connection::ClientConnectionManagerImpl;
};
} // namespace client
} // namespace hazelcast
