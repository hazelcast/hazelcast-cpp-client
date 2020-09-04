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

#include <string>
#include <boost/thread/future.hpp>

#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/proxy/ProxyImpl.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
        }
    }
    namespace cp {
        using namespace client::serialization::pimpl;

        class raft_proxy_factory;

        struct raft_group_id {
            std::string name;
            int64_t seed;
            int64_t group_id;
        };

        class cp_proxy : public client::proxy::ProxyImpl {
        public:
            cp_proxy(const std::string &serviceName, const std::string &proxyName, client::spi::ClientContext *context,
                     const raft_group_id &groupId, const std::string &objectName);

            const raft_group_id &getGroupId() const;

        protected:
            raft_group_id group_id_;
            std::string object_name_;

            void onDestroy() override;
        };

        /**
         * Client-side Raft-based proxy implementation of atomic long.
         */
        class HAZELCAST_API atomic_long : public cp_proxy {
        public:
            atomic_long(const std::string &name, client::spi::ClientContext &context,
                        const raft_group_id &groupId, const std::string &objectName);

            /**
             * Atomically adds the given value to the current value.
             *
             * @param delta the value to add to the current value
             * @return the updated value, the given value added to the current value
             */
            boost::future<int64_t> add_and_get(int64_t delta);

            /**
             * Atomically sets the value to the given updated value
             * only if the current value \code==\endcode the expected value.
             *
             * @param expect the expected value
             * @param update the new value
             * @return \code true\endcode if successful; or \code false\endcode if the actual value
             * was not equal to the expected value.
             */
            boost::future<bool> compare_and_set(int64_t expect, int64_t update);

            /**
             * Atomically decrements the current value by one.
             *
             * @return the updated value, the current value decremented by one
             */
            boost::future<int64_t> decrement_and_get();

            /**
             * Gets the current value.
             *
             * @return the current value
             */
            boost::future<int64_t> get();

            /**
             * Atomically adds the given value to the current value.
             *
             * @param delta the value to add to the current value
             * @return the old value before the add
             */
            boost::future<int64_t> get_and_add(int64_t delta);

            /**
             * Atomically sets the given value and returns the old value.
             *
             * @param newValue the new value
             * @return the old value
             */
            boost::future<int64_t> get_and_set(int64_t newValue);

            /**
             * Atomically increments the current value by one.
             *
             * @return the updated value, the current value incremented by one
             */
            boost::future<int64_t> increment_and_get();

            /**
             * Atomically increments the current value by one.
             *
             * @return the old value
             */
            boost::future<int64_t> get_and_increment();

            /**
             * Atomically sets the given value.
             *
             * @param newValue the new value
             */
            boost::future<void> set(int64_t newValue);

            /**
             * Alters the currently stored value by applying a function on it.
             *
             * @param function the function applied to the currently stored value
             */
            template<typename F>
            boost::future<void> alter(const F &function) {
                return toVoidFuture(alter_and_get(function));
            }

            /**
             * Alters the currently stored value by applying a function on it and
             * gets the result.
             *
             * @param function the function applied to the currently stored value
             * @return the new value
             */
            template<typename F>
            boost::future<int64_t> alter_and_get(const F &function) {
                auto f = toData(function);
                return alter_data(f, alter_result_type::NEW_VALUE);
            }

            /**
             * Alters the currently stored value by applying a function on it on and
             * gets the old value.
             *
             * @param function the function applied to the currently stored value
             * @return the old value
             * @since 3.2
             */
            template<typename F>
            boost::future<int64_t> get_and_alter(const F &function) {
                auto f = toData(function);
                return alter_data(f, alter_result_type::OLD_VALUE);
            }

            /**
             * Applies a function on the value, the actual stored value will not change.
             *
             * @param function the function applied to the value, the value is not changed
             * @param <R> the result type of the function
             * @return the result of the function application
             */
            template<typename F, typename R>
            boost::future<boost::optional<R>> apply(const F &function) {
                auto f = toData(function);
                return toObject<R>(apply_data(f));
            }

        private:

            static constexpr const char *SERVICE_NAME = "hz:raft:atomicLongService";
            /**
             * Denotes how the return value will be chosen.
             */
            enum alter_result_type {
                /**
                 * The value before the function is applied
                 */
                OLD_VALUE,
                /**
                 * The value after the function is applied
                 */
                NEW_VALUE
            };

            boost::future<int64_t>
            alter_data(Data &function_data, alter_result_type result_type);

            boost::future<boost::optional<Data>> apply_data(Data &function_data);
        };

        /**
         * Client-side Raft-based proxy implementation of atomic reference.
         */
        class HAZELCAST_API atomic_reference : public cp_proxy {
        public:
            atomic_reference(const std::string &name, client::spi::ClientContext &context,
                             const raft_group_id &groupId, const std::string &objectName);

            template<typename T>
            boost::future<boost::optional<typename std::remove_pointer<T>::type>> get() {
                return toObject<typename std::remove_pointer<T>::type>(get_data());
            }

            template<typename T>
            boost::future<boost::optional<typename std::remove_pointer<T>::type>> set(T new_value) {
                return toObject<typename std::remove_pointer<T>::type>(
                        set_data(toData<typename std::remove_pointer<T>::type>(new_value)));
            }

            template<typename T>
            boost::future<boost::optional<typename std::remove_pointer<T>::type>> get_and_set(T new_value) {
                return toObject<typename std::remove_pointer<T>::type>(
                        get_and_set_data(toData<typename std::remove_pointer<T>::type>(new_value)));
            }

            template<typename T>
            boost::future<bool> compare_and_set(T expect, T update) {
                return compare_and_set_data(toData<typename std::remove_pointer<T>::type>(expect),
                                            toData<typename std::remove_pointer<T>::type>(update));
            }

            boost::future<bool> is_null();

            boost::future<void> clear();

            template<typename T>
            boost::future<bool> contains(T value) {
                return contains_data(toData<typename std::remove_pointer<T>::type>(value));
            }

            template<typename F>
            boost::future<void> alter(const F &function) {
                return alter_data(toData(function));
            }

            template<typename T, typename F>
            boost::future<boost::optional<typename std::remove_pointer<T>::type>> alter_and_get(const F &function) {
                return toObject<typename std::remove_pointer<T>::type>(alter_and_get_data(toData(function)));
            }

            template<typename T, typename F>
            boost::future<boost::optional<typename std::remove_pointer<T>::type>> get_and_alter(const F &function) {
                return toObject<typename std::remove_pointer<T>::type>(get_and_alter_data(toData(function)));
            }

            template<typename R, typename F>
            boost::future<boost::optional<R>> apply(const F &function) {
                return toObject<R>(apply_data(toData(function)));
            }

        private:
            static constexpr const char *SERVICE_NAME = "hz:raft:atomicRefService";

            enum struct return_value_type {
                NO_VALUE,
                OLD,
                NEW
            };

            boost::future<boost::optional<Data>> get_data();

            boost::future<boost::optional<Data>> set_data(const Data &new_value_data);

            boost::future<boost::optional<Data>> get_and_set_data(const Data &new_value_data);

            boost::future<bool> compare_and_set_data(const Data &expect_data, const Data &update_data);

            boost::future<bool> contains_data(const Data &value_data);

            boost::future<void> alter_data(const Data &function_data);

            boost::future<boost::optional<Data>> alter_and_get_data(const Data &function_data);

            boost::future<boost::optional<Data>> get_and_alter_data(const Data &function_data);

            boost::future<boost::optional<Data>> apply_data(const Data &function_data);

            boost::future<boost::optional<Data>> invoke_apply(const Data function_data, return_value_type return_type,
                                                              bool alter);
        };

        class HAZELCAST_API latch : public cp_proxy {
        public:
            latch(const std::string &name, client::spi::ClientContext &context, const raft_group_id &groupId,
                  const std::string &objectName);

            boost::future<bool> try_set_count(int32_t count);

            boost::future<int32_t> get_count();

            boost::future<void> count_down();

            boost::future<bool> try_wait();

            boost::future<void> wait();

            template<typename Rep, typename Period>
            boost::future<std::cv_status> wait_for(const std::chrono::duration<Rep, Period> &rel_time) {
                using namespace std::chrono;
                return wait_for(duration_cast<milliseconds>(rel_time).count());
            }

            template<typename Clock, typename Duration>
            boost::future<std::cv_status> wait_until(const std::chrono::time_point<Clock, Duration> &timeout_time ) {
                using namespace std::chrono;
                return wait_for(duration_cast<milliseconds>(timeout_time - Clock::now()).count());
            }

        private:
            static constexpr const char *SERVICE_NAME = "hz:raft:countDownLatchService";

            boost::future<int32_t> get_round();

            void count_down(int round, boost::uuids::uuid invocation_uid);

            boost::future<std::cv_status> wait_for(int64_t milliseconds);
        };

        class HAZELCAST_API fenced_lock {

        };

        class HAZELCAST_API semaphore {

        };

        /**
         * Client-side Raft-based proxy implementation of atomic reference
         *
         */
        class raft_proxy_factory {
        public:
            raft_proxy_factory(client::spi::ClientContext &context);

            static std::string without_default_group_name(const std::string &n);

            static std::string object_name_for_proxy(const std::string &name);

            template<typename T>
            std::shared_ptr<T> create_proxy(const std::string &name) {
                auto proxy_name = without_default_group_name(name);
                auto object_name = object_name_for_proxy(proxy_name);
                auto group_id = get_group_id(proxy_name, object_name);
                return create<T>(std::move(group_id), proxy_name, object_name);
            }

        private:
            client::spi::ClientContext &context_;
            util::SynchronizedMap<std::string, fenced_lock> lock_proxies_;

            /**
             * Name of the DEFAULT CP group that is used when no group name is
             * specified while creating a CP data structure proxy.
             */
            static constexpr const char *DEFAULT_GROUP_NAME = "default";

            template<typename T, typename = typename std::enable_if<std::is_same<atomic_long, T>::value ||
                                                                    std::is_same<atomic_reference, T>::value ||
                                                                    std::is_same<latch, T>::value>::type>
            std::shared_ptr<T>
            create(raft_group_id &&group_id, const std::string &proxy_name, const std::string &object_name) {
                return std::make_shared<T>(proxy_name, context_, std::move(group_id), object_name);
            }

            std::shared_ptr<fenced_lock>
            create_fenced_lock(raft_group_id &&group_id, const std::string &proxy_name, const std::string &object_name);

            template<typename T, typename = typename std::enable_if<std::is_same<fenced_lock, T>::value>::type>
            std::shared_ptr<fenced_lock>
            create(raft_group_id &&group_id, const std::string &proxy_name, const std::string &object_name) {
                return create_fenced_lock(std::move(group_id), proxy_name, object_name);
            }

            std::shared_ptr<semaphore>
            create_semaphore(raft_group_id &&group_id, const std::string &proxy_name, const std::string &object_name);

            template<typename T, typename = typename std::enable_if<std::is_same<semaphore, T>::value>::type>
            std::shared_ptr<semaphore>
            create(raft_group_id &&group_id, const std::string &proxy_name, const std::string &object_name) {
                return create_semaphore(std::move(group_id), proxy_name, object_name);
            }

            raft_group_id get_group_id(const std::string &proxy_name, const std::string &object_name);
        };

        /**
         * CP Subsystem is a component of Hazelcast that builds a strongly consistent
         * layer for a set of distributed data structures. Its APIs can be used for
         * implementing distributed coordination use cases, such as leader election,
         * distributed locking, synchronization, and metadata management.
         * It is accessed via HazelcastClient::get_cp_subsystem. Its data
         * structures are CP with respect to the CAP principle, i.e., they always
         * maintain linearizability and prefer consistency over availability during
         * network partitions. Besides network partitions, CP Subsystem withstands
         * server and client failures.
         */
        class HAZELCAST_API cp_subsystem {
        public:
            /**
             * Returns a proxy for an atomic_long instance created on CP
             * Subsystem. Hazelcast's atomic_long is a distributed version of
             * <tt>java.util.concurrent.atomic.AtomicLong</tt>. If no group name is
             * given within the "name" parameter, then the atomic_long instance
             * will be created on the DEFAULT CP group. If a group name is given, like
             * cp_subsystem::get_atomic_long("myint64_t@group1"), the given group will be
             * initialized first, if not initialized already, and then the
             * atomic_long instance will be created on this group. Returned
             * atomic_long instance offers linearizability and behaves as a CP
             * register. When a network partition occurs, proxies that exist on the
             * minority side of its CP group lose availability.
             * <p>
             * <strong>Each call of this method performs a commit to the METADATA CP
             * group. Hence, callers should cache the returned proxy.</strong>
             *
             * @param name name of the atomic_long proxy
             * @return atomic_long proxy for the given name
             * @throws HazelcastException if CP Subsystem is not enabled
             */
            std::shared_ptr<atomic_long> get_atomic_long(const std::string &name);

            /**
             * Returns a proxy for an atomic_reference instance created on
             * CP Subsystem. Hazelcast's atomic_reference is a distributed
             * version of <tt>java.util.concurrent.atomic.AtomicLong</tt>. If no group
             * name is given within the "name" parameter, then
             * the atomic_reference instance will be created on the DEFAULT CP
             * group. If a group name is given, like
             * cp_subsystem::get_atomic_reference("myRef@group1"), the given group will be
             * initialized first, if not initialized already, and then the
             * atomic_reference instance will be created on this group.
             * Returned atomic_reference instance offers linearizability and
             * behaves as a CP register. When a network partition occurs, proxies that
             * exist on the minority side of its CP group lose availability.
             * <p>
             * <strong>Each call of this method performs a commit to the METADATA CP
             * group. Hence, callers should cache the returned proxy.</strong>
             *
             * @param name name of the atomic_reference proxy
             * @param <E>  the type of object referred to by the reference
             * @return atomic_reference proxy for the given name
             * @throws HazelcastException if CP Subsystem is not enabled
             */

            std::shared_ptr<atomic_reference> get_atomic_reference(const std::string &name);

            /**
             * Returns a proxy for an count_down_latch instance created on
             * CP Subsystem. Hazelcast's count_down_latch is a distributed
             * version of <tt>java.util.concurrent.CountDownLatch</tt>. If no group
             * name is given within the "name" parameter, then
             * the count_down_latch instance will be created on the DEFAULT CP
             * group. If a group name is given, like
             * cp_subsystem::get_atomic_reference("myLatch@group1"), the given group will be
             * initialized first, if not initialized already, and then the
             * count_down_latch instance will be created on this group. Returned
             * count_down_latch instance offers linearizability. When a network
             * partition occurs, proxies that exist on the minority side of its CP
             * group lose availability.
             * <p>
             * <strong>Each call of this method performs a commit to the METADATA CP
             * group. Hence, callers should cache the returned proxy.</strong>
             *
             * @param name name of the count_down_latch proxy
             * @return count_down_latch proxy for the given name
             * @throws HazelcastException if CP Subsystem is not enabled
             */
            std::shared_ptr<latch> get_latch(const std::string &name);

            /**
             * Returns a proxy for an fenced_lock instance created on CP
             * Subsystem. Hazelcast's fenced_lock is a distributed version of
             * <tt>java.util.concurrent.locks.Lock</tt>. If no group name is given
             * within the "name" parameter, then the fenced_lock instance will
             * be created on the DEFAULT CP group. If a group name is given, like
             * cp_subsystem::get_atomic_reference("myLock@group1"), the given group will be initialized
             * first, if not initialized already, and then the fenced_lock
             * instance will be created on this group. Returned fenced_lock
             * instance offers linearizability. When a network partition occurs,
             * proxies that exist on the minority side of its CP group lose
             * availability.
             * <p>
             * <strong>Each call of this method performs a commit to the METADATA CP
             * group. Hence, callers should cache the returned proxy.</strong>
             *
             * @param name name of the fenced_lock proxy
             * @return fenced_lock proxy for the given name
             * @throws HazelcastException if CP Subsystem is not enabled
             */
            std::shared_ptr<fenced_lock> get_lock(const std::string &name);

            /**
             * Returns a proxy for an semaphore instance created on CP
             * Subsystem. Hazelcast's semaphore is a distributed version of
             * <tt>java.util.concurrent.Semaphore</tt>. If no group name is given
             * within the "name" parameter, then the semaphore instance will
             * be created on the DEFAULT CP group. If a group name is given, like
             * cp_subsystem::get_atomic_reference("mySemaphore@group1"), the given group will be
             * initialized first, if not initialized already, and then the
             * semaphore instance will be created on this group. Returned
             * semaphore instance offers linearizability. When a network
             * partition occurs, proxies that exist on the minority side of its CP
             * group lose availability.
             * <p>
             * <strong>Each call of this method performs a commit to the METADATA CP
             * group. Hence, callers should cache the returned proxy.</strong>
             *
             * @param name name of the semaphore proxy
             * @return semaphore proxy for the given name
             * @throws HazelcastException if CP Subsystem is not enabled
             * @see SemaphoreConfig
             */
            std::shared_ptr<semaphore> getSemaphore(const std::string &name);

        private:
            friend client::impl::HazelcastClientInstanceImpl;
            client::spi::ClientContext &context_;
            raft_proxy_factory proxy_factory_;

            cp_subsystem(client::spi::ClientContext &context);
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
