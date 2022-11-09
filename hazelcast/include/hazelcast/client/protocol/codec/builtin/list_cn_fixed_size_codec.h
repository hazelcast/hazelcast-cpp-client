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

#include <vector>

#include "hazelcast/util/export.h"
#include "hazelcast/util/Optional.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
namespace client {
namespace protocol {
namespace codec {
namespace builtin {
class HAZELCAST_API list_cn_fixed_size_codec
{
public:
    template<typename T>
    static std::vector<util::optional<T>> decode(ClientMessage& msg)
    {
        msg.skip_frame_header_bytes();

        auto type = msg.get<byte>();
        auto count0 = msg.get<int32_t>();
        assert(count0 >= 0);
        auto count = static_cast<std::size_t>(count0);

        std::vector<util::optional<T>> res(count);

        auto element_type = static_cast<contains_nullable_list_type>(type);
        switch (element_type) {
            case contains_nullable_list_type::NULL_ONLY:
                for (std::size_t i = 0; i < count; ++i) {
                    res[i] = boost::none;
                }
                break;
            case contains_nullable_list_type::NOT_NULL_ONLY:
                for (std::size_t i = 0; i < count; ++i) {
                    res[i] = msg.get<T>();
                }
                break;
            default:
                assert(element_type == contains_nullable_list_type::MIXED);

                std::size_t read_count = 0;

                while (read_count < count) {
                    auto bit_mask = msg.get<byte>();

                    for (std::size_t i = 0;
                         i < ITEMS_PER_BITMASK && read_count < count;
                         i++) {
                        auto mask = 1 << i;

                        if ((bit_mask & mask) == mask) {
                            res[read_count] = msg.get<T>();
                        } else {
                            res[read_count] = boost::none;
                        }

                        ++read_count;
                    }
                }

                assert(read_count == res.size());
        }

        return res;
    }

private:
    enum class HAZELCAST_API contains_nullable_list_type
    {
        NULL_ONLY = 1,
        NOT_NULL_ONLY = 2,
        MIXED = 3
    };

    static constexpr std::size_t ITEMS_PER_BITMASK = 8;
};

} // namespace builtin
} // namespace codec
} // namespace protocol
} // namespace client
} // namespace hazelcast
