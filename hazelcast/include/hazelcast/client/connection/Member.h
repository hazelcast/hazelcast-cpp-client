//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MEMBER
#define HAZELCAST_MEMBER

#include "../protocol/ProtocolConstants.h"
#include "../Address.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class Member : public DataSerializable {

            public:
                Member();

                Member(const Member&);

                Member(const Address&);

                ~Member();

                Member& operator = (const Member&);

                bool operator ==(const Member&) const;

                int operator <(const Member&) const;

                const Address& getAddress() const;

                std::string getUuid() const;

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer);

                void readData(serialization::ObjectDataInput& reader);

            private:
                Address address;
                std::string uuid;
            };

            inline std::ostream& operator <<(std::ostream &strm, const Member &a) {
                return strm << "Member[" << a.getAddress().getHost() << "]:" << util::to_string(a.getAddress().getPort());
            };
        }
    }
}

#endif //HAZELCAST_MEMBER
