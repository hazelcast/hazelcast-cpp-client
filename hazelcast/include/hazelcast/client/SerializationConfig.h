//
// Created by sancar koyunlu on 25/06/14.
//


#ifndef HAZELCAST_SerializationConfig
#define HAZELCAST_SerializationConfig

#include "hazelcast/util/HazelcastDll.h"
#include <boost/shared_ptr.hpp>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization{
            class SerializerBase;
        }

        /**
         * SerializationConfig is used to
         *   * set version of portable classes in this client (@see Portable)
         *   * register custom serializers to be used (@see Serializer , @see SerializationConfig#registerSerializer)
         */
        class HAZELCAST_API SerializationConfig {
        public:

            /**
             * Constructor
             * default value of version is zero.
             */
            SerializationConfig();

            /**
             * Portable version will be used to differentiate two same class that have changes on it
             * , like adding/removing field or changing a type of a field.
             *
             *  @return version of portable classes that will be created by this client
             */
            int getPortableVersion() const;

            /**
             *
             * @param portableVersion
             * @return itself SerializationConfig
             */
            SerializationConfig& setPortableVersion(int portableVersion);

            /**
             *
             * @return vector of registered custom serializers
             */
            std::vector<boost::shared_ptr<serialization::SerializerBase> > const &getSerializers() const;

            /**
             * One can implement custom serializers other than Portable and IdentifiedDataSerializable
             * to be used in serialization of user objects. For details @see Serializer
             *
             * @param serializer custom serializer to be registered
             */
            SerializationConfig& registerSerializer(boost::shared_ptr<serialization::SerializerBase> serializer);

        private:
            int version;
            std::vector<boost::shared_ptr<serialization::SerializerBase> > serializers;
        };
    }
}

#endif //HAZELCAST_SerializationConfig
