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
            enum MemberAttributeOperationType {
                PUT = 1,
                REMOVE = 2
            };

            MemberAttributeEvent(Cluster &cluster, Member &member, MemberAttributeOperationType operationType, const std::string &key, const std::string &value, util::IOUtil::PRIMITIVE_ID primitive_id);

            /*
             *
             *  enum MemberAttributeOperationType {
             *   PUT = 2,
             *   REMOVE = 3
             *   };
             * @return map operation type put or removed
             */
            MemberAttributeOperationType getOperationType() const;

            /**
             * @return key of changed attribute
             */
            const std::string &getKey() const;

            /**
             * Returns null if
             *  => given type T is not compatible with available type, or
             *  => MemberAttributeOperationType is remove(REMOVE).
             * @return value of changed attribute.
             */
            template<typename T>
            boost::shared_ptr<T> getValue() const {
                T *tag = NULL;
                boost::shared_ptr<T> v(getValueResolved(tag));
                return v;
            }

        private:
            std::string key;
            std::string value;
            util::IOUtil::PRIMITIVE_ID primitive_id;
            MemberAttributeOperationType operationType;

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
