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
            class Member {
                template<typename HzWriter>
                friend void hazelcast::client::serialization::writePortable(HzWriter& writer, const hazelcast::client::connection::Member& ar);

                template<typename HzReader>
                friend void hazelcast::client::serialization::readPortable(HzReader& reader, hazelcast::client::connection::Member& ar);

            public:
                Member();

                Member(const Member&);

                ~Member();

                Member& operator = (const Member&);

                bool operator ==(const Member&) const;

                int operator <(const Member&) const;

                const Address& getAddress() const;

                std::string getUuid() const;

            private:
                hazelcast::client::Address address;
                std::string uuid;
            };
        }
    }
}

inline std::ostream& operator <<(std::ostream &strm, const hazelcast::client::connection::Member &a) {
    return strm << std::string("Member[") << a.getAddress().getHost() << std::string("]:") << hazelcast::util::to_string(a.getAddress().getPort());
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getSerializerId(const hazelcast::client::connection::Member& x) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            inline int getFactoryId(const hazelcast::client::connection::Member& ar) {
                return hazelcast::client::protocol::ProtocolConstants::DATA_FACTORY_ID;
            }

            inline int getClassId(const hazelcast::client::connection::Member& ar) {
                return hazelcast::client::protocol::ProtocolConstants::MEMBER_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::connection::Member& ar) {
                writer << ar.address;
                writer << ar.uuid;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::connection::Member& ar) {
                reader >> ar.address;
                reader >> ar.uuid;
            };

        }
    }
}
#endif //HAZELCAST_MEMBER
