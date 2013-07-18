//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_IdGeneratorSupport
#define HAZELCAST_IdGeneratorSupport

#include "../../util/ConcurrentMap.h"
#include <string>
#include <boost/thread/mutex.hpp>

namespace hazelcast {
    namespace client {
        namespace impl {
            class IdGeneratorSupport {
            public:
                IdGeneratorSupport();

                ~IdGeneratorSupport();

                boost::mutex *getLock(const std::string& instanceName);

            private:
                util::ConcurrentMap<std::string, boost::mutex> lockMap;
            };
        }
    }
}


#endif //HAZELCAST_IdGeneratorSupport
