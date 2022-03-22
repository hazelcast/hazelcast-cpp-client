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

//#include <unordered_map>
//#include <unordered_set>
//#include <type_traits>
//
//#include <boost/any.hpp>
//#include <boost/optional.hpp>
//#include <boost/optional/optional_io.hpp>
//#include <boost/uuid/uuid.hpp>
//#include <boost/thread/future.hpp>

//#include "hazelcast/client/hazelcast_json_value.h"
//#include "hazelcast/client/serialization/pimpl/data_input.h"
#include "hazelcast/client/serialization/pimpl/compact.h"
//#include "hazelcast/client/serialization/pimpl/data.h"
//#include "hazelcast/client/serialization/pimpl/data_output.h"
//#include "hazelcast/client/serialization_config.h"
//#include "hazelcast/client/partition_aware.h"
//#include "hazelcast/util/SynchronizedMap.h"
//#include "hazelcast/util/Disposable.h"

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {

inline CompactSerializer::CompactSerializer() {}
template<typename T>
T inline CompactSerializer::read(object_data_input& in)
{
    std::cout << "compact read " << std::endl;
    return T();
}

template<typename T>
void inline CompactSerializer::write(const T& object, object_data_output& out)
{
    std::cout << "compact write " << std::endl;
}

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast