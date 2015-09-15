//
// Created by sancar koyunlu on 23/07/14.
//


#ifndef HAZELCAST_PortableVersionHelper
#define HAZELCAST_PortableVersionHelper

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Portable;

            namespace pimpl {
                class HAZELCAST_API PortableVersionHelper {
                public:
                    static int getVersion(const Portable *portable, int defaultVersion);
                };
            }
        }
    }
}

#endif //HAZELCAST_PortableVersionHelper
