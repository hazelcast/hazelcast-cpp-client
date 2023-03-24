/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/uuid/uuid.hpp>
#include <boost/optional.hpp>

#include "hazelcast/util/byte.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
namespace client {
namespace connection {

class authentication_response
{
    static constexpr int AUTHENTICATION_CODEC_RESPONSE_MESSAGE_TYPE = 257;
    static constexpr int AUTHENTICATION_CUSTOM_CODEC_RESPONSE_MESSAGE_TYPE =
      513;
    static constexpr int
      EXPERIMENTAL_AUTHENTICATION_CODEC_RESPONSE_MESSAGE_TYPE = 16580865;
    static constexpr int
      EXPERIMENTAL_AUTHENTICATION_CUSTOM_CODEC_RESPONSE_MESSAGE_TYPE = 16581121;

public:
    authentication_response() = default;
    authentication_response(byte auth_status,
                            boost::optional<address> address,
                            boost::uuids::uuid member_uuid,
                            byte serialization_version,
                            std::string server_version,
                            int32_t partition_count,
                            boost::uuids::uuid cluster_id,
                            std::vector<int> tpc_ports);

    static authentication_response from(protocol::ClientMessage response);

    byte status() const;
    boost::optional<address> addr() const;
    boost::uuids::uuid member_uuid() const;
    byte serialization_version() const;
    const std::string& server_hazelcast_version() const;
    int partition_count() const;
    boost::uuids::uuid cluster_id() const;
    const std::vector<int>& tpc_ports() const;

private:
    static authentication_response from_authentication_codec(
      protocol::ClientMessage response);
    static authentication_response from_authentication_custom_codec(
      protocol::ClientMessage response);
    static authentication_response from_experimental_authentication_codec(
      protocol::ClientMessage response);
    static authentication_response
    from_experimental_authentication_custom_codec(
      protocol::ClientMessage response);
    static authentication_response decode(protocol::ClientMessage response,
                                          bool with_tpc_ports);

    byte status_;
    boost::optional<address> server_address_;
    boost::uuids::uuid member_uuid_;
    byte serialization_version_;
    std::string server_version_;
    int32_t partition_count_;
    boost::uuids::uuid cluster_id_;
    std::vector<int> tpc_ports_;
};

} // namespace connection
} // namespace client
} // namespace hazelcast