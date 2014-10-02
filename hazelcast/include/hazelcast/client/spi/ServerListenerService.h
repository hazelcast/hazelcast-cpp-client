//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SERVER_LISTENER_SERVICE
#define HAZELCAST_SERVER_LISTENER_SERVICE

#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class ClientRequest;

            class BaseRemoveListenerRequest;

            class BaseEventHandler;
        }

        namespace connection{
            class CallPromise;
        }

        namespace serialization {
            namespace pimpl {
                class Data;
            }

        }
        namespace spi {
            class ClientContext;

            class HAZELCAST_API ServerListenerService {
            public:
                ServerListenerService(spi::ClientContext &clientContext);

                std::string listen(const impl::ClientRequest *registrationRequest, int partitionId, impl::BaseEventHandler *handler);

                std::string listen(const impl::ClientRequest *registrationRequest, impl::BaseEventHandler *handler);

                bool stopListening(impl::BaseRemoveListenerRequest *request, const std::string &registrationId);

                void reRegisterListener(const std::string &registrationId, boost::shared_ptr<std::string> alias, int callId);

                void retryFailedListener(boost::shared_ptr<connection::CallPromise> listenerPromise);

                void triggerFailedListeners();

            private:

                util::Mutex failedListenerLock;
                std::vector< boost::shared_ptr<connection::CallPromise> > failedListeners;

                util::SynchronizedMap<std::string, int > registrationIdMap;
                util::SynchronizedMap<std::string, const std::string > registrationAliasMap;
                spi::ClientContext &clientContext;

                void registerListener(boost::shared_ptr<std::string> registrationId, int callId);

                bool deRegisterListener(std::string &registrationId);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //__ServerListenerService_H_

