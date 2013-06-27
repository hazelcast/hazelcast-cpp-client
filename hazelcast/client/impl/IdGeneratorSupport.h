//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_IdGeneratorSupport
#define HAZELCAST_IdGeneratorSupport

#include "../../util/ConcurrentMap.h"
#include "../../util/Lock.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace impl {
            class IdGeneratorSupport {
            public:
                IdGeneratorSupport();

                ~IdGeneratorSupport();

                util::Lock *getLock(const std::string& instanceName);

            private:
                util::ConcurrentMap<std::string, util::Lock> lockMap;
            };
        }
    }
}


#endif //HAZELCAST_IdGeneratorSupport
