//
// Created by sancar koyunlu on 30/01/14.
//

#include "hazelcast/client/map/PutIfAbsentRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {

            PutIfAbsentRequest::PutIfAbsentRequest(const std::string &name, serialization::Data &key, serialization::Data &value, long threadId, long ttl)
            : key(key)
            , name(name)
            , value(value)
            , threadId(threadId)
            , ttl(ttl) {

            };

            int PutIfAbsentRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int PutIfAbsentRequest::getClassId() const {
                return PortableHook::PUT_IF_ABSENT;
            }

            void PutIfAbsentRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", threadId);
                writer.writeLong("ttl", ttl);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                key.writeData(out);
                value.writeData(out);
            };

        }
    }
}

