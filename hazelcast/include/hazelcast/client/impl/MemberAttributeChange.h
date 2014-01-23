//
// Created by sancar koyunlu on 23/01/14.
//

#ifndef HAZELCAST_MemberAttributeChange
#define HAZELCAST_MemberAttributeChange

#include "hazelcast/client/DataSerializable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace impl {
            static const int DELTA_MEMBER_PROPERTIES_OP_PUT = 2;
            static const int DELTA_MEMBER_PROPERTIES_OP_REMOVE = 3;

            class HAZELCAST_API MemberAttributeChange : public DataSerializable {
            public:

                MemberAttributeChange();

                const std::string &getUuid() const;

                int getOperationType() const;

                const std::string &getKey() const;

                //public Object getValue();MTODO

                void writeData(serialization::ObjectDataOutput &writer) const;

                void readData(serialization::ObjectDataInput &reader);

            private:
                std::string uuid;
                int operationType;
                std::string key;
                std::string stringValue;
                long longValue;
                int intValue;
                short shortValue;
                byte byteValue;
                float floatValue;
                bool boolValue;
                double doubleValue;
            };
        }
    }
}

#endif //HAZELCAST_MemberAttributeChange
