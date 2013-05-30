//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_MEMBERSHIP_EVENT
#define HAZELCAST_MEMBERSHIP_EVENT

#include "Member.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            
            class MembershipEvent {
                template<typename HzWriter>
                friend void hazelcast::client::serialization::writePortable(HzWriter& writer, const hazelcast::client::connection::MembershipEvent& ar);
                template<typename HzReader>
                friend void hazelcast::client::serialization::readPortable(HzReader& reader, hazelcast::client::connection::MembershipEvent& ar);
                
            public:
                static const int MEMBER_ADDED = 1;
                
                static const int MEMBER_REMOVED = 3;
                
                MembershipEvent();
                
                MembershipEvent(const Member& member, int eventType);
                
                Member getMember() const;
                
                int getEventType() const;
                
            private:
                Member member;
                int eventType;
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getTypeId(const hazelcast::client::connection::MembershipEvent& x) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };
            
            inline int getFactoryId(const hazelcast::client::connection::MembershipEvent& ar) {
                return hazelcast::client::protocol::ProtocolConstants::DATA_FACTORY_ID;
            };
            
            inline int getClassId(const hazelcast::client::connection::MembershipEvent& ar) {
                return hazelcast::client::protocol::ProtocolConstants::MEMBERSHIP_EVENT;
            };
            
            
            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::connection::MembershipEvent& ar) {
                writer << ar.member;
                writer << ar.eventType;
            };
            
            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::connection::MembershipEvent& ar) {
                reader >> ar.member;
                reader >> ar.eventType;
            };
            
        }
    }
}
#endif //HAZELCAST_MEMBERSHIP_EVENT
