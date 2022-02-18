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

#include <string>
#include <memory>
#include <boost/thread/future.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/client/distributed_object.h"
#include "hazelcast/client/spi/impl/ListenerMessageCodec.h"
#include "hazelcast/client/spi/EventHandler.h"
#include "hazelcast/client/serialization/serialization.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace protocol {
class ClientMessage;
}
namespace impl {
class BaseEventHandler;
}

namespace spi {
class ClientContext;

/**
 * Base Interface for client proxies.
 *
 */
class HAZELCAST_API ClientProxy : public distributed_object
{
public:
    ClientProxy(const std::string& name,
                const std::string& service_name,
                ClientContext& context);

    ~ClientProxy() override;

    /**
     * Internal API.
     * Called when proxy is created.
     * Overriding implementations can add initialization specific logic into
     * this method like registering a listener, creating a cleanup task etc.
     */
    virtual void on_initialize();

    /**
     * Internal API.
     * Called before client shutdown.
     * Overriding implementations can add shutdown specific logic here.
     */
    virtual void on_shutdown();

    const std::string& get_name() const override;

    const std::string& get_service_name() const override;

    /**
     * Internal API.
     * @return context
     */
    ClientContext& get_context();

    /**
     * Destroys this object cluster-wide.
     * Clears and releases all resources for this object.
     */
    boost::future<void> destroy() override;

    /**
     * Internal API.
     * Destroys this client proxy instance locally without issuing distributed
     * object destroy request to the cluster as the {@link #destroy} method
     * does.
     * <p>
     * The local destruction operation still may perform some communication
     * with the cluster; for example, to unregister remote event subscriptions.
     */
    void destroy_locally();

    /**
     *  Internal API.
     * Destroys the remote distributed object counterpart of this proxy by
     * issuing the destruction request to the cluster.
     */
    boost::future<void> destroy_remotely();

    /**
     * Internal API.
     *
     * @param listenerMessageCodec The codec used for listener
     * register/deregister
     * @param handler Event handler for the listener
     */
    boost::future<boost::uuids::uuid> register_listener(
      std::shared_ptr<impl::ListenerMessageCodec> listener_message_codec,
      std::shared_ptr<client::impl::BaseEventHandler> handler);

    /**
     * Internal API.
     *
     * @param registrationId The registration id for the listener to be
     * unregistered.
     */
    boost::future<bool> deregister_listener(boost::uuids::uuid registration_id);

protected:
    /**
     * Called before proxy is destroyed and determines whether destroy should be
     * done.
     *
     * @return <code>true</code> if destroy should be done, otherwise
     * <code>false</code>
     */
    bool pre_destroy();

    /**
     * Called before proxy is destroyed.
     * Overriding implementations should clean/release resources created during
     * initialization.
     */
    virtual void on_destroy();

    /**
     * Called after proxy is destroyed.
     */
    virtual void post_destroy();

    serialization::pimpl::SerializationService& get_serialization_service();

    const std::string name_;

    const std::string service_name_;
    spi::ClientContext& context_;
};
} // namespace spi
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
