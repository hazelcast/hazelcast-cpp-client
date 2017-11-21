/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 25/06/14.
//


#ifndef HAZELCAST_SerializationConfig
#define HAZELCAST_SerializationConfig

#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/DataSerializableFactory.h"
#include "hazelcast/client/serialization/PortableFactory.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

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

            /**
             * @param factoryId               factory ID of DataSerializableFactory to be registered
             * @param dataSerializableFactory DataSerializableFactory object to be registered
             * @return configured {@link SerializerConfig} for chaining
             * @see DataSerializableFactory
             */
            SerializationConfig &addDataSerializableFactory(int32_t factoryId,
                                                            boost::shared_ptr<serialization::DataSerializableFactory> dataSerializableFactory);

            /**
             * @param factoryId       factory ID of portableFactory to be registered
             * @param portableFactory portableFactory object to be registered
             * @return configured {@link SerializerConfig} for chaining
             * @see PortableFactory
             */
            SerializationConfig &
            addPortableFactory(int32_t factoryId, boost::shared_ptr<serialization::PortableFactory> portableFactory);

            const std::map<int32_t, boost::shared_ptr<serialization::DataSerializableFactory> > &
            getDataSerializableFactories() const;

            const std::map<int32_t, boost::shared_ptr<serialization::PortableFactory> > &getPortableFactories() const;

        private:
            int version;
            std::vector<boost::shared_ptr<serialization::SerializerBase> > serializers;
            std::map<int32_t, boost::shared_ptr<serialization::DataSerializableFactory> > dataSerializableFactories;
            std::map<int32_t, boost::shared_ptr<serialization::PortableFactory> > portableFactories;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_SerializationConfig
