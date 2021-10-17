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

#include <string>
#include <memory>

#include <hazelcast/client/hazelcast_client.h>

#include "ClientTest.h"
#include "HazelcastServer.h"
#include "IdentifiedSerializables.h"
#include "TestHelperFunctions.h"
#include "remote_controller_client.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace cp {
                template<typename T>
                class cp_test : public ClientTest
            {
                protected:
                    virtual std::shared_ptr<T> get_cp_structure(const std::string &name) = 0;

                    virtual client_config get_client_config() {
                        client_config config = get_config();
                        config.set_cluster_name ("cp-test");
                        return config;
                    }

                    virtual void SetUp() {
                        client_.reset(new hazelcast_client(new_client(get_client_config()).get()));
                        auto test_name = get_test_name();
                        cp_structure_ = get_cp_structure(test_name + "@cp_test_group");
                    }

                    virtual void TearDown() {
                        if (cp_structure_) {
                            cp_structure_->destroy();
                        }
                    }

                    static void SetUpTestCase() {
                        if (std::string(testing::UnitTest::GetInstance()->current_test_suite()->name()) == "basic_sessionless_semaphore_test") {
                            factory = new HazelcastServerFactory("hazelcast/test/resources/hazelcast-cp-sessionless-semaphore.xml");
                        } else {
                            factory = new HazelcastServerFactory("hazelcast/test/resources/hazelcast-cp.xml");
                        }
                        server1 = new HazelcastServer(*factory);
                        server2 = new HazelcastServer(*factory);
                        server3 = new HazelcastServer(*factory);
                    }

                    static void TearDownTestCase() {
                        delete server1;
                        delete server2;
                        delete server3;
                        delete factory;
                    }

                    static HazelcastServerFactory *factory;
                    static HazelcastServer *server1;
                    static HazelcastServer *server2;
                    static HazelcastServer *server3;
                    std::unique_ptr<hazelcast_client> client_;

                    std::shared_ptr<T> cp_structure_;
                };

                template<typename T> HazelcastServerFactory *cp_test<T>::factory = nullptr;
                template<typename T> HazelcastServer *cp_test<T>::server1 = nullptr;
                template<typename T> HazelcastServer *cp_test<T>::server2 = nullptr;
                template<typename T> HazelcastServer *cp_test<T>::server3 = nullptr;

                class basic_atomic_long_test : public cp_test<hazelcast::cp::atomic_long> {
                protected:
                    std::shared_ptr<hazelcast::cp::atomic_long> get_cp_structure(const std::string &name) override {
                        return client_->get_cp_subsystem().get_atomic_long(name).get();
                    }
                };
                
                TEST_F(basic_atomic_long_test, create_proxy_on_metadata_cp_group) {
                    ASSERT_THROW(client_->get_cp_subsystem().get_atomic_long("long@METADATA").get(),
                                 exception::illegal_argument);
                }

                TEST_F(basic_atomic_long_test, test_set) {
                    cp_structure_->set(271).get();
                    ASSERT_EQ(271, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_get) {
                    ASSERT_EQ(0, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_get_and_decrement) {
                    ASSERT_EQ(0, cp_structure_->get_and_decrement().get());
                    ASSERT_EQ(-1, cp_structure_->get_and_decrement().get());
                }

                TEST_F(basic_atomic_long_test, test_decrement_and_get) {
                    ASSERT_EQ(-1, cp_structure_->decrement_and_get().get());
                    ASSERT_EQ(-2, cp_structure_->decrement_and_get().get());
                }

                TEST_F(basic_atomic_long_test, test_get_and_increment) {
                    ASSERT_EQ(0, cp_structure_->get_and_increment().get());
                    ASSERT_EQ(1, cp_structure_->get_and_increment().get());
                }

                TEST_F(basic_atomic_long_test, test_increment_and_get) {
                    ASSERT_EQ(1, cp_structure_->increment_and_get().get());
                    ASSERT_EQ(2, cp_structure_->increment_and_get().get());
                }

                TEST_F(basic_atomic_long_test, test_get_and_set) {
                    ASSERT_EQ(0, cp_structure_->get_and_set(271).get());
                    ASSERT_EQ(271, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_add_and_get) {
                    ASSERT_EQ(271, cp_structure_->add_and_get(271).get());
                    ASSERT_EQ(271, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_get_and_add) {
                    ASSERT_EQ(0, cp_structure_->get_and_add(271).get());
                    ASSERT_EQ(271, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_compare_and_set_when_success) {
                    ASSERT_TRUE(cp_structure_->compare_and_set(0, 271).get());
                    ASSERT_EQ(271, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_compare_and_set_when_not_success) {
                    ASSERT_FALSE(cp_structure_->compare_and_set(172, 0).get());
                    ASSERT_EQ(0, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_alter) {
                    cp_structure_->set(2).get();
                    ASSERT_NO_THROW(cp_structure_->alter(multiplication{5}).get());
                    ASSERT_EQ(10, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_alter_and_get) {
                    cp_structure_->set(2).get();
                    auto result = cp_structure_->alter_and_get(multiplication{5}).get();
                    ASSERT_EQ(10, result);
                    ASSERT_EQ(10, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_get_and_alter) {
                    cp_structure_->set(2).get();
                    auto result = cp_structure_->get_and_alter(multiplication{5}).get();
                    ASSERT_EQ(2, result);
                    ASSERT_EQ(10, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_apply) {
                    cp_structure_->set(2).get();
                    auto result = cp_structure_->apply<multiplication, int64_t>({5}).get();
                    ASSERT_TRUE(result);
                    ASSERT_EQ(10, *result);
                    ASSERT_EQ(2, cp_structure_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_use_after_destroy) {
                    cp_structure_->destroy().get();
                    ASSERT_THROW(cp_structure_->increment_and_get().get(),
                                 exception::distributed_object_destroyed);
                }

                TEST_F(basic_atomic_long_test, test_create_after_destroy) {
                    auto name = cp_structure_->get_name();
                    cp_structure_->destroy().get();

                    cp_structure_ = client_->get_cp_subsystem().get_atomic_long(name).get();
                    ASSERT_THROW(cp_structure_->increment_and_get().get(),
                                 exception::distributed_object_destroyed);
                }

                TEST_F(basic_atomic_long_test, test_multiple_destroy) {
                    ASSERT_NO_THROW(cp_structure_->destroy().get());
                    ASSERT_NO_THROW(cp_structure_->destroy().get());
                }

                class basic_atomic_ref_test : public cp_test<hazelcast::cp::atomic_reference> {
                protected:
                    std::shared_ptr<hazelcast::cp::atomic_reference>
                    get_cp_structure(const std::string &name) override {
                        return client_->get_cp_subsystem().get_atomic_reference(name).get();
                    }
                };

                TEST_F(basic_atomic_ref_test, create_proxy_on_metadata_cp_group) {
                    ASSERT_THROW(client_->get_cp_subsystem().get_atomic_reference("ref@METADATA").get(),
                                 exception::illegal_argument);
                }

                TEST_F(basic_atomic_ref_test, test_set) {
                    cp_structure_->set(std::string("str1")).get();
                    ASSERT_EQ("str1", *cp_structure_->get<std::string>().get());
                    ASSERT_EQ("str1", *cp_structure_->get_and_set<std::string>("str2").get());
                    ASSERT_EQ("str2", *cp_structure_->get<std::string>().get());
                }

                TEST_F(basic_atomic_ref_test, test_compare_and_set) {
                    std::string str1("str1");
                    std::string str2("str2");
                    ASSERT_TRUE(cp_structure_->compare_and_set(static_cast<std::string *>(nullptr), &str1).get());
                    ASSERT_EQ(str1, *cp_structure_->get<std::string>().get());
                    ASSERT_FALSE(cp_structure_->compare_and_set(static_cast<std::string *>(nullptr), &str1).get());
                    ASSERT_TRUE(cp_structure_->compare_and_set(str1, str2).get());
                    ASSERT_EQ(str2, *cp_structure_->get<std::string>().get());
                    ASSERT_FALSE(cp_structure_->compare_and_set(str1, str2).get());
                    ASSERT_TRUE(cp_structure_->compare_and_set(&str2, static_cast<std::string *>(nullptr)).get());
                    ASSERT_FALSE(cp_structure_->get<std::string>().get());
                    ASSERT_FALSE(cp_structure_->compare_and_set(&str2, static_cast<std::string *>(nullptr)).get());
                }

                TEST_F(basic_atomic_ref_test, test_is_null) {
                    ASSERT_TRUE(cp_structure_->is_null().get());
                    cp_structure_->set(std::string("str1")).get();
                    ASSERT_FALSE(cp_structure_->is_null().get());
                }

                TEST_F(basic_atomic_ref_test, test_clear) {
                    cp_structure_->set(std::string("str1")).get();
                    cp_structure_->clear().get();
                    ASSERT_TRUE(cp_structure_->is_null().get());
                }

                TEST_F(basic_atomic_ref_test, test_contains) {
                    std::string str1("str1");
                    ASSERT_TRUE(cp_structure_->contains(static_cast<std::string *>(nullptr)).get());
                    ASSERT_FALSE(cp_structure_->contains(str1).get());

                    cp_structure_->set(str1).get();

                    ASSERT_FALSE(cp_structure_->contains(static_cast<std::string *>(nullptr)).get());
                    ASSERT_TRUE(cp_structure_->contains(str1).get());
                }

                TEST_F(basic_atomic_ref_test, test_alter) {
                    cp_structure_->set(std::string("str1")).get();

                    cp_structure_->alter(test::append_string{"str2"}).get();

                    ASSERT_EQ("str1str2", *cp_structure_->get<std::string>().get());

                    auto val = cp_structure_->alter_and_get<std::string>(test::append_string{"str3"}).get();
                    ASSERT_TRUE(val);
                    ASSERT_EQ("str1str2str3", *val);

                    val = cp_structure_->get_and_alter<std::string>(test::append_string{"str4"}).get();
                    ASSERT_TRUE(val);
                    ASSERT_EQ("str1str2str3", *val);

                    ASSERT_EQ("str1str2str3str4", *cp_structure_->get<std::string>().get());
                }

                TEST_F(basic_atomic_ref_test, test_apply) {
                    cp_structure_->set(std::string("str1")).get();

                    auto val = cp_structure_->apply<std::string>(test::append_string{"str2"}).get();
                    ASSERT_TRUE(val);
                    ASSERT_EQ("str1str2", *val);
                    ASSERT_EQ("str1", *cp_structure_->get<std::string>().get());
                }

                TEST_F(basic_atomic_ref_test, test_use_after_destroy) {
                    cp_structure_->destroy().get();
                    ASSERT_THROW((cp_structure_->set(std::string("str1")).get()),
                                 exception::distributed_object_destroyed);
                }

                TEST_F(basic_atomic_ref_test, test_create_after_destroy) {
                    auto name = cp_structure_->get_name();
                    cp_structure_->destroy().get();

                    cp_structure_ = client_->get_cp_subsystem().get_atomic_reference(name).get();
                    ASSERT_THROW(cp_structure_->set(std::string("str1")).get(),
                                 exception::distributed_object_destroyed);
                }

                TEST_F(basic_atomic_ref_test, test_multiple_destroy) {
                    ASSERT_NO_THROW(cp_structure_->destroy().get());
                    ASSERT_NO_THROW(cp_structure_->destroy().get());
                }

                class basic_latch_test : public cp_test<hazelcast::cp::latch> {
                protected:
                    std::shared_ptr<hazelcast::cp::latch> get_cp_structure(const std::string &name) override {
                        return client_->get_cp_subsystem().get_latch(name).get();
                    }
                };

                TEST_F(basic_latch_test, create_proxy_on_metadata_cp_group) {
                    ASSERT_THROW(client_->get_cp_subsystem().get_latch("ref@METADATA").get(),
                                 exception::illegal_argument);
                }

                TEST_F(basic_latch_test, test_try_set_count_when_argument_negative) {
                    ASSERT_THROW(cp_structure_->try_set_count(-20).get(), exception::illegal_argument);
                }

                TEST_F(basic_latch_test, test_try_set_count_when_argument_zero) {
                    ASSERT_THROW(cp_structure_->try_set_count(0).get(), exception::illegal_argument);
                }

                TEST_F(basic_latch_test, test_try_set_count_when_count_is_not_zero) {
                    ASSERT_TRUE(cp_structure_->try_set_count(10).get());

                    ASSERT_FALSE(cp_structure_->try_set_count(20).get());
                    ASSERT_FALSE(cp_structure_->try_set_count(1).get());
                    ASSERT_EQ(10, cp_structure_->get_count().get());
                }

                TEST_F(basic_latch_test, test_try_set_count_when_already_set) {
                    ASSERT_TRUE(cp_structure_->try_set_count(10).get());

                    ASSERT_FALSE(cp_structure_->try_set_count(20).get());
                    ASSERT_FALSE(cp_structure_->try_set_count(100).get());
                    ASSERT_FALSE(cp_structure_->try_set_count(1).get());
                    ASSERT_EQ(10, cp_structure_->get_count().get());
                }

                TEST_F(basic_latch_test, test_count_down) {
                    cp_structure_->try_set_count(20).get();

                    for (int i = 19; i >= 0; i--) {
                        cp_structure_->count_down().get();
                        ASSERT_EQ(i, cp_structure_->get_count().get());
                    }

                    cp_structure_->count_down().get();
                    ASSERT_EQ(0, cp_structure_->get_count().get());
                }

                TEST_F(basic_latch_test, test_get_count) {
                    cp_structure_->try_set_count(20).get();

                    ASSERT_EQ(20, cp_structure_->get_count().get());
                }

                TEST_F(basic_latch_test, test_wait_for) {
                    cp_structure_->try_set_count(1).get();
                    std::thread([=]() {
                        cp_structure_->count_down().get();
                    }).detach();

                    ASSERT_OPEN_EVENTUALLY_ASYNC(cp_structure_);
                }

                TEST_F(basic_latch_test, test_wait_until) {
                    cp_structure_->try_set_count(1).get();
                    std::thread([=]() {
                        cp_structure_->count_down().get();
                    }).detach();

                    ASSERT_EQ(std::cv_status::no_timeout,
                              cp_structure_->wait_until(std::chrono::steady_clock::now() + std::chrono::seconds(120)).get());
                }

                TEST_F(basic_latch_test, test_wait) {
                    cp_structure_->try_set_count(1).get();
                    std::thread([=]() {
                        try {
                            cp_structure_->count_down().get();
                        } catch (exception::hazelcast_client_not_active &) {
                            // can get this exception if below wait finishes earlier and client is shutting down
                        }
                    }).detach();

                    cp_structure_->wait().get();
                }

                TEST_F(basic_latch_test, test_wait_for_when_timeout) {
                    cp_structure_->try_set_count(1).get();
                    auto start = std::chrono::steady_clock::now();
                    ASSERT_EQ(std::cv_status::timeout, cp_structure_->wait_for(std::chrono::milliseconds(100)).get());
                    auto elapsed = std::chrono::steady_clock::now() - start;
                    ASSERT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(), 100);
                    ASSERT_EQ(1, cp_structure_->get_count().get());
                }

                TEST_F(basic_latch_test, test_count_down_after_destroy) {
                    cp_structure_->destroy().get();

                    ASSERT_THROW(cp_structure_->count_down().get(), exception::distributed_object_destroyed);
                }

                TEST_F(basic_latch_test, test_multiple_destroy) {
                    ASSERT_NO_THROW(cp_structure_->destroy().get());
                    ASSERT_NO_THROW(cp_structure_->destroy().get());
                }

                class basic_lock_test : public cp_test<hazelcast::cp::fenced_lock> {
                protected:
                    static constexpr size_t LOCK_SERVICE_WAIT_TIMEOUT_TASK_UPPER_BOUND_MILLIS = 1500; // msecs

                    std::shared_ptr<hazelcast::cp::fenced_lock> get_cp_structure(const std::string &name) override {
                        return client_->get_cp_subsystem().get_lock(name).get();
                    }

                    void try_lock(const std::chrono::milliseconds timeout) {
                        std::async([=]() {
                            cp_structure_->lock().get();
                        }).get();

                        auto fence = cp_structure_->try_lock_and_get_fence(timeout).get();

                        ASSERT_EQ(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);
                        ASSERT_TRUE(cp_structure_->is_locked().get());
                        ASSERT_FALSE(cp_structure_->is_locked_by_current_thread().get());
                        ASSERT_EQ(1, cp_structure_->get_lock_count().get());
                    }

                    void close_session(const hazelcast::cp::raft_group_id &group_id, int64_t session_id) {
                        auto request = client::protocol::codec::cpsession_closesession_encode(group_id, session_id);
                        auto context = spi::ClientContext(*client_);
                        spi::impl::ClientInvocation::create(context, request, "sessionManager")->invoke().get();
                    }

                    void test_when_session_closed(std::function<void()> f) {
                        auto fence = cp_structure_->lock_and_get_fence().get();
                        ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);

                        auto group_id = cp_structure_->get_group_id();
                        auto session_id = spi::ClientContext(*client_).get_proxy_session_manager().get_session(
                                group_id);
                        close_session(group_id, session_id);

                        ASSERT_THROW(f(), exception::lock_ownership_lost);
                    }

                    void test_when_new_session_created(std::function<void()> f) {
                        auto fence = cp_structure_->lock_and_get_fence().get();
                        ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);

                        auto group_id = cp_structure_->get_group_id();
                        auto session_id = spi::ClientContext(*client_).get_proxy_session_manager().get_session(
                                group_id);
                        close_session(group_id, session_id);

                        std::async([=]() { cp_structure_->lock().get(); }).get();

                        // now we have a new session
                        try {
                            f();
                        } catch (exception::lock_ownership_lost &) {
                            // ignored
                        }

                        ASSERT_FALSE(cp_structure_->is_locked_by_current_thread().get());
                    }
                };

                TEST_F(basic_lock_test, test_lock_when_not_locked) {
                    auto fence = cp_structure_->lock_and_get_fence().get();
                    ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);
                    ASSERT_TRUE(cp_structure_->is_locked_by_current_thread().get());
                    ASSERT_EQ(1, cp_structure_->get_lock_count().get());
                    ASSERT_EQ(fence, cp_structure_->get_fence().get());
                }

                TEST_F(basic_lock_test, test_lock_when_locked_by_self) {
                    auto fence = cp_structure_->lock_and_get_fence().get();
                    ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);

                    auto new_fence = cp_structure_->lock_and_get_fence().get();
                    ASSERT_EQ(fence, new_fence);
                    ASSERT_TRUE(cp_structure_->is_locked_by_current_thread().get());
                    ASSERT_EQ(2, cp_structure_->get_lock_count().get());
                }

                TEST_F(basic_lock_test, test_lock_when_locked_by_other) {
                    auto fence = cp_structure_->lock_and_get_fence().get();
                    ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);
                    ASSERT_TRUE(cp_structure_->is_locked().get());
                    ASSERT_EQ(1, cp_structure_->get_lock_count().get());
                    ASSERT_TRUE(cp_structure_->is_locked_by_current_thread().get());

                    boost::latch start(1);
                    auto f = std::async([=, &start]() {
                        auto invocation = cp_structure_->lock();
                        start.count_down();
                        invocation.get();
                        return true;
                    });

                    ASSERT_EQ(boost::cv_status::no_timeout, start.wait_for(boost::chrono::seconds(120)));
                    ASSERT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(3)));
                    // the following line is ready for the async task to finish and hence the test can finish
                    cp_structure_->destroy().get();
                }

                TEST_F(basic_lock_test, test_unlock_when_free) {
                    ASSERT_THROW(cp_structure_->unlock().get(), exception::illegal_monitor_state);
                }

                TEST_F(basic_lock_test, test_get_fence_when_free) {
                    ASSERT_THROW(cp_structure_->get_fence().get(), exception::illegal_monitor_state);
                }

                TEST_F(basic_lock_test, test_is_locked_when_free) {
                    ASSERT_FALSE(cp_structure_->is_locked().get());
                }

                TEST_F(basic_lock_test, test_lock_is_acquired_by_current_thread_when_free) {
                    ASSERT_FALSE(cp_structure_->is_locked_by_current_thread().get());
                }

                TEST_F(basic_lock_test, test_get_lock_count_when_free) {
                    ASSERT_EQ(0,cp_structure_->get_lock_count().get());
                }

                TEST_F(basic_lock_test, test_unlock_when_locked_by_self) {
                    cp_structure_->lock().get();

                    cp_structure_->unlock().get();

                    ASSERT_FALSE(cp_structure_->is_locked().get());
                    ASSERT_EQ(0, cp_structure_->get_lock_count().get());

                    ASSERT_THROW(cp_structure_->get_fence().get(), exception::illegal_monitor_state);
                }

                TEST_F(basic_lock_test, test_unlock_when_reentrantly_locked_by_self) {
                    auto fence = cp_structure_->lock_and_get_fence().get();
                    ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);
                    cp_structure_->lock().get();
                    cp_structure_->unlock().get();

                    ASSERT_TRUE(cp_structure_->is_locked_by_current_thread().get());
                    ASSERT_TRUE(cp_structure_->is_locked().get());
                    ASSERT_EQ(1, cp_structure_->get_lock_count().get());
                    ASSERT_EQ(fence, cp_structure_->get_fence().get());
                }

                TEST_F(basic_lock_test, test_lock_unlock_then_lock) {
                    auto fence = cp_structure_->lock_and_get_fence().get();
                    ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);
                    cp_structure_->unlock().get();

                    auto f = std::async([=]() {
                        return cp_structure_->lock_and_get_fence().get();
                    });

                    ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(120)));

                    ASSERT_GE(f.get(), fence);
                    ASSERT_TRUE(cp_structure_->is_locked().get());
                    ASSERT_EQ(1, cp_structure_->get_lock_count().get());
                    ASSERT_FALSE(cp_structure_->is_locked_by_current_thread().get());

                    ASSERT_THROW(cp_structure_->get_fence().get(), exception::illegal_monitor_state);
                }


                TEST_F(basic_lock_test, test_lock_unlock_when_pending_lock_of_other_thread) {
                    auto fence = cp_structure_->lock_and_get_fence().get();
                    ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);

                    boost::latch start(1);
                    auto f = std::async([=, &start]() {
                        auto invocation = cp_structure_->try_lock_and_get_fence(std::chrono::seconds(60));
                        start.count_down();
                        return invocation.get();
                    });

                    ASSERT_EQ(boost::cv_status::no_timeout, start.wait_for(boost::chrono::seconds(120)));

                    cp_structure_->unlock().get();

                    ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(120)));
                    ASSERT_GE(f.get(), fence);

                    ASSERT_TRUE(cp_structure_->is_locked().get());
                    ASSERT_FALSE(cp_structure_->is_locked_by_current_thread().get());
                    ASSERT_EQ(1, cp_structure_->get_lock_count().get());
                    ASSERT_THROW(cp_structure_->get_fence().get(), exception::illegal_monitor_state);
                }

                TEST_F(basic_lock_test, test_unlock_when_locked_by_other) {
                    std::async([=] () {
                        cp_structure_->lock().get();
                    }).get();

                    ASSERT_THROW(cp_structure_->unlock().get(), exception::illegal_monitor_state);

                    ASSERT_TRUE(cp_structure_->is_locked().get());
                    ASSERT_FALSE(cp_structure_->is_locked_by_current_thread().get());
                    ASSERT_EQ(1, cp_structure_->get_lock_count().get());
                }

                TEST_F(basic_lock_test, test_try_lock_when_not_locked) {
                    auto fence = cp_structure_->try_lock_and_get_fence().get();

                    ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);
                    ASSERT_EQ(fence, cp_structure_->get_fence().get());
                    ASSERT_TRUE(cp_structure_->is_locked_by_current_thread().get());
                    ASSERT_EQ(1, cp_structure_->get_lock_count().get());
                }

                TEST_F(basic_lock_test, test_try_lock_when_locked_by_self) {
                    auto fence = cp_structure_->lock_and_get_fence().get();
                    ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);

                    auto new_fence = cp_structure_->try_lock_and_get_fence().get();
                    ASSERT_EQ(fence, new_fence);
                    ASSERT_EQ(fence, cp_structure_->get_fence().get());
                    ASSERT_TRUE(cp_structure_->is_locked_by_current_thread().get());
                    ASSERT_EQ(2, cp_structure_->get_lock_count().get());
                }

                TEST_F(basic_lock_test, test_try_lock_timeout) {
                    auto fence = cp_structure_->try_lock_and_get_fence(std::chrono::seconds(1)).get();

                    ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);
                    ASSERT_TRUE(cp_structure_->is_locked_by_current_thread().get());
                    ASSERT_EQ(1, cp_structure_->get_lock_count().get());
                }

                TEST_F(basic_lock_test, test_try_lock_timeout_when_locked_by_self) {
                    auto fence = cp_structure_->lock_and_get_fence().get();
                    ASSERT_NE(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);

                    auto new_fence = cp_structure_->try_lock_and_get_fence(std::chrono::seconds(1)).get();

                    ASSERT_EQ(fence, new_fence);
                    ASSERT_EQ(fence, cp_structure_->get_fence().get());
                    ASSERT_TRUE(cp_structure_->is_locked_by_current_thread().get());
                    ASSERT_EQ(2, cp_structure_->get_lock_count().get());
                }

                TEST_F(basic_lock_test, test_try_lock_timeout_when_locked_by_other) {
                    try_lock(std::chrono::milliseconds(100));
                }

                TEST_F(basic_lock_test, test_try_lock_long_timeout_when_locked_by_other) {
                    try_lock(std::chrono::milliseconds(LOCK_SERVICE_WAIT_TIMEOUT_TASK_UPPER_BOUND_MILLIS + 1));
                }

                TEST_F(basic_lock_test, test_reentrant_lock_fails_when_session_closed) {
                    test_when_session_closed([=] () { cp_structure_->lock().get(); });
                }

                TEST_F(basic_lock_test, test_reentrant_try_lock_fails_when_session_closed) {
                    test_when_session_closed([=] () { cp_structure_->try_lock().get(); });
                }

                TEST_F(basic_lock_test, test_reentrant_try_lock_with_timeout_fails_when_session_closed) {
                    test_when_session_closed([=] () { cp_structure_->try_lock(std::chrono::seconds(1)).get(); });
                }

                TEST_F(basic_lock_test, test_reentrant_unlock_fails_when_session_closed) {
                    test_when_session_closed([=] () { cp_structure_->unlock().get(); });

                    ASSERT_FALSE(cp_structure_->is_locked_by_current_thread().get());
                    ASSERT_FALSE(cp_structure_->is_locked().get());
                }

                TEST_F(basic_lock_test, test_unlock_fails_when_session_created) {
                    test_when_new_session_created([=] () { cp_structure_->unlock().get(); });
                }

                TEST_F(basic_lock_test, test_get_fence_fails_when_session_created) {
                    test_when_new_session_created([=] () { cp_structure_->get_fence().get(); });
                }

                TEST_F(basic_lock_test, test_failed_try_lock_does_not_acquire_session) {
                    std::async([=]() { cp_structure_->lock().get(); }).get();

                    auto group_id = cp_structure_->get_group_id();
                    auto &session_manager = spi::ClientContext(*client_).get_proxy_session_manager();
                    auto session_id = session_manager.get_session(group_id);
                    ASSERT_NE(::hazelcast::cp::internal::session::proxy_session_manager::NO_SESSION_ID, session_id);
                    ASSERT_EQ(1, session_manager.get_session_acquire_count(group_id, session_id));

                    auto fence = cp_structure_->try_lock_and_get_fence().get();
                    ASSERT_EQ(hazelcast::cp::fenced_lock::INVALID_FENCE, fence);
                    ASSERT_EQ(1, session_manager.get_session_acquire_count(group_id, session_id));
                }

                TEST_F(basic_lock_test, test_destroy) {
                    cp_structure_->try_lock().get();
                    cp_structure_->destroy().get();

                    ASSERT_THROW(cp_structure_->try_lock().get(), exception::distributed_object_destroyed);
                }

                TEST_F(basic_lock_test, test_lock_auto_release_on_client_shutdown) {
                    auto c = hazelcast::new_client(
                            std::move(get_config().set_cluster_name(
                                    client_->get_client_config().get_cluster_name()))).get();
                    auto proxy_name = get_test_name();
                    auto l = c.get_cp_subsystem().get_lock(proxy_name).get();
                    l->lock().get();

                    c.shutdown().get();

                    std::ostringstream script;
                    script << "result = instance_0.getCPSubsystem().getLock(\"" << proxy_name
                           << "\").isLocked() ? \"1\" : \"0\";";
                    Response response;

                    ASSERT_TRUE_EVENTUALLY(
                      (remote_controller_client().executeOnController(response,
                                                                      factory->get_cluster_id(),
                                                                      script.str().c_str(),
                                                                      Lang::JAVASCRIPT),
                       response.success && response.result == "0"));
                }

                class basic_sessionless_semaphore_test : public cp_test<hazelcast::cp::counting_semaphore> {
                protected:
                    std::shared_ptr<hazelcast::cp::counting_semaphore>
                    get_cp_structure(const std::string &name) override {
                        return client_->get_cp_subsystem().get_semaphore(name).get();
                    }

                    client_config get_client_config() override {
                        return std::move(cp_test::get_client_config().set_cluster_name("sessionless-semaphore"));
                    }
                };

                TEST_F(basic_sessionless_semaphore_test, create_proxy_on_metadata_cp_group) {
                    ASSERT_THROW(client_->get_cp_subsystem().get_semaphore("semaphore@METADATA").get(),
                                 exception::illegal_argument);
                }

                TEST_F(basic_sessionless_semaphore_test, test_init) {
                    ASSERT_TRUE(cp_structure_->init(7).get());
                    ASSERT_EQ(7, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_init_fails_when_already_initialized) {
                    ASSERT_TRUE(cp_structure_->init(7).get());
                    ASSERT_FALSE(cp_structure_->init(5).get());
                    ASSERT_EQ(7, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_acquire) {
                    int32_t number_of_permits = 20;
                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; ++i) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        cp_structure_->acquire().get();
                    }

                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_acquire_when_no_permits) {
                    ASSERT_TRUE(cp_structure_->init(0).get());
                    auto f = std::async([=] () {
                        cp_structure_->acquire().get();
                    });

                    ASSERT_TRUE_ALL_THE_TIME((f.wait_for(std::chrono::seconds::zero()) == std::future_status::timeout &&
                                              cp_structure_->available_permits().get() == 0), 3);

                    // let the async task finish
                    cp_structure_->destroy().get();
                }

                TEST_F(basic_sessionless_semaphore_test, test_acquire_when_no_permits_and_semaphore_destroyed) {
                    ASSERT_TRUE(cp_structure_->init(0).get());
                    auto f = std::async([=] () {
                        cp_structure_->acquire().get();
                    });

                    cp_structure_->destroy().get();

                    try {
                        f.get();
                    } catch (exception::iexception &e) {
                        std::cout << e << '\n';
                    }
                }

                TEST_F(basic_sessionless_semaphore_test, test_release) {
                    int32_t number_of_permits = 20;
                    for (int32_t i = 0; i < number_of_permits; ++i) {
                        ASSERT_EQ(i, cp_structure_->available_permits().get());
                        cp_structure_->release().get();
                    }

                    ASSERT_EQ(number_of_permits, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_allow_negative_permits) {
                    ASSERT_TRUE(cp_structure_->init(10).get());

                    cp_structure_->reduce_permits(15).get();

                    ASSERT_EQ(-5, cp_structure_->available_permits().get());

                    cp_structure_->release(10).get();

                    ASSERT_EQ(5, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_allow_negative_permits_juc_compatibility) {
                    ASSERT_TRUE(cp_structure_->init(0).get());

                    cp_structure_->reduce_permits(100).get();
                    cp_structure_->release(10).get();

                    ASSERT_EQ(-90, cp_structure_->available_permits().get());
                    ASSERT_EQ(-90, cp_structure_->drain_permits().get());

                    cp_structure_->release(10).get();

                    ASSERT_EQ(10, cp_structure_->available_permits().get());
                    ASSERT_EQ(10, cp_structure_->drain_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_increase_permits) {
                    ASSERT_TRUE(cp_structure_->init(10).get());

                    ASSERT_EQ(10, cp_structure_->available_permits().get());

                    cp_structure_->increase_permits(100).get();

                    ASSERT_EQ(110, cp_structure_->drain_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_release_when_argument_negative) {
                    ASSERT_THROW(cp_structure_->release(-5).get(), exception::illegal_argument);

                    ASSERT_EQ(0, cp_structure_->drain_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_release_when_blocked_acquire_thread) {
                    ASSERT_TRUE(cp_structure_->init(0).get());

                    auto f = std::async([=] () {
                        cp_structure_->acquire().get();
                    });

                    cp_structure_->release().get();

                    ASSERT_EQ_EVENTUALLY(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_multiple_acquire) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; i += 5) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        cp_structure_->acquire(5).get();
                    }

                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_multiple_acquire_when_negative) {
                    int32_t number_of_permits = 10;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());

                    ASSERT_THROW(cp_structure_->acquire(-5).get(), exception::illegal_argument);

                    ASSERT_EQ(number_of_permits, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_multiple_acquire_when_not_enough_permits) {
                    int32_t number_of_permits = 5;
                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());

                    auto f = std::async([=] () {
                       cp_structure_->acquire(6).get();
                       ASSERT_EQ(number_of_permits, cp_structure_->available_permits().get());
                       cp_structure_->acquire(6).get();
                       ASSERT_EQ(number_of_permits, cp_structure_->available_permits().get());
                    });

                    ASSERT_TRUE_ALL_THE_TIME((f.wait_for(std::chrono::seconds::zero()) == std::future_status::timeout &&
                                              cp_structure_->available_permits().get() == number_of_permits), 3);

                    // let the async task finish
                    cp_structure_->destroy().get();
                }

                TEST_F(basic_sessionless_semaphore_test, test_multiple_release) {
                    int32_t number_of_permits = 20;

                    for (int32_t i = 0; i < number_of_permits; i += 5) {
                        ASSERT_EQ(i, cp_structure_->available_permits().get());
                        cp_structure_->release(5).get();
                    }

                    ASSERT_EQ(number_of_permits, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_multiple_release_when_negative) {
                    ASSERT_TRUE(cp_structure_->init(0).get());

                    ASSERT_THROW(cp_structure_->release(-5).get(), exception::illegal_argument);

                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_multiple_release_when_blocked_acquire_threads) {
                    int32_t number_of_permits = 10;
                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    ASSERT_NO_THROW(cp_structure_->acquire(number_of_permits).get());

                    auto f = std::async([=] () {
                        cp_structure_->acquire().get();
                    });

                    ASSERT_NO_THROW(cp_structure_->release().get());

                    ASSERT_NO_THROW(f.get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_drain) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    ASSERT_NO_THROW(cp_structure_->acquire(5).get());
                    ASSERT_EQ(number_of_permits - 5, cp_structure_->drain_permits().get());
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_drain_when_no_permits) {
                    ASSERT_TRUE(cp_structure_->init(0).get());
                    ASSERT_EQ(0, cp_structure_->drain_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_reduce) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; i += 5) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        ASSERT_NO_THROW(cp_structure_->reduce_permits(5).get());
                    }

                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_reduce_when_negative) {
                    ASSERT_THROW(cp_structure_->reduce_permits(-5).get(), exception::illegal_argument);
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_increase_when_negative) {
                    ASSERT_THROW(cp_structure_->increase_permits(-5).get(), exception::illegal_argument);
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_try_acquire) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; ++i) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        ASSERT_TRUE(cp_structure_->try_acquire().get());
                    }

                    ASSERT_FALSE(cp_structure_->try_acquire().get());
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_try_acquire_for) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; ++i) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        ASSERT_TRUE(cp_structure_->try_acquire_for(std::chrono::seconds(120)).get());
                    }

                    ASSERT_FALSE(cp_structure_->try_acquire_for(std::chrono::seconds(0)).get());
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_try_acquire_for_when_no_permit) {
                    ASSERT_FALSE(cp_structure_->try_acquire_for(std::chrono::seconds(2)).get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_try_acquire_until) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; ++i) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        ASSERT_TRUE(cp_structure_->try_acquire_until(std::chrono::steady_clock::now() + std::chrono::seconds(120)).get());
                    }

                    ASSERT_FALSE(cp_structure_->try_acquire().get());
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_try_acquire_until_when_no_permit) {
                    ASSERT_FALSE(cp_structure_->try_acquire_until(std::chrono::steady_clock::now() + std::chrono::seconds(2)).get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_try_acquire_multiple) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; i += 5) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        ASSERT_TRUE(cp_structure_->try_acquire(5).get());
                    }

                    ASSERT_FALSE(cp_structure_->try_acquire().get());
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_try_acquire_multiple_when_argument_negative) {
                    ASSERT_TRUE(cp_structure_->init(0).get());
                    ASSERT_THROW(cp_structure_->try_acquire(-5).get(), exception::illegal_argument);
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_sessionless_semaphore_test, test_destroy) {
                    ASSERT_NO_THROW(cp_structure_->destroy().get());
                    ASSERT_THROW(cp_structure_->init(1).get(), exception::distributed_object_destroyed);
                }

                TEST_F(basic_sessionless_semaphore_test, test_acquire_on_multiple_proxies) {
                    hazelcast_client client2(new_client(
                            std::move(client_config().set_cluster_name(
                                    client_->get_client_config().get_cluster_name()))).get());
                    auto semaphore2 = client2.get_cp_subsystem().get_semaphore(cp_structure_->get_name()).get();
                    ASSERT_TRUE(cp_structure_->init(1).get());
                    ASSERT_TRUE(cp_structure_->try_acquire().get());
                    ASSERT_FALSE(semaphore2->try_acquire().get());
                }

                class basic_session_semaphore_test : public cp_test<hazelcast::cp::counting_semaphore> {
                protected:
                    std::shared_ptr<hazelcast::cp::counting_semaphore>
                    get_cp_structure(const std::string &name) override {
                        return client_->get_cp_subsystem().get_semaphore(name).get();
                    }
                };

                TEST_F(basic_session_semaphore_test, create_proxy_on_metadata_cp_group) {
                    ASSERT_THROW(client_->get_cp_subsystem().get_semaphore("semaphore@METADATA").get(),
                                 exception::illegal_argument);
                }

                TEST_F(basic_session_semaphore_test, test_init) {
                    ASSERT_TRUE(cp_structure_->init(7).get());
                    ASSERT_EQ(7, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_init_fails_when_already_initialized) {
                    ASSERT_TRUE(cp_structure_->init(7).get());
                    ASSERT_FALSE(cp_structure_->init(5).get());
                    ASSERT_EQ(7, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_acquire) {
                    int32_t number_of_permits = 20;
                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; ++i) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        cp_structure_->acquire().get();
                    }

                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_acquire_when_no_permits) {
                    ASSERT_TRUE(cp_structure_->init(0).get());
                    auto f = std::async([=] () {
                        cp_structure_->acquire().get();
                    });

                    ASSERT_TRUE_ALL_THE_TIME((f.wait_for(std::chrono::seconds::zero()) == std::future_status::timeout &&
                                              cp_structure_->available_permits().get() == 0), 3);

                    // let the async task finish
                    cp_structure_->destroy().get();
                }

                TEST_F(basic_session_semaphore_test, test_acquire_when_no_permits_and_semaphore_destroyed) {
                    ASSERT_TRUE(cp_structure_->init(0).get());
                    auto f = std::async([=] () {
                        cp_structure_->acquire().get();
                    });

                    cp_structure_->destroy().get();

                    try {
                        f.get();
                    } catch (exception::iexception &e) {
                        std::cout << e << '\n';
                    }
                }

                TEST_F(basic_session_semaphore_test, test_release) {
                    ASSERT_TRUE(cp_structure_->init(7).get());
                    ASSERT_NO_THROW(cp_structure_->acquire().get());
                    ASSERT_NO_THROW(cp_structure_->release().get());
                    ASSERT_EQ(7, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_release_when_not_acquired) {
                    ASSERT_TRUE(cp_structure_->init(7).get());
                    ASSERT_NO_THROW(cp_structure_->acquire().get());
                    ASSERT_THROW(cp_structure_->release(3).get(), exception::illegal_state);
                }

                TEST_F(basic_session_semaphore_test, test_release_when_no_session_created) {
                    ASSERT_TRUE(cp_structure_->init(7).get());
                    ASSERT_THROW(cp_structure_->release().get(), exception::illegal_state);
                }

                TEST_F(basic_session_semaphore_test, test_acquire_after_release) {
                    ASSERT_TRUE(cp_structure_->init(1).get());
                    ASSERT_NO_THROW(cp_structure_->acquire().get());

                    auto f = std::async([=] () {
                        ASSERT_NO_THROW(cp_structure_->release().get());
                    });

                    ASSERT_NO_THROW(cp_structure_->acquire().get());
                    ASSERT_NO_THROW(f.get());
                }

                TEST_F(basic_session_semaphore_test, test_multiple_acquires_after_release) {
                    ASSERT_TRUE(cp_structure_->init(2).get());
                    ASSERT_NO_THROW(cp_structure_->acquire(2).get());

                    auto latch1 = std::make_shared<boost::latch>(2);
                    auto latch2 = std::make_shared<boost::latch>(2);
                    std::vector<std::future<void>> futures;
                    for (int i = 0; i < 2; ++i) {
                        futures.emplace_back(std::async([=] () {
                            latch1->count_down();
                            ASSERT_NO_THROW(cp_structure_->acquire().get());
                            latch2->count_down();
                        }));
                    }

                    ASSERT_OPEN_EVENTUALLY(*latch1);
                    // This sleep exists at Java test but sleep is not good and reliable in tests and vital here.
                    //std::this_thread::sleep_for(std::chrono::seconds(2));

                    ASSERT_NO_THROW(cp_structure_->release(2).get());

                    ASSERT_OPEN_EVENTUALLY(*latch2);
                }

                TEST_F(basic_session_semaphore_test, test_allow_negative_permits) {
                    ASSERT_TRUE(cp_structure_->init(10).get());

                    cp_structure_->reduce_permits(15).get();

                    ASSERT_EQ(-5, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_allow_negative_permits_juc_compatibility) {
                    ASSERT_TRUE(cp_structure_->init(0).get());

                    cp_structure_->reduce_permits(100).get();
                    ASSERT_EQ(-100, cp_structure_->available_permits().get());
                    ASSERT_EQ(-100, cp_structure_->drain_permits().get());

                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_increase_permits) {
                    ASSERT_TRUE(cp_structure_->init(10).get());

                    ASSERT_EQ(10, cp_structure_->available_permits().get());

                    cp_structure_->increase_permits(100).get();

                    ASSERT_EQ(110, cp_structure_->drain_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_release_when_argument_negative) {
                    ASSERT_THROW(cp_structure_->release(-5).get(), exception::illegal_argument);

                    ASSERT_EQ(0, cp_structure_->drain_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_release_when_blocked_acquire_thread) {
                    int32_t permits = 10;
                    ASSERT_TRUE(cp_structure_->init(permits).get());
                    ASSERT_NO_THROW(cp_structure_->acquire(permits).get());

                    auto f = std::async([=] () {
                        cp_structure_->acquire().get();
                    });

                    cp_structure_->release().get();

                    ASSERT_NO_THROW(f.get());
                }

                TEST_F(basic_session_semaphore_test, test_multiple_acquire) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; i += 5) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        cp_structure_->acquire(5).get();
                    }

                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_multiple_acquire_when_negative) {
                    int32_t number_of_permits = 10;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());

                    ASSERT_THROW(cp_structure_->acquire(-5).get(), exception::illegal_argument);

                    ASSERT_EQ(number_of_permits, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_multiple_acquire_when_not_enough_permits) {
                    int32_t number_of_permits = 5;
                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());

                    auto f = std::async([=] () {
                        cp_structure_->acquire(6).get();
                        ASSERT_EQ(number_of_permits, cp_structure_->available_permits().get());
                        cp_structure_->acquire(6).get();
                        ASSERT_EQ(number_of_permits, cp_structure_->available_permits().get());
                    });

                    ASSERT_TRUE_ALL_THE_TIME((f.wait_for(std::chrono::seconds::zero()) == std::future_status::timeout &&
                                              cp_structure_->available_permits().get() == number_of_permits), 3);

                    // let the async task finish
                    cp_structure_->destroy().get();
                }

                TEST_F(basic_session_semaphore_test, test_multiple_release) {
                    int32_t number_of_permits = 20;
                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    ASSERT_NO_THROW(cp_structure_->acquire(number_of_permits).get());

                    for (int32_t i = 0; i < number_of_permits; i += 5) {
                        ASSERT_EQ(i, cp_structure_->available_permits().get());
                        cp_structure_->release(5).get();
                    }

                    ASSERT_EQ(number_of_permits, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_multiple_release_when_negative) {
                    ASSERT_TRUE(cp_structure_->init(0).get());

                    ASSERT_THROW(cp_structure_->release(-5).get(), exception::illegal_argument);

                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_multiple_release_when_blocked_acquire_threads) {
                    int32_t number_of_permits = 10;
                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    ASSERT_NO_THROW(cp_structure_->acquire(number_of_permits).get());

                    auto f = std::async([=] () {
                        cp_structure_->acquire().get();
                    });

                    ASSERT_NO_THROW(cp_structure_->release().get());

                    ASSERT_NO_THROW(f.get());
                }

                TEST_F(basic_session_semaphore_test, test_drain) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    ASSERT_NO_THROW(cp_structure_->acquire(5).get());
                    ASSERT_EQ(number_of_permits - 5, cp_structure_->drain_permits().get());
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_drain_when_no_permits) {
                    ASSERT_TRUE(cp_structure_->init(0).get());
                    ASSERT_EQ(0, cp_structure_->drain_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_reduce) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; i += 5) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        ASSERT_NO_THROW(cp_structure_->reduce_permits(5).get());
                    }

                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_reduce_when_negative) {
                    ASSERT_THROW(cp_structure_->reduce_permits(-5).get(), exception::illegal_argument);
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_increase_when_negative) {
                    ASSERT_THROW(cp_structure_->increase_permits(-5).get(), exception::illegal_argument);
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_try_acquire) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; ++i) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        ASSERT_TRUE(cp_structure_->try_acquire().get());
                    }

                    ASSERT_FALSE(cp_structure_->try_acquire().get());
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_try_acquire_for) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; ++i) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        ASSERT_TRUE(cp_structure_->try_acquire_for(std::chrono::seconds(120)).get());
                    }

                    ASSERT_FALSE(cp_structure_->try_acquire_for(std::chrono::seconds(0)).get());
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_try_acquire_for_when_no_permit) {
                    ASSERT_FALSE(cp_structure_->try_acquire_for(std::chrono::seconds(2)).get());
                }

                TEST_F(basic_session_semaphore_test, test_try_acquire_until) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; ++i) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        ASSERT_TRUE(cp_structure_->try_acquire_until(std::chrono::steady_clock::now() + std::chrono::seconds(120)).get());
                    }

                    ASSERT_FALSE(cp_structure_->try_acquire().get());
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_try_acquire_until_when_no_permit) {
                    ASSERT_FALSE(cp_structure_->try_acquire_until(std::chrono::steady_clock::now() + std::chrono::seconds(2)).get());
                }

                TEST_F(basic_session_semaphore_test, test_try_acquire_multiple) {
                    int32_t number_of_permits = 20;

                    ASSERT_TRUE(cp_structure_->init(number_of_permits).get());
                    for (int32_t i = 0; i < number_of_permits; i += 5) {
                        ASSERT_EQ(number_of_permits - i, cp_structure_->available_permits().get());
                        ASSERT_TRUE(cp_structure_->try_acquire(5).get());
                    }

                    ASSERT_FALSE(cp_structure_->try_acquire().get());
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_try_acquire_multiple_when_argument_negative) {
                    ASSERT_TRUE(cp_structure_->init(0).get());
                    ASSERT_THROW(cp_structure_->try_acquire(-5).get(), exception::illegal_argument);
                    ASSERT_EQ(0, cp_structure_->available_permits().get());
                }

                TEST_F(basic_session_semaphore_test, test_destroy) {
                    ASSERT_NO_THROW(cp_structure_->destroy().get());
                    ASSERT_THROW(cp_structure_->init(1).get(), exception::distributed_object_destroyed);
                }

                TEST_F(basic_session_semaphore_test, test_acquire_on_multiple_proxies) {
                    hazelcast_client client2(new_client(
                            std::move(client_config().set_cluster_name(
                                    client_->get_client_config().get_cluster_name()))).get());
                    auto semaphore2 = client2.get_cp_subsystem().get_semaphore(cp_structure_->get_name()).get();
                    ASSERT_TRUE(cp_structure_->init(1).get());
                    ASSERT_TRUE(cp_structure_->try_acquire().get());
                    ASSERT_FALSE(semaphore2->try_acquire().get());
                }

            }
        }
    }
}
