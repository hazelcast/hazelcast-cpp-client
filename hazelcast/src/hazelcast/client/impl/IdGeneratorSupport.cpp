//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/IdGeneratorSupport.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            IdGeneratorSupport::IdGeneratorSupport() {

            };

            IdGeneratorSupport::~IdGeneratorSupport() {
                lockMap.clear();
            };

            util::Lock *IdGeneratorSupport::getLock(const std::string& instanceName) {
                util::Lock *oldLock = lockMap.get(instanceName);
                if (oldLock != NULL) {
                    return oldLock;
                }
                util::Lock *mutex = new util::Lock;
                util::Lock *pLock = lockMap.putIfAbsent(instanceName, mutex);
                if (pLock != NULL) {
                    delete mutex;
                    return pLock;
                } else {
                    return mutex;
                }
            }


        }
    }
}