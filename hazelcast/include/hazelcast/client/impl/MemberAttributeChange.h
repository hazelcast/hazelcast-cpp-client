//
// Created by sancar koyunlu on 23/01/14.
//

#ifndef HAZELCAST_MemberAttributeChange
#define HAZELCAST_MemberAttributeChange

#include "hazelcast/client/serialization/pimpl/DataSerializable.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/util/IOUtil.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace impl {


            class HAZELCAST_API MemberAttributeChange : public serialization::pimpl::DataSerializable {
            public:

                MemberAttributeChange();

                const std::string &getUuid() const;

                MemberAttributeEvent::MapOperationType getOperationType() const;

                const std::string &getKey() const;

                const std::string &getValue() const;

                util::IOUtil::PRIMITIVE_ID getTypeId() const;

                void writeData(serialization::ObjectDataOutput &writer) const;

                void readData(serialization::ObjectDataInput &reader);

            private:
                std::string uuid;
                MemberAttributeEvent::MapOperationType operationType;
                std::string key;
                std::string value;
                util::IOUtil::PRIMITIVE_ID primitive_id;
            };
        }
    }
}

#endif //HAZELCAST_MemberAttributeChange
