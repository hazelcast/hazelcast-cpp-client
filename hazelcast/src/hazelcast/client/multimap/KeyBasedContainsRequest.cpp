//
// Created by sancar koyunlu on 18/08/14.
//

#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/multimap/KeyBasedContainsRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {

            KeyBasedContainsRequest::KeyBasedContainsRequest(const std::string& name, const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long threadId)
            : KeyBasedRequest(name, key)
            , hasValue(true)
            , value(value)
            , threadId(threadId) {

            }

            KeyBasedContainsRequest::KeyBasedContainsRequest(const std::string& name, const serialization::pimpl::Data& key, long threadId)
            : KeyBasedRequest(name, key)
            , hasValue(false)
            , threadId(threadId) {

            }

            int KeyBasedContainsRequest::getClassId() const {
                return MultiMapPortableHook::KEY_BASED_CONTAINS;
            }

            void KeyBasedContainsRequest::write(serialization::PortableWriter& writer) const {
                writer.writeLong("threadId", threadId);
                KeyBasedRequest::write(writer);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                if (hasValue) {
                    out.writeData(&value);
                } else {
                    out.writeData(NULL);
                }
            }

            bool KeyBasedContainsRequest::isRetryable() const {
                return true;
            }
        }
    }
}

