//
// Created by sancar koyunlu on 23/07/14.
//


#ifndef HAZELCAST_PortableVersionHelper
#define HAZELCAST_PortableVersionHelper

#include "hazelcast/client/serialization/VersionedPortable.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class PortableVersionHelper {
                public:
                    static int getVersion(const Portable *portable, int defaultVersion) {
                        int version = defaultVersion;
                        if (const VersionedPortable *versionedPortable = dynamic_cast<const VersionedPortable *>(portable)) {
                            version = versionedPortable->getClassVersion();
                            if (version < 0) {
                                throw exception::IllegalArgumentException("PortableVersionHelper:getVersion", "Version cannot be negative!");
                            }
                        }
                        return version;
                    }
                };
            }
        }
    }
}

#endif //HAZELCAST_PortableVersionHelper
