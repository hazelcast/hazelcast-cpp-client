/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include <memory>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {
        namespace serialization{
            struct global_serializer;
        }

        /**
         * SerializationConfig is used to
         *   * set version of portable classes in this client (@see portable_serializer)
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
            int get_portable_version() const;

            /**
             *
             * @param v
             * @return itself SerializationConfig
             */
            SerializationConfig& set_portable_version(int v);

            std::shared_ptr<serialization::global_serializer> get_global_serializer() const;

            void set_global_serializer(const std::shared_ptr<serialization::global_serializer> &global_serializer);
        private:
            int version_;
            std::shared_ptr<serialization::global_serializer> global_serializer_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 
