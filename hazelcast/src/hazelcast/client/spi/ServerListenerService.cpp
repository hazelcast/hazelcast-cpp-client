////
//// Created by sancar koyunlu on 6/24/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//#include "hazelcast/client/spi/ServerListenerService.h"
//
//
//namespace hazelcast {
//    namespace client {
//        namespace spi {
//
//            ServerListenerService::ServerListenerService(InvocationService& invocationService)
//            :invocationService(invocationService) {
//
//            };
//
//            ServerListenerService::~ServerListenerService() {
//                boost::lock_guard<boost::mutex> lockGuard(lock);
//                std::map<long, ListenerSupportBase *>::iterator it;
//                for (it = allListeners.begin(); it != allListeners.end(); it++) {
//                    delete it->second;
//                }
//            };
//
//            bool ServerListenerService::stopListening(long registrationId) {
//                boost::lock_guard<boost::mutex> lockGuard(lock);
//                if (allListeners.count(registrationId) > 0) {
//                    ListenerSupportBase *listenerSupportBase = allListeners[registrationId];
//                    listenerSupportBase->stop();
//                    allListeners.erase(registrationId);
//                    delete listenerSupportBase;
//					return true;
//                }
//                return false;
//            };
//        }
//    }
//}
