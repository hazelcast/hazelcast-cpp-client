//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/ServerListenerService.h"


namespace hazelcast {
    namespace client {


        namespace spi {


            ServerListenerService::ServerListenerService(InvocationService& invocationService)
            :invocationService(invocationService) {

            };

            ServerListenerService::~ServerListenerService() {
                vector<boost::shared_ptr<util::ConcurrentSmartMap<long, ListenerSupportBase> > > values = allListeners.values();
                values.clear();
            };

            bool ServerListenerService::stopListening(const std::string& instanceName, long registrationId) {
                boost::shared_ptr<util::ConcurrentSmartMap<long, ListenerSupportBase> > pMap = allListeners.get(instanceName);
                boost::shared_ptr<ListenerSupportBase> listenerSupportBase = pMap->remove(registrationId);
                if (listenerSupportBase != NULL) {
                    listenerSupportBase->stop();
                    return true;
                }
                return false;
            };
        }
    }
}
