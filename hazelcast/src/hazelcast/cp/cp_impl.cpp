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

#include <boost/thread/shared_lock_guard.hpp>
#include <boost/container_hash/hash.hpp>

#include "hazelcast/cp/cp_impl.h"
#include "hazelcast/cp/cp.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/proxy/SerializingProxy.h"

namespace hazelcast {
namespace cp {
namespace internal {
namespace session {
constexpr int64_t proxy_session_manager::NO_SESSION_ID;
constexpr int64_t proxy_session_manager::SHUTDOWN_TIMEOUT_SECONDS;
proxy_session_manager::proxy_session_manager(
  hazelcast::client::spi::ClientContext& client)
  : client_(client)
{}

int64_t
proxy_session_manager::acquire_session(const raft_group_id& group_id)
{
    return get_or_create_session(group_id).acquire(1);
}

int64_t
proxy_session_manager::acquire_session(const raft_group_id& group_id,
                                       int32_t count)
{
    return get_or_create_session(group_id).acquire(count);
}

proxy_session_manager::session_state&
proxy_session_manager::get_or_create_session(const raft_group_id& group_id)
{
    boost::upgrade_lock<boost::shared_mutex> read_lock(lock_);

    if (!running_) {
        BOOST_THROW_EXCEPTION(client::exception::hazelcast_instance_not_active(
          "proxy_session_manager::get_or_create_session",
          "Session manager is already shut down!"));
    }

    auto session = sessions_.find(group_id);
    if (session == sessions_.end() || !session->second.is_valid()) {
        // upgrade to write lock
        boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(
          read_lock);
        session = sessions_.find(group_id);
        if (session == sessions_.end() || !session->second.is_valid()) {
            session = create_new_session(group_id);
        }
    }
    return session->second;
}

std::unordered_map<raft_group_id,
                   proxy_session_manager::session_state>::iterator
proxy_session_manager::create_new_session(const raft_group_id& group_id)
{
    // the lock_ is already acquired as write lock
    auto response = request_new_session(group_id);
    auto result = sessions_.emplace(
      group_id, session_state{ response.id, response.ttl_millis });
    assert(result.second);
    auto session =
      sessions_
        .emplace(group_id, session_state{ response.id, response.ttl_millis })
        .first;
    schedule_heartbeat_task(response.heartbeat_millis);
    return session;
}

proxy_session_manager::session_response
proxy_session_manager::request_new_session(const raft_group_id& group_id)
{
    auto request = client::protocol::codec::cpsession_createsession_encode(
      group_id, client_.get_name());
    auto response = client::spi::impl::ClientInvocation::create(
                      client_, request, "sessionManager")
                      ->invoke()
                      .get();
    auto session_id = response.get_first_fixed_sized_field<int64_t>();
    auto ttl_millis = response.get<int64_t>();
    auto hearbeat_millis = response.get<int64_t>();
    return { session_id, ttl_millis, hearbeat_millis };
}

void
proxy_session_manager::schedule_heartbeat_task(int64_t hearbeat_millis)
{
    bool current = false;
    if (scheduled_heartbeat_.compare_exchange_strong(current, true)) {
        auto prev_heartbeats =
          std::make_shared<std::vector<boost::future<void>>>();
        auto duration = std::chrono::milliseconds(hearbeat_millis);
        heartbeat_timer_ =
          client_.get_client_execution_service().schedule_with_repetition(
            [=]() {
                // we can not cancel a future
                prev_heartbeats->clear();
                std::vector<std::tuple<raft_group_id, int64_t, bool>> sessions;
                {
                    boost::upgrade_lock<boost::shared_mutex> read_lock(lock_);
                    for (const auto& s : sessions_) {
                        sessions.emplace_back(
                          s.first, s.second.id, s.second.is_in_use());
                    }
                }
                for (const auto& entry : sessions) {
                    raft_group_id group_id;
                    int64_t session_id;
                    bool in_use;
                    std::tie(group_id, session_id, in_use) = entry;
                    if (in_use) {
                        prev_heartbeats->emplace_back(
                          heartbeat(group_id, session_id)
                            .then(
                              boost::launch::sync,
                              [=](boost::future<client::protocol::ClientMessage>
                                    f) {
                                  try {
                                      f.get();
                                  } catch (
                                    client::exception::session_expired&) {
                                      invalidate_session(group_id, session_id);
                                  } catch (
                                    client::exception::cp_group_destroyed&) {
                                      invalidate_session(group_id, session_id);
                                  }
                              }));
                    }
                }
            },
            duration,
            duration);
    }
}

boost::future<client::protocol::ClientMessage>
proxy_session_manager::heartbeat(const raft_group_id& group_id,
                                 int64_t session_id)
{
    auto request = client::protocol::codec::cpsession_heartbeatsession_encode(
      group_id, session_id);
    return client::spi::impl::ClientInvocation::create(
             client_, request, "sessionManager")
      ->invoke();
}

void
proxy_session_manager::invalidate_session(const raft_group_id& group_id,
                                          int64_t session_id)
{
    {
        boost::upgrade_lock<boost::shared_mutex> read_lock(lock_);
        auto session = sessions_.find(group_id);
        if (session != sessions_.end()) {
            // upgrade to write lock
            boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(
              read_lock);
            sessions_.erase(session);
        }
    }
}

void
proxy_session_manager::release_session(const raft_group_id& group_id,
                                       int64_t session_id)
{
    release_session(group_id, session_id, 1);
}

void
proxy_session_manager::release_session(const raft_group_id& group_id,
                                       int64_t session_id,
                                       int32_t count)
{
    boost::upgrade_lock<boost::shared_mutex> read_lock(lock_);
    auto session = sessions_.find(group_id);
    if (session != sessions_.end() && session->second.id == session_id) {
        session->second.release(count);
    }
}

int64_t
proxy_session_manager::get_session(const raft_group_id& group_id)
{
    boost::upgrade_lock<boost::shared_mutex> read_lock(lock_);
    auto session = sessions_.find(group_id);
    return session == sessions_.end() ? NO_SESSION_ID : session->second.id;
}

int64_t
proxy_session_manager::get_or_create_unique_thread_id(
  const raft_group_id& group_id)
{
    boost::upgrade_lock<boost::shared_mutex> read_lock(lock_);
    auto key = std::make_pair(group_id, util::get_current_thread_id());
    auto global_thread_id_it = thread_ids_.find(key);
    if (global_thread_id_it != thread_ids_.end()) {
        return global_thread_id_it->second;
    }

    auto global_thread_id = generate_thread_id(group_id);

    // upgrade to write lock
    boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);
    global_thread_id_it = thread_ids_.find(key);
    if (global_thread_id_it != thread_ids_.end()) {
        return global_thread_id_it->second;
    }

    thread_ids_.emplace(std::move(key), global_thread_id);
    return global_thread_id;
}

void
proxy_session_manager::shutdown()
{
    boost::unique_lock<boost::shared_mutex> write_lock(lock_);
    if (scheduled_heartbeat_ && heartbeat_timer_) {
        heartbeat_timer_->cancel();
    }

    std::vector<boost::future<client::protocol::ClientMessage>> invocations;
    for (const auto& s : sessions_) {
        invocations.emplace_back(close_session(s.first, s.second.id));
    }

    auto waiting_future =
      boost::when_all(invocations.begin(), invocations.end());

    waiting_future.wait_for(boost::chrono::seconds(SHUTDOWN_TIMEOUT_SECONDS));

    sessions_.clear();
    running_ = false;
}

boost::future<client::protocol::ClientMessage>
proxy_session_manager::close_session(const raft_group_id& group_id,
                                     int64_t session_id)
{
    auto request = client::protocol::codec::cpsession_closesession_encode(
      group_id, session_id);
    return client::spi::impl::ClientInvocation::create(
             client_, request, "sessionManager")
      ->invoke();
}

int64_t
proxy_session_manager::get_session_acquire_count(const raft_group_id& group_id,
                                                 int64_t session_id)
{
    boost::upgrade_lock<boost::shared_mutex> read_lock(lock_);
    auto session = sessions_.find(group_id);
    return session != sessions_.end() && session->second.id == session_id
             ? session->second.acquire_count.load()
             : 0;
}

int64_t
proxy_session_manager::generate_thread_id(const raft_group_id& group_id)
{
    auto request =
      client::protocol::codec::cpsession_generatethreadid_encode(group_id);
    return client::spi::impl::ClientInvocation::create(
             client_, request, "sessionManager")
      ->invoke()
      .get()
      .get_first_fixed_sized_field<int64_t>();
}

bool
proxy_session_manager::session_state::is_valid() const
{
    return is_in_use() || !is_expired();
}

bool
proxy_session_manager::session_state::is_in_use() const
{
    return acquire_count.load() > 0;
}

bool
proxy_session_manager::session_state::is_expired() const
{
    auto expirationTime = creation_time + ttl;
    if (expirationTime.time_since_epoch().count() < 0) {
        expirationTime = (std::chrono::steady_clock::time_point::max)();
    }
    return std::chrono::steady_clock::now() > expirationTime;
}

proxy_session_manager::session_state::session_state(int64_t id,
                                                    int64_t ttl_millis)
  : id(id)
  , ttl(ttl_millis)
  , creation_time(std::chrono::steady_clock::now())
{}

proxy_session_manager::session_state::session_state(const session_state& rhs)
  : id(rhs.id)
  , ttl(rhs.ttl)
  , creation_time(rhs.creation_time)
  , acquire_count(rhs.acquire_count.load())
{}

int64_t
proxy_session_manager::session_state::acquire(int32_t count)
{
    acquire_count.fetch_add(count);
    return id;
}

void
proxy_session_manager::session_state::release(int32_t count)
{
    acquire_count.fetch_sub(count);
}

} // namespace session
} // namespace internal
} // namespace cp
} // namespace hazelcast
