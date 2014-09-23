//
// Created by sancar koyunlu on 18/08/14.
//

#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/multimap/KeyBasedContainsRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {

            KeyBasedContainsRequest::KeyBasedContainsRequest(const std::string& name, const serialization::pimpl::Data& key, const serialization::pimpl::Data& value)
            : KeyBasedRequest(name, key)
            , hasValue(true)
            , value(value) {

            }

            KeyBasedContainsRequest::KeyBasedContainsRequest(const std::string& name, const serialization::pimpl::Data& key)
            : KeyBasedRequest(name, key)
            , hasValue(false) {

            }

            int KeyBasedContainsRequest::getClassId() const {
                return MultiMapPortableHook::KEY_BASED_CONTAINS;
            }

            void KeyBasedContainsRequest::write(serialization::PortableWriter& writer) const {
                KeyBasedRequest::write(writer);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                out.writeBoolean(hasValue);
                if (hasValue) {
                    value.writeData(out);
                }
            }

            bool KeyBasedContainsRequest::isRetryable() const {
                return true;
            }
        }
    }
}

