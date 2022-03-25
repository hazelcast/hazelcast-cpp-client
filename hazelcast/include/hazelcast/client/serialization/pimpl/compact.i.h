/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/serialization/pimpl/compact.h"

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {

template<typename T>
struct schema_of
{
    static schema build_schema()
    {
        T t;
        schema_writer schema_writer(hz_serializer<T>::type_name());
        serialization::compact_writer writer =
          create_compact_writer(&schema_writer);
        serialization::hz_serializer<T>::write(t, writer);
        return std::move(schema_writer).build();
    }

    const static schema schema_v;
};

template<typename T>
const schema schema_of<T>::schema_v = schema_of<T>::build_schema();

template<typename T>
T inline compact_stream_serializer::read(object_data_input& in)
{
    std::cout << "compact read " << std::endl;
    return T();
}

template<typename T>
void inline compact_stream_serializer::write(const T& object,
                                             object_data_output& out)
{
    const auto& schema_v = schema_of<T>::schema_v;
    put_to_schema_service(schema_v);
    out.write<int64_t>(schema_v.schema_id());
    default_compact_writer default_writer;
    compact_writer writer = create_compact_writer(&default_writer);
    hz_serializer<T>::write(object, writer);
    default_writer.end();
}

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast