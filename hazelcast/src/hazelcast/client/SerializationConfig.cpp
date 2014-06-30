//
// Created by sancar koyunlu on 25/06/14.
//

#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {

        SerializationConfig::SerializationConfig() : version(0) {

        }

        int SerializationConfig::getPortableVersion() const {
            return version;
        }

        SerializationConfig &SerializationConfig::setPortableVersion(int version) {
            this->version = version;
            return *this;
        }

        std::vector<boost::shared_ptr<serialization::SerializerBase> > const &SerializationConfig::getSerializers() const {
            return serializers;
        }

        SerializationConfig &SerializationConfig::registerSerializer(boost::shared_ptr<serialization::SerializerBase> serializer) {
            serializers.push_back(serializer);
            return *this;
        }
    }
}