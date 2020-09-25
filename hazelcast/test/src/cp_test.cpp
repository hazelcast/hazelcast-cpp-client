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
#include "ClientTestSupport.h"
#include "HazelcastServer.h"
#include "IdentifiedSerializables.h"

#include <hazelcast/client/HazelcastClient.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace cp {
                class basic_atomic_long_test : public ClientTestSupport {
                protected:
                    virtual void SetUp() {
                        auto test_name = getTestName();
                        atomic_long_ = client->get_cp_subsystem().get_atomic_long(
                                test_name + "@" + test_name + "_group");
                    }

                    static void SetUpTestCase() {
                        server1 = new HazelcastServer(*g_srvFactory);
                        server2 = new HazelcastServer(*g_srvFactory);
                        server3 = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient(getConfig());
                    }

                    static void TearDownTestCase() {
                        delete client;
                        delete server1;
                        delete server2;
                        delete server3;
                    }

                    static HazelcastServer *server1;
                    static HazelcastServer *server2;
                    static HazelcastServer *server3;
                    static HazelcastClient *client;

                    std::shared_ptr<atomic_long> atomic_long_;
                };

                HazelcastServer *basic_atomic_long_test::server1 = nullptr;
                HazelcastServer *basic_atomic_long_test::server2 = nullptr;
                HazelcastServer *basic_atomic_long_test::server3 = nullptr;
                HazelcastClient *basic_atomic_long_test::client = nullptr;

                TEST_F(basic_atomic_long_test, create_proxy_on_metadata_cp_group) {
                    ASSERT_THROW(client->get_cp_subsystem().get_atomic_long("long@METADATA"),
                                 exception::IllegalArgumentException);
                }

                TEST_F(basic_atomic_long_test, test_set) {
                    atomic_long_->set(271).get();
                    ASSERT_EQ(271, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_get) {
                    ASSERT_EQ(0, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_decrement_and_get) {
                    ASSERT_EQ(-1, atomic_long_->decrement_and_get().get());
                    ASSERT_EQ(-2, atomic_long_->decrement_and_get().get());
                }

                TEST_F(basic_atomic_long_test, test_increment_and_get) {
                    ASSERT_EQ(1, atomic_long_->increment_and_get().get());
                    ASSERT_EQ(2, atomic_long_->increment_and_get().get());
                }

                TEST_F(basic_atomic_long_test, test_get_and_set) {
                    ASSERT_EQ(0, atomic_long_->get_and_set(271).get());
                    ASSERT_EQ(271, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_add_and_get) {
                    ASSERT_EQ(271, atomic_long_->add_and_get(271).get());
                    ASSERT_EQ(271, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_get_and_add) {
                    ASSERT_EQ(0, atomic_long_->get_and_add(271).get());
                    ASSERT_EQ(271, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_compare_and_set_when_success) {
                    ASSERT_TRUE(atomic_long_->compare_and_set(0, 271).get());
                    ASSERT_EQ(271, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_compare_and_set_when_not_success) {
                    ASSERT_FALSE(atomic_long_->compare_and_set(172, 0).get());
                    ASSERT_EQ(0, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_alter) {
                    atomic_long_->set(2).get();
                    ASSERT_NO_THROW(atomic_long_->alter(multiplication{5}).get());
                    ASSERT_EQ(10, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_alter_and_get) {
                    atomic_long_->set(2).get();
                    auto result = atomic_long_->alter_and_get(multiplication{5}).get();
                    ASSERT_EQ(10, result);
                    ASSERT_EQ(10, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_get_and_alter) {
                    atomic_long_->set(2).get();
                    auto result = atomic_long_->get_and_alter(multiplication{5}).get();
                    ASSERT_EQ(2, result);
                    ASSERT_EQ(10, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_apply) {
                    atomic_long_->set(2).get();
                    auto result = atomic_long_->apply<multiplication, int64_t>({5}).get();
                    ASSERT_TRUE(result);
                    ASSERT_EQ(10, *result);
                    ASSERT_EQ(2, atomic_long_->get().get());
                }

                TEST_F(basic_atomic_long_test, test_use_after_destroy) {
                    atomic_long_->destroy().get();
                    ASSERT_THROW(atomic_long_->increment_and_get().get(),
                                 exception::DistributedObjectDestroyedException);
                }

                TEST_F(basic_atomic_long_test, test_create_after_destroy) {
                    auto name = atomic_long_->getName();
                    atomic_long_->destroy().get();

                    atomic_long_ = client->get_cp_subsystem().get_atomic_long(name);
                    ASSERT_THROW(atomic_long_->increment_and_get().get(),
                                 exception::DistributedObjectDestroyedException);
                }

                TEST_F(basic_atomic_long_test, test_multiple_destroy) {
                    ASSERT_NO_THROW(atomic_long_->destroy().get());
                    ASSERT_NO_THROW(atomic_long_->destroy().get());
                }

                class basic_atomic_ref_test : public ClientTestSupport {
                protected:
                    virtual void SetUp() {
                        auto test_name = getTestName();
                        atomic_ref_ = client->get_cp_subsystem().get_atomic_reference(
                                test_name + "@" + test_name + "_group");
                    }

                    static void SetUpTestCase() {
                        server1 = new HazelcastServer(*g_srvFactory);
                        server2 = new HazelcastServer(*g_srvFactory);
                        server3 = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient(getConfig());
                    }

                    static void TearDownTestCase() {
                        delete client;
                        delete server1;
                        delete server2;
                        delete server3;
                    }

                    static HazelcastServer *server1;
                    static HazelcastServer *server2;
                    static HazelcastServer *server3;
                    static HazelcastClient *client;

                    std::shared_ptr<atomic_reference> atomic_ref_;
                };

                HazelcastServer *basic_atomic_ref_test::server1 = nullptr;
                HazelcastServer *basic_atomic_ref_test::server2 = nullptr;
                HazelcastServer *basic_atomic_ref_test::server3 = nullptr;
                HazelcastClient *basic_atomic_ref_test::client = nullptr;

                TEST_F(basic_atomic_ref_test, create_proxy_on_metadata_cp_group) {
                    ASSERT_THROW(client->get_cp_subsystem().get_atomic_reference("ref@METADATA"),
                                 exception::IllegalArgumentException);
                }

                TEST_F(basic_atomic_ref_test, test_set) {
                    atomic_ref_->set(std::string("str1")).get();
                    ASSERT_EQ("str1", *atomic_ref_->get<std::string>().get());
                    ASSERT_EQ("str1", *atomic_ref_->get_and_set<std::string>("str2").get());
                    ASSERT_EQ("str2", *atomic_ref_->get<std::string>().get());
                }

                TEST_F(basic_atomic_ref_test, test_compare_and_set) {
                    std::string str1("str1");
                    std::string str2("str2");
                    ASSERT_TRUE(atomic_ref_->compare_and_set(static_cast<std::string *>(nullptr), &str1).get());
                    ASSERT_EQ(str1, *atomic_ref_->get<std::string>().get());
                    ASSERT_FALSE(atomic_ref_->compare_and_set(static_cast<std::string *>(nullptr), &str1).get());
                    ASSERT_TRUE(atomic_ref_->compare_and_set(str1, str2).get());
                    ASSERT_EQ(str2, *atomic_ref_->get<std::string>().get());
                    ASSERT_FALSE(atomic_ref_->compare_and_set(str1, str2).get());
                    ASSERT_TRUE(atomic_ref_->compare_and_set(&str2, static_cast<std::string *>(nullptr)).get());
                    ASSERT_FALSE(atomic_ref_->get<std::string>().get());
                    ASSERT_FALSE(atomic_ref_->compare_and_set(&str2, static_cast<std::string *>(nullptr)).get());
                }

                TEST_F(basic_atomic_ref_test, test_is_null) {
                    ASSERT_TRUE(atomic_ref_->is_null().get());
                    atomic_ref_->set(std::string("str1")).get();
                    ASSERT_FALSE(atomic_ref_->is_null().get());
                }

                TEST_F(basic_atomic_ref_test, test_clear) {
                    atomic_ref_->set(std::string("str1")).get();
                    atomic_ref_->clear().get();
                    ASSERT_TRUE(atomic_ref_->is_null().get());
                }

                TEST_F(basic_atomic_ref_test, test_contains) {
                    std::string str1("str1");
                    ASSERT_TRUE(atomic_ref_->contains(static_cast<std::string *>(nullptr)).get());
                    ASSERT_FALSE(atomic_ref_->contains(str1).get());

                    atomic_ref_->set(str1).get();

                    ASSERT_FALSE(atomic_ref_->contains(static_cast<std::string *>(nullptr)).get());
                    ASSERT_TRUE(atomic_ref_->contains(str1).get());
                }

                TEST_F(basic_atomic_ref_test, test_alter) {
                    atomic_ref_->set(std::string("str1")).get();

                    atomic_ref_->alter(test::append_string{"str2"}).get();

                    ASSERT_EQ("str1str2", *atomic_ref_->get<std::string>().get());

                    auto val = atomic_ref_->alter_and_get<std::string>(test::append_string{"str3"}).get();
                    ASSERT_TRUE(val);
                    ASSERT_EQ("str1str2str3", *val);

                    val = atomic_ref_->get_and_alter<std::string>(test::append_string{"str4"}).get();
                    ASSERT_TRUE(val);
                    ASSERT_EQ("str1str2str3", *val);

                    ASSERT_EQ("str1str2str3str4", *atomic_ref_->get<std::string>().get());
                }

                TEST_F(basic_atomic_ref_test, test_apply) {
                    atomic_ref_->set(std::string("str1")).get();

                    auto val = atomic_ref_->apply<std::string>(test::append_string{"str2"}).get();
                    ASSERT_TRUE(val);
                    ASSERT_EQ("str1str2", *val);
                    ASSERT_EQ("str1", *atomic_ref_->get<std::string>().get());
                }

                TEST_F(basic_atomic_ref_test, test_use_after_destroy) {
                    atomic_ref_->destroy().get();
                    ASSERT_THROW((atomic_ref_->set(std::string("str1")).get()),
                                 exception::DistributedObjectDestroyedException);
                }

                TEST_F(basic_atomic_ref_test, test_create_after_destroy) {
                    auto name = atomic_ref_->getName();
                    atomic_ref_->destroy().get();

                    atomic_ref_ = client->get_cp_subsystem().get_atomic_reference(name);
                    ASSERT_THROW(atomic_ref_->set(std::string("str1")).get(),
                                 exception::DistributedObjectDestroyedException);
                }

                TEST_F(basic_atomic_ref_test, test_multiple_destroy) {
                    ASSERT_NO_THROW(atomic_ref_->destroy().get());
                    ASSERT_NO_THROW(atomic_ref_->destroy().get());
                }

                class basic_latch_test : public ClientTestSupport {
                protected:
                    virtual void SetUp() {
                        auto test_name = getTestName();
                        latch_ = client->get_cp_subsystem().get_latch(test_name + "@" + test_name + "_group");
                    }

                    static void SetUpTestCase() {
                        server1 = new HazelcastServer(*g_srvFactory);
                        server2 = new HazelcastServer(*g_srvFactory);
                        server3 = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient(getConfig());
                    }

                    static void TearDownTestCase() {
                        delete client;
                        delete server1;
                        delete server2;
                        delete server3;
                    }

                    static HazelcastServer *server1;
                    static HazelcastServer *server2;
                    static HazelcastServer *server3;
                    static HazelcastClient *client;

                    std::shared_ptr<latch> latch_;
                };

                HazelcastServer *basic_latch_test::server1 = nullptr;
                HazelcastServer *basic_latch_test::server2 = nullptr;
                HazelcastServer *basic_latch_test::server3 = nullptr;
                HazelcastClient *basic_latch_test::client = nullptr;

                TEST_F(basic_latch_test, create_proxy_on_metadata_cp_group) {
                    ASSERT_THROW(client->get_cp_subsystem().get_latch("ref@METADATA"),
                                 exception::IllegalArgumentException);
                }

                TEST_F(basic_latch_test, test_try_set_count_when_argument_negative) {
                    ASSERT_THROW(latch_->try_set_count(-20).get(), exception::IllegalArgumentException);
                }

                TEST_F(basic_latch_test, test_try_set_count_when_count_is_not_zero) {
                    ASSERT_TRUE(latch_->try_set_count(10).get());

                    ASSERT_FALSE(latch_->try_set_count(20).get());
                    ASSERT_FALSE(latch_->try_set_count(0).get());
                    ASSERT_EQ(10, latch_->get_count().get());
                }

                TEST_F(basic_latch_test, test_count_down) {
                    latch_->try_set_count(20).get();

                    for (int i = 19; i >= 0; i--) {
                        latch_->count_down().get();
                        ASSERT_EQ(i, latch_->get_count().get());
                    }

                    latch_->count_down().get();
                    ASSERT_EQ(0, latch_->get_count().get());
                }

                TEST_F(basic_latch_test, test_get_count) {
                    latch_->try_set_count(20).get();

                    ASSERT_EQ(20, latch_->get_count().get());
                }

                TEST_F(basic_latch_test, test_wait_for) {
                    latch_->try_set_count(1).get();
                    std::thread bg([=]() {
                        latch_->count_down().get();
                    });

                    ASSERT_OPEN_EVENTUALLY_ASYNC(latch_);
                    bg.join();
                }

                TEST_F(basic_latch_test, test_wait_until) {
                    latch_->try_set_count(1).get();
                    std::thread bg([=]() {
                        latch_->count_down().get();
                    });

                    ASSERT_EQ(std::cv_status::no_timeout,
                              latch_->wait_until(std::chrono::steady_clock::now() + std::chrono::seconds(120)).get());
                    bg.join();
                }

                TEST_F(basic_latch_test, test_wait) {
                    latch_->try_set_count(1).get();
                    std::thread bg([=]() {
                        latch_->count_down().get();
                    });

                    latch_->wait().get();
                    bg.join();
                }

                TEST_F(basic_latch_test, test_wait_for_when_timeout) {
                    latch_->try_set_count(1).get();
                    auto start = std::chrono::steady_clock::now();
                    ASSERT_EQ(std::cv_status::timeout, latch_->wait_for(std::chrono::milliseconds(100)).get());
                    auto elapsed = std::chrono::steady_clock::now() - start;
                    ASSERT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(), 100);
                    ASSERT_EQ(1, latch_->get_count().get());
                }

                TEST_F(basic_latch_test, test_count_down_after_destroy) {
                    latch_->destroy().get();

                    ASSERT_THROW(latch_->count_down().get(), exception::DistributedObjectDestroyedException);
                }

                TEST_F(basic_latch_test, test_multiple_destroy) {
                    ASSERT_NO_THROW(latch_->destroy().get());
                    ASSERT_NO_THROW(latch_->destroy().get());
                }

            }
        }
    }
}
