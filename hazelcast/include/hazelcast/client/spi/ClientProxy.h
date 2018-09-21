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

#ifndef HAZELCAST_CLIENT_SPI_CLIENTPROXY_H_
#define HAZELCAST_CLIENT_SPI_CLIENTPROXY_H_

#include <string>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/DistributedObject.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class IDistributedObject;

        namespace spi {
            class ClientContext;

            /**
             * Base Interface for client proxies.
             *
             */
            class HAZELCAST_API ClientProxy : public virtual DistributedObject {
            public:
                ClientProxy(const std::string &name, const std::string &serviceName, ClientContext &context);

                virtual ~ClientProxy();

                /**
                 * Called when proxy is created.
                 * Overriding implementations can add initialization specific logic into this method
                 * like registering a listener, creating a cleanup task etc.
                 */
                virtual void onInitialize() = 0;

                const std::string &getName() const;

                const std::string &getServiceName() const;

                ClientContext &getContext();

                virtual void destroy();

                /**
                 * Destroys this client proxy instance locally without issuing distributed
                 * object destroy request to the cluster as the {@link #destroy} method
                 * does.
                 * <p>
                 * The local destruction operation still may perform some communication
                 * with the cluster; for example, to unregister remote event subscriptions.
                 */
                void destroyLocally();


                /**
                 * Destroys the remote distributed object counterpart of this proxy by
                 * issuing the destruction request to the cluster.
                 */
                void destroyRemotely();

            protected:
                /**
                 * Called before proxy is destroyed and determines whether destroy should be done.
                 *
                 * @return <code>true</code> if destroy should be done, otherwise <code>false</code>
                 */
                bool preDestroy();

                /**
                 * Called before proxy is destroyed.
                 * Overriding implementations should clean/release resources created during initialization.
                 */
                void onDestroy();

                /**
                 * Called after proxy is destroyed.
                 */
                void postDestroy();

                const std::string name;

            private:
                const std::string serviceName;
                spi::ClientContext &context;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_CLIENTPROXY_H_

