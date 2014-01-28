//
// Created by sancar koyunlu on 28/01/14.
//


#ifndef HAZELCAST_MemberAttributeEvent
#define HAZELCAST_MemberAttributeEvent


#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/util/IOUtil.h"
#include <boost/smart_ptr/shared_ptr.hpp>

namespace hazelcast {
    namespace client {

        class Cluster;

        class Member;

        /**
         * Membership event fired when a new member is added
         * to the cluster and/or when a member leaves the cluster.
         *
         * @see MembershipListener
         */
        class MemberAttributeEvent : public MembershipEvent {
        public:
            enum MapOperationType {
                DELTA_MEMBER_PROPERTIES_OP_PUT = 2,
                DELTA_MEMBER_PROPERTIES_OP_REMOVE = 3
            };

            MemberAttributeEvent(Cluster &cluster, Member &member, MapOperationType operationType, const std::string &key, const std::string &value, util::IOUtil::PRIMITIVE_ID primitive_id);

            /*
             *
             *  enum MapOperationType {
             *   DELTA_MEMBER_PROPERTIES_OP_PUT = 2,
             *   DELTA_MEMBER_PROPERTIES_OP_REMOVE = 3
             *   };
             * @return map operation type put or removed
             */
            MapOperationType getOperationType() const;

            /**
             * @return key of changed attribute
             */
            const std::string &getKey() const;

            /**
             * Returns null if
             *  => given type T is not compatible with available type, or
             *  => MapOperationType is remove(DELTA_MEMBER_PROPERTIES_OP_REMOVE).
             * @return value of changed attribute.
             */
            template<typename T>
            boost::shared_ptr<T> getValue() const {
                T *tag;
                boost::shared_ptr<T> v(getValueResolved(tag));
                return value;
            }

        private:
            std::string key;
            std::string value;
            util::IOUtil::PRIMITIVE_ID primitive_id;
            MapOperationType operationType;

            int *getValueResolved(int *tag) const;

            float *getValueResolved(float *tag) const;

            short *getValueResolved(short *tag) const;

            long *getValueResolved(long *tag) const;

            byte *getValueResolved(byte *tag) const;

            bool *getValueResolved(bool *tag) const;

            std::string *getValueResolved(std::string *tag) const;

            double *getValueResolved(double *tag) const;
        };

    }
}

#endif //HAZELCAST_MemberAttributeEvent
