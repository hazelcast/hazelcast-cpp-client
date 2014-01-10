//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/SmartConnectionManager.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            SmartConnectionManager::SmartConnectionManager(spi::ClientContext &clientContext)
            :ConnectionManager(clientContext) {

            };
        }
    }
}
