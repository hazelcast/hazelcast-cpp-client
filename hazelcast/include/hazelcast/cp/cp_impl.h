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

#include <unordered_map>
#include <boost/thread/shared_mutex.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread/future.hpp>

#include "hazelcast/cp/cp.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client { namespace spi { class ClientContext; } }
    namespace cp {
        namespace internal {
            namespace session {
                class HAZELCAST_API proxy_session_manager {
                public:
                    /**
                     * Represents absence of a Raft session
                     */
                    static constexpr int64_t NO_SESSION_ID = -1;

                    proxy_session_manager(client::spi::ClientContext &client);

                    /**
                     * Increments acquire count of the session.
                     * Creates a new session if there is no session yet.
                     */
                    int64_t acquire_session(const raft_group_id &group_id);

                    /**
                     * Increments acquire count of the session.
                     * Creates a new session if there is no session yet.
                     */
                    int64_t acquire_session(const raft_group_id &group_id, int32_t count);

                    /**
                     * Decrements acquire count of the session.
                     * Returns silently if no session exists for the given id.
                     */
                    void release_session(const raft_group_id &group_id, int64_t session_id);

                    void release_session(const raft_group_id &group_id, int64_t session_id, int32_t count);

                    /**
                     * Invalidates the given session.
                     * No more heartbeats will be sent for the given session.
                     */
                    void invalidate_session(const raft_group_id &group_id, int64_t session_id);

                    int64_t get_session(const raft_group_id &group_id);

                    int64_t get_or_create_unique_thread_id(const raft_group_id &group_id);

                    /**
                     * Invokes a shutdown call on server to close all existing sessions.
                     */
                    void shutdown();

                    // for testing
                    int64_t get_session_acquire_count(const raft_group_id &group_id, int64_t session_id);

                private:
                    struct session_state {
                        session_state(int64_t id, int64_t ttl_millis);
                        session_state(const session_state &rhs);

                        int64_t id;
                        std::chrono::milliseconds ttl;
                        std::chrono::steady_clock::time_point creation_time;
                        std::atomic_int32_t acquire_count = {0};

                        bool is_valid() const;

                        bool is_in_use() const;

                        bool is_expired() const;

                        int64_t acquire(int32_t count);

                        void release(int32_t count);
                    };

                    struct session_response {
                        int64_t id;
                        int64_t ttl_millis;
                        int64_t heartbeat_millis;
                    };

                    static constexpr int64_t SHUTDOWN_TIMEOUT_SECONDS = 60;

                    client::spi::ClientContext &client_;
                    boost::shared_mutex lock_;
                    bool running_ = true;
                    std::atomic_bool scheduled_heartbeat_= {false};
                    std::unordered_map<raft_group_id, session_state> sessions_;
                    typedef std::pair<raft_group_id, int64_t> key_type;
                    std::unordered_map<key_type, int64_t, boost::hash<key_type>> thread_ids_;
                    std::shared_ptr<boost::asio::steady_timer> heartbeat_timer_;

                    session_state &get_or_create_session(const raft_group_id &group_id);

                    std::unordered_map<raft_group_id, session_state>::iterator
                    create_new_session(const raft_group_id &group_id);

                    session_response request_new_session(const raft_group_id &group_id);

                    void schedule_heartbeat_task(int64_t hearbeat_millis);

                    boost::future<client::protocol::ClientMessage> heartbeat(const raft_group_id &group_id, int64_t session_id);

                    boost::future<client::protocol::ClientMessage> close_session(const raft_group_id &group_id, int64_t session_id);

                    int64_t generate_thread_id(const raft_group_id &group_id);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
