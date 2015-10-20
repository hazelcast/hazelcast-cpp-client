//
// Created by sancar koyunlu on 04/09/14.
//

#include "hazelcast/client/map/EvictAllRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            EvictAllRequest::EvictAllRequest(const std::string& name)
            : name(name) {

            }

            int EvictAllRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int EvictAllRequest::getClassId() const {
                return PortableHook::EVICT_ALL;
            }

            void EvictAllRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", &name);
            }

        }
    }
}

