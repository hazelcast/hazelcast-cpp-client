#include "hazelcast/client/serialization/pimpl/PortableVersionHelper.h"

#include "hazelcast/client/serialization/VersionedPortable.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                int PortableVersionHelper::getVersion(const Portable *portable, int defaultVersion) {
                    int version = defaultVersion;
                    if (const VersionedPortable *versionedPortable = dynamic_cast<const VersionedPortable *>(portable)) {
                        version = versionedPortable->getClassVersion();
                        if (version < 0) {
                            throw exception::IllegalArgumentException("PortableVersionHelper:getVersion", "Version cannot be negative!");
                        }
                    }
                    return version;
                }
            }
        }
    }
}
