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

#include <functional>

#include "hazelcast/util/export.h"
#include "hazelcast/util/noop.h"
#include "hazelcast/util/type_traits.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
class lifecycle_event;
namespace spi {
class lifecycle_service;
}

/**
 * Listener object for listening lifecycle events of hazelcast instance
 *
 * \warning
 * 1 - If listener should do a time consuming operation, off-load the operation
 * to another thread. otherwise it will slow down the system. \warning 2 - Do
 * not make a call to hazelcast. It can cause a deadlock.
 *
 * \see LifecycleEvent
 * \see hazelcast_client::addlifecycle_listener
 */
class HAZELCAST_API lifecycle_listener final
{
public:
    /**
     * Set an handler function to be invoked when the client is starting
     * \param h a `void` function object that is callable without any
     * parameters. \return `*this`
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener& on_starting(Handler&& h) &
    {
        starting_ = std::forward<Handler>(h);
        return *this;
    }

    /**
     * \copydoc lifecycle_listener::on_starting
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener&& on_starting(Handler&& h) &&
    {
        on_starting(std::forward<Handler>(h));
        return std::move(*this);
    }

    /**
     * Set an handler function to be invoked when the client has started
     * \param h a `void` function object that is callable without any
     * parameters. \return `*this`
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener& on_started(Handler&& h) &
    {
        started_ = std::forward<Handler>(h);
        return *this;
    }

    /**
     * \copydoc lifecycle_listener::on_started
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener&& on_started(Handler&& h) &&
    {
        on_started(std::forward<Handler>(h));
        return std::move(*this);
    }

    /**
     * Set an handler function to be invoked when the client is shutting down
     * \param h a `void` function object that is callable without any
     * parameters. \return `*this`
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener& on_shutting_down(Handler&& h) &
    {
        shutting_down_ = std::forward<Handler>(h);
        return *this;
    }

    /**
     * \copydoc lifecycle_listener::on_shutting_down
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener&& on_shutting_down(Handler&& h) &&
    {
        on_shutting_down(std::forward<Handler>(h));
        return std::move(*this);
    }

    /**
     * Set an handler function to be invoked when the client's shutdown has
     * completed \param h a `void` function object that is callable without any
     * parameters. \return `*this`
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener& on_shutdown(Handler&& h) &
    {
        shutdown_ = std::forward<Handler>(h);
        return *this;
    }

    /**
     * \copydoc lifecycle_listener::on_shutdown
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener&& on_shutdown(Handler&& h) &&
    {
        on_shutdown(std::forward<Handler>(h));
        return std::move(*this);
    }

    /**
     * Set an handler function to be invoked when the client is connected to the
     * cluster \param h a `void` function object that is callable without any
     * parameters. \return `*this`
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener& on_connected(Handler&& h) &
    {
        connected_ = std::forward<Handler>(h);
        return *this;
    }

    /**
     * \copydoc lifecycle_listener::on_connected
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener&& on_connected(Handler&& h) &&
    {
        on_connected(std::forward<Handler>(h));
        return std::move(*this);
    }

    /**
     * Set an handler function to be invoked when client is disconnected from
     * the cluster. \param h a `void` function object that is callable without
     * any parameters. \return `*this`
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener& on_disconnected(Handler&& h) &
    {
        disconnected_ = std::forward<Handler>(h);
        return *this;
    }

    /**
     * \copydoc lifecycle_listener::on_disconnected
     */
    template<typename Handler,
             typename = util::enable_if_rvalue_ref_trait<Handler&&>>
    lifecycle_listener&& on_disconnected(Handler&& h) &&
    {
        on_disconnected(std::forward<Handler>(h));
        return std::move(*this);
    }

private:
    using HandlerType = std::function<void()>;
    HandlerType starting_{ util::noop<> }, started_{ util::noop<> },
      shutting_down_{ util::noop<> }, shutdown_{ util::noop<> },
      connected_{ util::noop<> }, disconnected_{ util::noop<> };

    friend class spi::lifecycle_service;
};
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
