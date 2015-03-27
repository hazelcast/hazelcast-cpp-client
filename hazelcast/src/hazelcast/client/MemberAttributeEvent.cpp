//
// Created by sancar koyunlu on 28/01/14.
//

#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/Cluster.h"

namespace hazelcast {
    namespace client {

        MemberAttributeEvent::MemberAttributeEvent(Cluster &cluster, Member &member, MemberAttributeOperationType operationType, const std::string &key, const std::string &value, util::IOUtil::PRIMITIVE_ID primitive_id)
        :MembershipEvent(cluster, MembershipEvent::MEMBER_ATTRIBUTE_CHANGED, member)
        , operationType(operationType)
        , key(key)
        , value(value)
        , primitive_id(primitive_id) {

        }

        MemberAttributeEvent::MemberAttributeOperationType MemberAttributeEvent::getOperationType() const {
            return operationType;
        }

        const std::string &MemberAttributeEvent::getKey() const {
            return key;
        }

        int *MemberAttributeEvent::getValueResolved(int *tag) const {
            if (primitive_id != util::IOUtil::PRIMITIVE_TYPE_INTEGER)
                return NULL;
            return new int(hazelcast::util::IOUtil::to_value<int>(value));
        }

        float *MemberAttributeEvent::getValueResolved(float *tag) const {
            if (primitive_id != util::IOUtil::PRIMITIVE_TYPE_FLOAT)
                return NULL;
            return new float(hazelcast::util::IOUtil::to_value<float>(value));
        }

        short *MemberAttributeEvent::getValueResolved(short *tag) const {
            if (primitive_id != util::IOUtil::PRIMITIVE_TYPE_SHORT)
                return NULL;
            return new short(hazelcast::util::IOUtil::to_value<short>(value));
        }

        long *MemberAttributeEvent::getValueResolved(long *tag) const {
            if (primitive_id != util::IOUtil::PRIMITIVE_TYPE_LONG)
                return NULL;
            return new long(hazelcast::util::IOUtil::to_value<long>(value));
        }

        byte *MemberAttributeEvent::getValueResolved(byte *tag) const {
            if (primitive_id != util::IOUtil::PRIMITIVE_TYPE_BYTE)
                return NULL;
            return new byte(hazelcast::util::IOUtil::to_value<byte>(value));
        }

        bool *MemberAttributeEvent::getValueResolved(bool *tag) const {
            if (primitive_id != util::IOUtil::PRIMITIVE_TYPE_BOOLEAN)
                return NULL;
            return new bool(hazelcast::util::IOUtil::to_value<bool>(value));
        }

        double *MemberAttributeEvent::getValueResolved(double *tag) const {
            if (primitive_id != util::IOUtil::PRIMITIVE_TYPE_DOUBLE)
                return NULL;
            return new double(hazelcast::util::IOUtil::to_value<double>(value));
        }

        std::string *MemberAttributeEvent::getValueResolved(std::string *tag) const {
            if (primitive_id != util::IOUtil::PRIMITIVE_TYPE_UTF)
                return NULL;
            return new std::string(value);
        }
    }
}
