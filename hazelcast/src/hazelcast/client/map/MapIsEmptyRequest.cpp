//
// Created by sancar koyunlu on 01/08/14.
//

#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/map/MapIsEmptyRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {

            MapIsEmptyRequest::MapIsEmptyRequest(const std::string& name)
            : name(name) {

            }

            int MapIsEmptyRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int MapIsEmptyRequest::getClassId() const {
                return PortableHook::IS_EMPTY;
            }

            void MapIsEmptyRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", &name);
            }
        }
    }
}
