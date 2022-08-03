/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <string>

#include "hazelcast/util/export.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            /**
             * Configuration of an index. Hazelcast support two types of indexes: sorted index and hash index.
             * Sorted indexes could be used with equality and range predicates and have logarithmic search time.
             * Hash indexes could be used with equality predicates and have constant search time assuming the hash
             * function of the indexed field disperses the elements properly.
             * <p>
             * Index could be created on one or more attributes.
             *
             */
            struct HAZELCAST_API index_config {
                struct HAZELCAST_API bitmap_index_options {
                    enum unique_key_transformation {
                        /**
                         * Extracted unique key value is interpreted as an object value.
                         * Non-negative unique ID is assigned to every distinct object value.
                         */
                        OBJECT,

                        /**
                         * Extracted unique key value is interpreted as a whole integer value of
                         * byte, short, int or long type. The extracted value is upcasted to
                         * long (if necessary) and unique non-negative ID is assigned to every
                         * distinct value.
                         */
                        LONG,

                        /**
                         * Extracted unique key value is interpreted as a whole integer value of
                         * byte, short, int or long type. The extracted value is upcasted to
                         * long (if necessary) and the resulting value is used directly as an ID.
                         */
                        RAW
                    };

                    std::string key;
                    unique_key_transformation transformation;

                    /**
                     * The default for \key.
                     */
                    static const std::string DEFAULT_KEY;

                    /**
                     * The default for \transformation.
                     */
                    static const unique_key_transformation DEFAULT_TRANSFORMATION;

                    /**
                     * Constructs a new bitmap index options instance with all options set to
                     * default values.
                     */
                    bitmap_index_options();
                };

                enum index_type {
                    /** Sorted index. Can be used with equality and range predicates. */
                    SORTED,

                    /** Hash index. Can be used with equality predicates. */
                    HASH,

                    /** Bitmap index. Can be used with equality predicates. */
                    BITMAP
                };

                /** Name of the index. */
                boost::optional<std::string> name;

                /** Type of the index. */
                index_type type;

                /** Indexed attributes. */
                std::vector<std::string> attributes;

                boost::optional<bitmap_index_options> options;

                static const index_type DEFAULT_TYPE;

                index_config();

                /**
                 * Creates an index configuration of the given type.
                 *
                 * \param type Index type.
                 */
                index_config(index_type type);

                /**
                 * Creates an index configuration of the given type with provided attributes.
                 *
                 * \param type Index type.
                 * \param attributes Attributes to be indexed.
                 */
                 template<typename ...T>
                 index_config(index_type t, T... attrs) : type(t) {
                    add_attributes(std::forward<T>(attrs)...);
                 }

            private:
                template<typename ...T>
                void add_attributes(std::string attribute, T... attrs) {
                    attributes.emplace_back(std::move(attribute));
                    add_attributes(attrs...);
                }

                void add_attributes();
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
