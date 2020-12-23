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
#include <condition_variable>
#include <boost/thread/future.hpp>

#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/exception/protocol_exceptions.h"

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
        class raft_proxy_factory;

        struct HAZELCAST_API raft_group_id {
            std::string name;
            int64_t seed;
            int64_t group_id;

            bool operator==(const raft_group_id &rhs) const;

            bool operator!=(const raft_group_id &rhs) const;
        };

        class HAZELCAST_API cp_proxy : public client::proxy::ProxyImpl {
        public:
            cp_proxy(const std::string &service_name, const std::string &proxy_name, client::spi::ClientContext *context,
                     const raft_group_id &group_id, const std::string &object_name);

            const raft_group_id &get_group_id() const;

        protected:
            raft_group_id group_id_;
            std::string object_name_;

            void on_destroy() ;
        };

        namespace internal {
            namespace session {
                class proxy_session_manager;
            }
        }
        class HAZELCAST_API session_aware_proxy : public cp_proxy {
        public:
            session_aware_proxy(const std::string &service_name, const std::string &proxy_name,
                                client::spi::ClientContext *context, const raft_group_id &group_id,
                                const std::string &object_name,
                                internal::session::proxy_session_manager &session_manager);

        protected:
            internal::session::proxy_session_manager &session_manager_;

            /**
             * Decrements acquire count of the session.
             * Returns silently if no session exists for the given id.
             */
            void release_session(int64_t session_id);
        };

        /**
         * Client-side Raft-based proxy implementation of atomic long.
         */
        class HAZELCAST_API atomic_long : public cp_proxy {
        public:
            atomic_long(const std::string &name, client::spi::ClientContext &context,
                        const raft_group_id &group_id, const std::string &object_name);

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
             * @return the old value.
             */
            boost::future<int64_t> get_and_decrement();

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
            boost::future<int64_t> get_and_set(int64_t new_value);

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
            boost::future<void> set(int64_t new_value);

            /**
             * Alters the currently stored value by applying a function on it.
             *
             * @param function the function applied to the currently stored value
             */
            template<typename F>
            boost::future<void> alter(const F &function) {
                return to_void_future(alter_and_get(function));
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
                auto f = to_data(function);
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
                auto f = to_data(function);
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
                auto f = to_data(function);
                return to_object<R>(apply_data(f));
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
            alter_data(client::serialization::pimpl::data &function_data, alter_result_type result_type);

            boost::future<boost::optional<client::serialization::pimpl::data>>
            apply_data(client::serialization::pimpl::data &function_data);
        };

        /**
         * Client-side Raft-based proxy implementation of atomic reference.
         */
        class HAZELCAST_API atomic_reference : public cp_proxy {
        public:
            atomic_reference(const std::string &name, client::spi::ClientContext &context,
                             const raft_group_id &group_id, const std::string &object_name);

            template<typename T>
            boost::future<boost::optional<typename std::remove_pointer<T>::type>> get() {
                return to_object<typename std::remove_pointer<T>::type>(get_data());
            }

            template<typename T>
            boost::future<boost::optional<typename std::remove_pointer<T>::type>> set(T new_value) {
                return to_object<typename std::remove_pointer<T>::type>(
                        set_data(to_data<typename std::remove_pointer<T>::type>(new_value)));
            }

            template<typename T>
            boost::future<boost::optional<typename std::remove_pointer<T>::type>> get_and_set(T new_value) {
                return to_object<typename std::remove_pointer<T>::type>(
                        get_and_set_data(to_data<typename std::remove_pointer<T>::type>(new_value)));
            }

            template<typename T, typename V>
            boost::future<bool> compare_and_set(T expect, V update) {
                return compare_and_set_data(to_data<typename std::remove_pointer<T>::type>(expect),
                                            to_data<typename std::remove_pointer<V>::type>(update));
            }

            boost::future<bool> is_null();

            boost::future<void> clear();

            template<typename T>
            boost::future<bool> contains(T value) {
                return contains_data(to_data<typename std::remove_pointer<T>::type>(value));
            }

            template<typename F>
            boost::future<void> alter(const F &function) {
                return alter_data(to_data(function));
            }

            template<typename T, typename F>
            boost::future<boost::optional<typename std::remove_pointer<T>::type>> alter_and_get(const F &function) {
                return to_object<typename std::remove_pointer<T>::type>(alter_and_get_data(to_data(function)));
            }

            template<typename T, typename F>
            boost::future<boost::optional<typename std::remove_pointer<T>::type>> get_and_alter(const F &function) {
                return to_object<typename std::remove_pointer<T>::type>(get_and_alter_data(to_data(function)));
            }

            template<typename R, typename F>
            boost::future<boost::optional<R>> apply(const F &function) {
                return to_object<R>(apply_data(to_data(function)));
            }

        private:
            static constexpr const char *SERVICE_NAME = "hz:raft:atomicRefService";

            enum struct return_value_type {
                NO_VALUE,
                OLD,
                NEW
            };

            boost::future<boost::optional<client::serialization::pimpl::data>> get_data();

            boost::future<boost::optional<client::serialization::pimpl::data>>
            set_data(const client::serialization::pimpl::data &new_value_data);

            boost::future<boost::optional<client::serialization::pimpl::data>>
            get_and_set_data(const client::serialization::pimpl::data &new_value_data);

            boost::future<bool> compare_and_set_data(const client::serialization::pimpl::data &expect_data,
                                                     const client::serialization::pimpl::data &update_data);

            boost::future<bool> contains_data(const client::serialization::pimpl::data &value_data);

            boost::future<void> alter_data(const client::serialization::pimpl::data &function_data);

            boost::future<boost::optional<client::serialization::pimpl::data>>
            alter_and_get_data(const client::serialization::pimpl::data &function_data);

            boost::future<boost::optional<client::serialization::pimpl::data>>
            get_and_alter_data(const client::serialization::pimpl::data &function_data);

            boost::future<boost::optional<client::serialization::pimpl::data>>
            apply_data(const client::serialization::pimpl::data &function_data);

            boost::future<boost::optional<client::serialization::pimpl::data>>
            invoke_apply(const client::serialization::pimpl::data function_data, return_value_type return_type,
                         bool alter);
        };

        class HAZELCAST_API latch : public cp_proxy {
        public:
            latch(const std::string &name, client::spi::ClientContext &context, const raft_group_id &group_id,
                  const std::string &object_name);

            /**
             * Sets the count to the given value if the current count is zero.
             * <p>
             * If count is not zero, then this method does nothing and returns \false.
             *
             * @param count the number of times \count_down must be invoked
             *              before threads can pass through \wait_for
             * @return \true if the new count was set, \false if the current count is not zero
             * @throws illegal_argument if \count is not positive number
             */
            boost::future<bool> try_set_count(int32_t count);

            /**
             * Returns the current count.
             *
             * @return the current count
             */
            boost::future<int32_t> get_count();

            /**
             * Decrements the count of the latch, releasing all waiting threads if
             * the count reaches zero.
             * <p>
             * If the current count is greater than zero, then it is decremented.
             * If the new count is zero:
             * <ul>
             * <li>All waiting threads are re-enabled for thread scheduling purposes, and
             * <li>Countdown owner is set to \null.
             * </ul>
             * If the current count equals zero, then nothing happens.
             */
            boost::future<void> count_down();

            /**
             * see https://en.cppreference.com/w/cpp/thread/latch/try_wait
             *
             * @return true only if the internal counter has reached zero. This function may spuriously return false with
             * very low probability even if the internal counter has reached zero.
             */
            boost::future<bool> try_wait();

            /**
             * see https://en.cppreference.com/w/cpp/thread/latch/wait
             *
             * Blocks the calling thread until the internal counter reaches 0 or server thread is interrupted (e.g. cluster shutdown).
             * If it is zero already, returns immediately.
             *
             * @return when the latch count becaomes 0.
             */
            boost::future<void> wait();

            /**
             * Causes the current thread to wait until the latch has counted down to
             * zero, or an exception is thrown, or the specified waiting time elapses.
             * <p>
             * If the current count is zero then this method returns immediately
             * with the value \no_timeout.
             * <p>
             * If the current count is greater than zero, then the current
             * thread becomes disabled for thread scheduling purposes and lies
             * dormant until one of five things happen:
             * <ul>
             * <li>the count reaches zero due to invocations of the \count_down method,
             * <li>this \latch instance is destroyed,
             * <li>the countdown owner becomes disconnected,
             * <li>some other thread interrupts the current server thread, or
             * <li>the specified waiting time elapses.
             * </ul>
             * If the count reaches zero, then the method returns with the
             * value \no_timeout.
             * <p>
             * If the specified waiting time elapses then the value \timeout
             * is returned.  If the time is less than or equal to zero, the method
             * will not wait at all.
             *
             * @param rel_time the maximum time to wait
             * @return \no_timeout if the count reached zero, \timeout
             * if the waiting time elapsed before the count reached zero
             * @throws illegal_state if the Hazelcast instance is shutdown while waiting
             */
            boost::future<std::cv_status> wait_for(std::chrono::milliseconds timeout);

            /**
             * see \wait_for for details of the operation.
             *
             * @tparam Clock The clock type used
             * @tparam Duration The duration type used
             * @param timeout_time The time to wait until.
             * @return \no_timeout if the count reached zero, \timeout
             * if the waiting time elapsed before the count reached zero
             */
            template<typename Clock, typename Duration>
            boost::future<std::cv_status> wait_until(const std::chrono::time_point<Clock, Duration> &timeout_time) {
                return wait_for(std::chrono::duration_cast<std::chrono::milliseconds>(timeout_time - Clock::now()));
            }

        private:
            static constexpr const char *SERVICE_NAME = "hz:raft:countDownLatchService";

            boost::future<int32_t> get_round();

            void count_down(int round, boost::uuids::uuid invocation_uid);

        };

        /**
         * Client-side proxy of Raft-based lock API
         */
        class HAZELCAST_API fenced_lock : public session_aware_proxy {
        public:
            /**
             * Representation of a failed lock attempt where
             * the caller thread has not acquired the lock
             */
            static constexpr int64_t INVALID_FENCE = 0L;

            fenced_lock(const std::string &name, client::spi::ClientContext &context,
                             const raft_group_id &group_id, const std::string &object_name);

            /**
             * Acquires the lock.
             * <p>
             * When the caller already holds the lock and the current lock() call is
             * reentrant, the call can fail with
             * \lock_acquire_limit_reached if the lock acquire limit is
             * already reached. Please see server side FencedLockConfig for more
             * information.
             * <p>
             * If the lock is not available then the current thread becomes disabled
             * for thread scheduling purposes and lies dormant until the lock has been
             * acquired.
             * <p>
             * Consider the following scenario:
             * <pre>
             *     auto lock = ...;
             *     lock->lock().get();
             *     // The caller thread hits a long pause
             *     // and its CP session is closed on the CP group.
             *     lock->lock().get();
             * </pre>
             * In this scenario, a thread acquires the lock, then user process
             * encounters a long pause, which is longer than
             * server side CPSubsystemConfig#getSessionTimeToLiveSeconds config. In this case,
             * its CP session will be closed on the corresponding CP group because
             * it could not commit session heartbeats in the meantime. After the user pause
             * wakes up again, the same thread attempts to acquire the lock
             * reentrantly. In this case, the second lock() call fails by throwing
             * \lock_ownership_lost. If the caller wants to deal with
             * its session loss by taking some custom actions, it can handle the thrown
             * \lock_ownership_lost instance.
             *
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed while locking reentrantly
             * @throws lock_acquire_limit_reached if the lock call is reentrant
             *         and the configured lock acquire limit is already reached.
             */
            boost::future<void> lock();

            /**
             * Acquires the lock and returns the fencing token assigned to the current
             * thread for this lock acquire. If the lock is acquired reentrantly,
             * the same fencing token is returned, or the lock() call can fail with
             * \lock_acquire_limit_reached if the lock acquire limit is
             * already reached. Please see server FencedLockConfig for more
             * information.
             * <p>
             * If the lock is not available then the current thread becomes disabled
             * for thread scheduling purposes and lies dormant until the lock has been
             * acquired.
             * <p>
             * This is a convenience method for the following pattern:
             * <pre>
             *     auto lock = ...;
             *     lock.lock().get();
             *     return lock->get_fence().get();
             * </pre>
             * <p>
             * Consider the following scenario:
             * <pre>
             *     auto lock = ...;
             *     lock->lock_and_get_fence().get();
             *     // The caller thread hits a long pause
             *     // and its CP session is closed on the CP group.
             *     lock->lock_and_get_fence().get();
             * </pre>
             * In this scenario, a thread acquires the lock, then user process
             * encounters a long pause, which is longer than
             * server side CPSubsystemConfig#getSessionTimeToLiveSeconds config. In this case,
             * its CP session will be closed on the corresponding CP group because
             * it could not commit session heartbeats in the meantime. After the user pause
             * wakes up again, the same thread attempts to acquire the lock
             * reentrantly. In this case, the second lock() call fails by throwing
             * \lock_ownership_lost. If the caller wants to deal with
             * its session loss by taking some custom actions, it can handle the thrown
             * \lock_ownership_lost instance.
             *
             * Fencing tokens are monotonic numbers that are incremented each time
             * the lock switches from the free state to the acquired state. They are
             * simply used for ordering lock holders. A lock holder can pass
             * its fencing to the shared resource to fence off previous lock holders.
             * When this resource receives an operation, it can validate the fencing
             * token in the operation.
             * <p>
             * Consider the following scenario where the lock is free initially:
             * <pre>
             *     auto lock = ...; // the lock is free
             *     auto fence1 = lock->lock_and_get_fence().get(); // (1)
             *     long fence2 = lock->lock_and_get_fence().get(); // (2)
             *     assert(fence1 == fence2);
             *     lock->unlock().get();
             *     lock->unlock().get();
             *     long fence3 = lock->lock_and_get_fence().get(); // (3)
             *     assert(fence3 &gt; fence1);
             * </pre>
             * In this scenario, the lock is acquired by a thread in the cluster. Then,
             * the same thread reentrantly acquires the lock again. The fencing token
             * returned from the second acquire is equal to the one returned from the
             * first acquire, because of reentrancy. After the second acquire, the lock
             * is released 2 times, hence becomes free. There is a third lock acquire
             * here, which returns a new fencing token. Because this last lock acquire
             * is not reentrant, its fencing token is guaranteed to be larger than the
             * previous tokens, independent of the thread that has acquired the lock.
             *
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed while locking reentrantly
             * @throws lock_acquire_limit_reached if the lock call is reentrant
             *         and the configured lock acquire limit is already reached.
             */
            boost::future<int64_t> lock_and_get_fence();

            /**
             * Acquires the lock if it is available or already held by the current
             * thread at the time of invocation &amp; the acquire limit is not exceeded,
             * and immediately returns with the value \true. If the lock is not
             * available, then this method immediately returns with the value
             * \false. When the call is reentrant, it can return \false
             * if the lock acquire limit is exceeded. Please see
             * server FencedLockConfig for more information.
             * <p>
             * A typical usage idiom for this method would be:
             * <pre>
             *     auto lock = ...;
             *     if (lock->try_lock().get()) {
             *         try {
             *             // manipulate protected state
             *             lock->unlock().get();
             *         } catch(...) {
             *             lock->unlock().get();
             *         }
             *     } else {
             *         // perform alternative actions
             *     }
             * </pre>
             * This usage ensures that the lock is unlocked if it was acquired,
             * and doesn't try to unlock if the lock was not acquired.
             *
             * @return \true if the lock was acquired and
             *         \false otherwise
             *
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed while locking reentrantly
             */
            boost::future<bool> try_lock();

            /**
             * Acquires the lock if it is free within the given waiting time,
             * or already held by the current thread.
             * <p>
             * If the lock is available, this method returns immediately with the value
             * \true. When the call is reentrant, it immediately returns
             * \true if the lock acquire limit is not exceeded. Otherwise,
             * it returns \false on the reentrant lock attempt if the acquire
             * limit is exceeded. Please see server FencedLockConfig for more
             * information.
             * <p>
             * If the lock is not available then the current thread becomes disabled
             * for thread scheduling purposes and lies dormant until the lock is
             * acquired by the current thread or the specified waiting time elapses.
             * <p>
             * If the lock is acquired, then the value \true is returned.
             * <p>
             * If the specified waiting time elapses, then the value \false
             * is returned. If the time is less than or equal to zero, the method does
             * not wait at all.
             *
             * @param timeout the maximum time to wait for the lock
             * @return \true if the lock was acquired and \false
             *         if the waiting time elapsed before the lock was acquired
             *
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed while locking reentrantly
             */
            boost::future<bool> try_lock(std::chrono::milliseconds timeout);

            /**
             * Acquires the lock only if it is free or already held by the current
             * thread at the time of invocation &amp; the acquire limit is not exceeded,
             * and returns the fencing token assigned to the current thread for this
             * lock acquire. If the lock is acquired reentrantly, the same fencing
             * token is returned. If the lock is already held by another caller or
             * the lock acquire limit is exceeded, then this method immediately returns
             * \INVALID_FENCE that represents a failed lock attempt.
             * Please see server FencedLockConfig for more information.
             * <p>
             * This is a convenience method for the following pattern:
             * <pre>
             *     auto lock = ...;
             *     if (lock->try_lock().get()) {
             *         return lock->get_fence().get();
             *     } else {
             *         return INVALID_FENCE;
             *     }
             * </pre>
             * <p>
             * Consider the following scenario where the lock is free initially:
             * <pre>
             *     auto lock = ...; // the lock is free
             *     lock->try_lock_and_get_fence().get();
             *     // JVM of the caller thread hits a long pause
             *     // and its CP session is closed on the CP group.
             *     lock->try_lock_and_get_fence().get();
             * </pre>
             * In this scenario, a thread acquires the lock, then its JVM instance
             * encounters a long pause, which is longer than
             * server config CPSubsystemConfig#getSessionTimeToLiveSeconds. In this case,
             * its CP session will be closed on the corresponding CP group because
             * it could not commit session heartbeats in the meantime. After the JVM
             * instance wakes up again, the same thread attempts to acquire the lock
             * reentrantly. In this case, the second lock() call fails by throwing
             * \lock_ownership_lost which extends
             * \illegal_monitor_state_exception. If the caller wants to deal with
             * its session loss by taking some custom actions, it can handle the thrown
             * \lock_ownership_lost instance.
             * <p>
             * Fencing tokens are monotonic numbers that are incremented each time
             * the lock switches from the free state to the acquired state. They are
             * simply used for ordering lock holders. A lock holder can pass
             * its fencing to the shared resource to fence off previous lock holders.
             * When this resource receives an operation, it can validate the fencing
             * token in the operation.
             * <p>
             * Consider the following scenario where the lock is free initially:
             * <pre>
             *     auto lock = ...; // the lock is free
             *     long fence1 = lock->try_lock_and_get_fence(); // (1)
             *     long fence2 = lock->try_lock_and_get_fence(); // (2)
             *     assert fence1 == fence2;
             *     lock->unlock().get();
             *     lock->unlock().get();
             *     long fence3 = lock->try_lock_and_get_fence(); // (3)
             *     assert fence3 &gt; fence1;
             * </pre>
             * In this scenario, the lock is acquired by a thread in the cluster. Then,
             * the same thread reentrantly acquires the lock again. The fencing token
             * returned from the second acquire is equal to the one returned from the
             * first acquire, because of reentrancy. After the second acquire, the lock
             * is released 2 times, hence becomes free. There is a third lock acquire
             * here, which returns a new fencing token. Because this last lock acquire
             * is not reentrant, its fencing token is guaranteed to be larger than the
             * previous tokens, independent of the thread that has acquired the lock.
             *
             * @return the fencing token if the lock was acquired and
             *         \INVALID_FENCE otherwise
             *
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed while locking reentrantly
             */
            boost::future<int64_t> try_lock_and_get_fence();

            /**
             * Acquires the lock if it is free within the given waiting time,
             * or already held by the current thread at the time of invocation &amp;
             * the acquire limit is not exceeded, and returns the fencing token
             * assigned to the current thread for this lock acquire. If the lock is
             * acquired reentrantly, the same fencing token is returned. If the lock
             * acquire limit is exceeded, then this method immediately returns
             * \INVALID_FENCE that represents a failed lock attempt.
             * Please see server FencedLockConfig for more information.
             * <p>
             * If the lock is not available then the current thread becomes disabled
             * for thread scheduling purposes and lies dormant until the lock is
             * acquired by the current thread or the specified waiting time elapses.
             * <p>
             * If the specified waiting time elapses, then \INVALID_FENCE
             * is returned. If the time is less than or equal to zero, the method does
             * not wait at all.
             * <p>
             * This is a convenience method for the following pattern:
             * <pre>
             *     auto lock = ...;
             *     if (lock->try_lock(timeout).get()) {
             *         return lock->get_fence().get();
             *     } else {
             *         return INVALID_FENCE;
             *     }
             * </pre>
             * <p>
             * Consider the following scenario where the lock is free initially:
             * <pre>
             *      auto lock = ...; // the lock is free
             *      lock->try_lock_and_get_fence(timeout).get();
             *      // The caller process hits a long pause and its CP session
             *      is closed on the CP group.
             *      lock->try_lock_and_get_fence(timeout).get();
             * </pre>
             * In this scenario, a thread acquires the lock, then its process
             * encounters a long pause, which is longer than
             * server config \CPSubsystemConfig#getSessionTimeToLiveSeconds. In this case,
             * its CP session will be closed on the corresponding CP group because
             * it could not commit session heartbeats in the meantime. After the process
             * wakes up again, the same thread attempts to acquire the lock
             * reentrantly. In this case, the second lock() call fails by throwing
             * \lock_ownership_lost which extends
             * {@link illegal_monitor_state}. If the caller wants to deal with
             * its session loss by taking some custom actions, it can handle the thrown
             * \lock_ownership_lost instance.
             * <p>
             * Fencing tokens are monotonic numbers that are incremented each time
             * the lock switches from the free state to the acquired state. They are
             * simply used for ordering lock holders. A lock holder can pass
             * its fencing to the shared resource to fence off previous lock holders.
             * When this resource receives an operation, it can validate the fencing
             * token in the operation.
             * <p>
             * Consider the following scenario where the lock is free initially:
             * <pre>
             *     auto lock = ...; // the lock is free
             *     long fence1 = lock->try_lock_and_get_fence(timeout).get(); // (1)
             *     long fence2 = lock->try_lock_and_get_fence(timeout).get(); // (2)
             *     assert fence1 == fence2;
             *     lock->unlock().get();
             *     lock->unlock().get();
             *     long fence3 = lock->try_lock_and_get_fence(timeout).get(); // (3)
             *     assert fence3 &gt; fence1;
             * </pre>
             * In this scenario, the lock is acquired by a thread in the cluster. Then,
             * the same thread reentrantly acquires the lock again. The fencing token
             * returned from the second acquire is equal to the one returned from the
             * first acquire, because of reentrancy. After the second acquire, the lock
             * is released 2 times, hence becomes free. There is a third lock acquire
             * here, which returns a new fencing token. Because this last lock acquire
             * is not reentrant, its fencing token is guaranteed to be larger than the
             * previous tokens, independent of the thread that has acquired the lock.
             *
             * @param timeout the maximum time to wait for the lock
             * @return the fencing token if the lock was acquired and
             *         \INVALID_FENCE otherwise
             *
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed while locking reentrantly
             */
            boost::future<int64_t> try_lock_and_get_fence(std::chrono::milliseconds timeout);

            /**
             * Releases the lock if the lock is currently held by the current thread.
             *
             * @throws illegal_monitor_state if the lock is not held by
             *         the current thread
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed before the current thread releases the lock
             */
            boost::future<void> unlock();

            /**
             * Returns the fencing token if the lock is held by the current thread.
             * <p>
             * Fencing tokens are monotonic numbers that are incremented each time
             * the lock switches from the free state to the acquired state. They are
             * simply used for ordering lock holders. A lock holder can pass
             * its fencing to the shared resource to fence off previous lock holders.
             * When this resource receives an operation, it can validate the fencing
             * token in the operation.
             *
             * @return the fencing token if the lock is held by the current thread
             *
             * @throws illegal_monitor_state if the lock is not held by
             *         the current thread
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed while the current thread is holding the lock
             */
            boost::future<int64_t> get_fence();

            /**
             * Returns whether this lock is locked or not.
             *
             * @return \true if this lock is locked by any thread
             *         in the cluster, \false otherwise.
             *
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed while the current thread is holding the lock
             */
            boost::future<bool> is_locked();

            /**
             * Returns whether the lock is held by the current thread or not.
             *
             * @return \true if the lock is held by the current thread or not,
             *         \false otherwise.
             *
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed while the current thread is holding the lock
             */
            boost::future<bool> is_locked_by_current_thread();

            /**
             * Returns the reentrant lock count if the lock is held by any thread
             * in the cluster.
             *
             * @return the reentrant lock count if the lock is held by any thread
             *         in the cluster
             *
             * @throws lock_ownership_lost if the underlying CP session is
             *         closed while the current thread is holding the lock
             */
            boost::future<int32_t> get_lock_count();

            /**
             * Returns id of the CP group that runs this \fenced_lock instance
             *
             * @return id of the CP group that runs this \fenced_lock instance
             */
            const raft_group_id &get_group_id();

            friend bool operator==(const fenced_lock &lhs, const fenced_lock &rhs);

        protected:
            void post_destroy() ;

        private:
            struct lock_ownership_state {
                int64_t fence;
                int32_t lock_count;
                int64_t session_id;
                int64_t thread_id;

                bool is_locked_by(int64_t session, int64_t thread);

                bool is_locked();
            };

            static constexpr const char *SERVICE_NAME = "hz:raft:lockService";

            // thread id -> id of the session that has acquired the lock
            util::SynchronizedMap<int64_t, int64_t> locked_session_ids_;

            void
            verify_locked_session_id_if_present(int64_t thread_id, int64_t session_id, bool should_release);

            void throw_lock_ownership_lost(int64_t session_id) const;

            void throw_illegal_monitor_state() const;

            boost::future<int64_t>
            do_lock(int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid);

            boost::future<int64_t>
            do_try_lock(int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid,
                        std::chrono::milliseconds timeout);

            boost::future<bool>
            do_unlock(int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid);

            boost::future<lock_ownership_state> do_get_lock_ownership_state();

            void invalidate_session(int64_t session_id);

            void verify_no_locked_session_id_present(int64_t thread_id);
        };

        /**
         * \counting_semaphore is a fault-tolerant distributed alternative to the
         * stl \counting_semaphore (@see https://en.cppreference.com/w/cpp/thread/counting_semaphore). Semaphores are
         * often used to restrict the number of threads than can access some physical or logical resource.
         * <p>
         * \counting_semaphore is a cluster-wide counting semaphore. Conceptually, it maintains
         * a set of permits. Each \acquire blocks if necessary until a permit
         * is available, and then takes it. Dually, each \release adds a
         * permit, potentially releasing a blocking acquirer. However, no actual permit
         * objects are used; the semaphore just keeps a count of the number available
         * and acts accordingly.
         * <p>
         * Hazelcast's distributed semaphore implementation guarantees that threads
         * invoking any of the \acquire methods are selected to
         * obtain permits in the order of their invocations (first-in-first-out; FIFO).
         * Note that FIFO ordering implies the order which the primary replica of an
         * \counting_semaphore receives these acquire requests. Therefore, it is
         * possible for one member to invoke \acquire before another member,
         * but its request hits the primary replica after the other member.
         * <p>
         * This class also provides convenience methods to work
         * with multiple permits at once. Beware of the increased risk of
         * indefinite postponement when using the multiple-permit acquire. If permits
         * are released one by one, a thread waiting for one permit will acquire
         * it before a thread waiting for multiple permits regardless of the call order.
         * <p>
         * Correct usage of a semaphore is established by programming convention
         * in the application.
         * <p>
         * \counting_semaphore works on top of the Raft consensus algorithm. It offers linearizability during crash
         * failures and network partitions. It is CP with respect to the CAP principle.
         * If a network partition occurs, it remains available on at most one side of
         * the partition.
         * <p>
         * It has 2 variations:
         * <ul>
         * <li>
         * The default impl is session-aware. In this
         * one, when a caller makes its very first \acquire call, it starts
         * a new CP session with the underlying CP group. Then, liveliness of the
         * caller is tracked via this CP session. When the caller fails, permits
         * acquired by this HazelcastInstance are automatically and safely released. However,
         * the session-aware version comes with a limitation, that is,
         * a HazelcastInstance cannot release permits before acquiring them
         * first. In other words, a Hazelcast client can release only
         * the permits it has acquired earlier. It means, you can acquire a permit
         * from one thread and release it from another thread using the same
         * Hazelcast client, but not different instances of Hazelcast client. You can use
         * the session-aware CP \counting_semaphore impl by disabling server side JDK compatibility
         * server SemaphoreConfig#setJDKCompatible(boolean). Although
         * the session-aware impl has a minor difference to the JDK Semaphore, we think
         * it is a better fit for distributed environments because of its safe
         * auto-cleanup mechanism for acquired permits.
         * </li>
         * <li>
         * The second impl is sessionless. This impl
         * does not perform auto-cleanup of acquired permits on failures. Acquired
         * permits are not bound to HazelcastInstance and permits can be released without
         * acquiring first. However, you need to handle failed permit owners on your own. If a Hazelcast
         * server or a client fails while holding some permits, they will not be
         * automatically released. You can use the sessionless CP \counting_semaphore
         * impl by enabling server side config SemaphoreConfig#setJDKCompatible(boolean).
         * </li>
         * </ul>
         * <p>
         * There is a subtle difference between the lock and semaphore abstractions.
         * A lock can be assigned to at most one endpoint at a time, so we have a total
         * order among its holders. However, permits of a semaphore can be assigned to
         * multiple endpoints at a time, which implies that we may not have a total
         * order among permit holders. In fact, permit holders are partially ordered.
         * For this reason, the fencing token approach, which is explained in
         * \fenced_lock, does not work for the semaphore abstraction. Moreover,
         * each permit is an independent entity. Multiple permit acquires and reentrant
         * lock acquires of a single endpoint are not equivalent. The only case where
         * a semaphore behaves like a lock is the binary case, where the semaphore has
         * only 1 permit (similar to stl \binary_semaphore). In this case, the semaphore works like a non-reentrant lock.
         * <p>
         * All of the API methods in the new CP \fenced_lock impl offer
         * the exactly-once execution semantics for the session-aware version.
         * For instance, even if a \release call is internally retried
         * because of a crashed Hazelcast member, the permit is released only once.
         * However, this guarantee is not given for the sessionless, a.k.a,
         * JDK-compatible CP \counting_semaphore. For this version, you can tune
         * execution semantics via server side config CPSubsystemConfig#setFailOnIndeterminateOperationState(boolean).
         */
        class HAZELCAST_API counting_semaphore : public session_aware_proxy {
        public:
            //---- std::counting_semaphore method impl starts ---------
            /**
             * Acquires a permit if \permits are available, and returns immediately,
             * reducing the number of available permits by number of permits.
             * <p>
             * If no permit is available, then the current thread becomes
             * disabled for thread scheduling purposes and lies dormant until
             * one of three things happens:
             * <ul>
             * <li>some other thread invokes one of the \release methods for this
             * semaphore and the current thread is next to be assigned a permit,</li>
             * <li>this counting_semaphore instance is destroyed, or</li>
             * <li>some other thread interrupts the current server thread.</li>
             * </ul>
             * <p>If the current server thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or</li>
             * <li>is interrupted while waiting for a permit,</li>
             * </ul>
             * then \interrupted is thrown and the current thread's
             * interrupted status is cleared.
             *
             * @param permits the number of permits to acquire
             * @throws illegal_argument if \permits is negative or zero
             * @throws illegal_state if hazelcast instance is shutdown while waiting
             */
            virtual boost::future<void> acquire(int32_t permits = 1) = 0;

            /**
             * Releases a permit and increases the number of available permits by one.
             * If some threads in the cluster are blocked for acquiring a permit, one
             * of them will unblock by acquiring the permit released by this call.
             * <p>
             * If the underlying \counting_semaphore is configured as non-JDK compatible
             * via server side SemaphoreConfig then a HazelcastInstance can only release a permit which
             * it has acquired before. In other words, a HazelcastInstance cannot release a permit
             * without acquiring it first.
             * <p>
             * Otherwise, which means the underlying impl is the JDK compatible
             * Semaphore is configured via server side SemaphoreConfig, there is no requirement
             * that a HazelcastInstance that releases a permit must have acquired that permit by
             * calling one of the \acquire methods. A HazelcastInstance can freely
             * release a permit without acquiring it first. In this case, correct usage
             * of a semaphore is established by programming convention in the application.
             *
             * @param permits The number of permits to release.
             * @throws illegal_argument if \permits is negative or zero
             * @throws illegal_state if the Semaphore is non-JDK-compatible
             *         and the caller does not have a permit
             */
            virtual boost::future<void> release(int32_t permits = 1 ) = 0;

            /**
             * Acquires if the given number of permits available, and returns \true
             * immediately. If no permit is available, returns \false immediately.
             *
             * @param permits the number of permits to acquire
             * @return {@code true} if a permit was acquired, {@code false} otherwise
             */
            boost::future<bool> try_acquire(int32_t permits = 1);

            /**
             * Acquires the number of permits and returns , if one becomes available
             * during the given waiting time and the current server thread has not been
             * interrupted. If a permit is acquired,
             * the number of available permits in the \counting_semaphore instance is
             * also reduced by number of permits.
             * <p>
             * If no permit is available, then the current thread becomes
             * disabled for thread scheduling purposes and lies dormant until
             * one of three things happens:
             * <ul>
             * <li>some other thread releases a permit and the current thread is next
             * to be assigned a permit,</li>
             * <li>or this \counting_semaphore instance is destroyed,</li>
             * <li>or some other thread interrupts the current server thread,</li>
             * <li>or the specified waiting time elapses.</li>
             * </ul>
             * <p>
             * Returns \true if a permit is successfully acquired.
             * <p>
             * Returns \false if the specified waiting time elapses without
             * acquiring a permit. If the time is less than or equal to zero,
             * the method will not wait at all.
             * <p>
             *
             * @param the number of permits to acquire
             * @param rel_time the maximum time to wait for a permit. The supported resolution is milliseconds.
             * @return \true if a permit was acquired and {@code false}
             * if the waiting time elapsed before a permit was acquired
             * @throws interrupted  if the current server thread is interrupted
             * @throws illegal_state if hazelcast instance is shutdown while waiting
             */
            boost::future<bool> try_acquire_for(std::chrono::milliseconds rel_time, int32_t permits = 1);

            /**
             * Same as \try_acquire_for except that the wait time is calculated from the \abs_time point.
             */
            template<class Clock, class Duration>
            boost::future<bool> try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time, int32_t permits = 1) {
                auto now = Clock::now();
                return try_acquire_for(std::chrono::duration_cast<std::chrono::milliseconds>(abs_time - now), permits);
            }

            //---- std::counting_semaphore method impl ends ----------

            // --- extended methods
            /**
             * Tries to initialize this counting_semaphore instance with the given permit count
             *
             * @param permits the given permit count
             * @return true if initialization success. false if already initialized
             * @throws illegal_argument if \permits is negative
             */
            boost::future<bool> init(int32_t permits);

            /**
             * Returns the current number of permits currently available in this semaphore.
             * <p>
             * This method is typically used for debugging and testing purposes.
             *
             * @return the number of permits available in this semaphore
             */
            boost::future<int32_t> available_permits();

            /**
             * Acquires and returns all permits that are available at invocation time.
             *
             * @return the number of permits drained
             */
            virtual boost::future<int32_t> drain_permits() = 0;

            /**
             * Reduces the number of available permits by the indicated amount. This
             * method differs from \acquire as it does not block until permits
             * become available. Similarly, if the caller has acquired some permits,
             * they are not released with this call.
             *
             * @param reduction the number of permits to reduce
             * @throws illegal_argument if \reduction is negative
             */
            boost::future<void> reduce_permits(int32_t reduction);

            /**
             * Increases the number of available permits by the indicated amount. If
             * there are some threads waiting for permits to become available, they
             * will be notified. Moreover, if the caller has acquired some permits,
             * they are not released with this call.
             *
             * @param increase the number of permits to increase
             * @throws illegal_argument if \increase is negative
             */
            boost::future<void> increase_permits(int32_t increase);

        protected:
            static constexpr const char *SERVICE_NAME = "hz:raft:semaphoreService";

            counting_semaphore(const std::string &proxy_name, client::spi::ClientContext *context,
                               const raft_group_id &group_id, const std::string &object_name,
                               internal::session::proxy_session_manager &session_manager);

            virtual boost::future<bool> try_acquire_for_millis(int32_t permits, std::chrono::milliseconds timeout) = 0;

            virtual int64_t get_thread_id() = 0;

            virtual boost::future<void> do_change_permits(int32_t delta) = 0;

            boost::future<void> do_release(int32_t permits, int64_t thread_id, int64_t session_id);
        };

        class HAZELCAST_API sessionless_semaphore : public counting_semaphore {
        public:
            sessionless_semaphore(const std::string &proxy_name, client::spi::ClientContext *context,
                                  const raft_group_id &group_id, const std::string &object_name,
                                  internal::session::proxy_session_manager &session_manager);

            boost::future<void> acquire(int32_t permits) override;

            boost::future<void> release(int32_t permits) override;

            boost::future<int32_t> drain_permits() override;

        protected:
            boost::future<bool> try_acquire_for_millis(int32_t permits, std::chrono::milliseconds timeout) override;

            int64_t get_thread_id() override;

            boost::future<void> do_change_permits(int32_t delta) override;

        private:
            boost::future<bool> do_try_acquire(int32_t permits, std::chrono::milliseconds timeout_ms);
        };

        class HAZELCAST_API session_semaphore : public counting_semaphore {
        public:
            session_semaphore(const std::string &proxy_name, client::spi::ClientContext *context,
                                  const raft_group_id &group_id, const std::string &object_name,
                                  internal::session::proxy_session_manager &session_manager);

            boost::future<void> acquire(int32_t permits) override;

            boost::future<void> release(int32_t permits) override;

            boost::future<int32_t> drain_permits() override;

        protected:
            /**
             * Since a proxy does not know how many permits will be drained on
             * the Raft group, it uses this constant to increment its local session
             * acquire count. Then, it adjusts the local session acquire count after
             * the drain response is returned.
             */
            static constexpr int32_t DRAIN_SESSION_ACQ_COUNT = 1024;

            boost::future<bool> try_acquire_for_millis(int32_t permits, std::chrono::milliseconds timeout) override;

            void throw_illegal_state_exception(std::exception_ptr e);

            int64_t get_thread_id() override;

            boost::future<void> do_change_permits(int32_t delta) override;
        };

        /**
         * Client-side Raft-based proxy implementation of atomic reference
         *
         */
        class HAZELCAST_API raft_proxy_factory {
        public:
            raft_proxy_factory(client::spi::ClientContext &context);

            static std::string without_default_group_name(const std::string &n);

            static std::string object_name_for_proxy(const std::string &name);

            template<typename T>
            boost::future<std::shared_ptr<T>> create_proxy(const std::string &name) {
                auto proxy_name = without_default_group_name(name);
                auto object_name = object_name_for_proxy(proxy_name);
                return get_group_id(proxy_name, object_name).then([=] (boost::future<raft_group_id> f) {
                    auto group_id = f.get();
                    return create<T>(std::move(group_id), proxy_name, object_name);
                });
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

            std::shared_ptr<counting_semaphore>
            create_semaphore(raft_group_id &&group_id, const std::string &proxy_name, const std::string &object_name);

            template<typename T, typename = typename std::enable_if<std::is_same<counting_semaphore, T>::value>::type>
            std::shared_ptr<counting_semaphore>
            create(raft_group_id &&group_id, const std::string &proxy_name, const std::string &object_name) {
                return create_semaphore(std::move(group_id), proxy_name, object_name);
            }

            boost::future<raft_group_id> get_group_id(const std::string &proxy_name, const std::string &object_name);
        };

        /**
         * CP Subsystem is a component of Hazelcast that builds a strongly consistent
         * layer for a set of distributed data structures. Its APIs can be used for
         * implementing distributed coordination use cases, such as leader election,
         * distributed locking, synchronization, and metadata management.
         * It is accessed via hazelcast_client::get_cp_subsystem. Its data
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
             * @throws hazelcast_ if CP Subsystem is not enabled
             */
            boost::future<std::shared_ptr<atomic_long>> get_atomic_long(const std::string &name);

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
             * @throws hazelcast_ if CP Subsystem is not enabled
             */
            boost::future<std::shared_ptr<atomic_reference>> get_atomic_reference(const std::string &name);

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
             * @throws hazelcast_ if CP Subsystem is not enabled
             */
            boost::future<std::shared_ptr<latch>> get_latch(const std::string &name);

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
             * @throws hazelcast_ if CP Subsystem is not enabled
             */
            boost::future<std::shared_ptr<fenced_lock>> get_lock(const std::string &name);

            /**
             * Returns a proxy for an semaphore instance created on CP
             * Subsystem. Hazelcast's semaphore is a distributed version of
             * <tt>java.util.concurrent.Semaphore</tt>. If no group name is given
             * within the "name" parameter, then the semaphore instance will
             * be created on the DEFAULT CP group. If a group name is given, like
             * cp_subsystem::get_semaphore("mySemaphore@group1"), the given group will be
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
             * @throws hazelcast_ if CP Subsystem is not enabled
             */
            boost::future<std::shared_ptr<counting_semaphore>> get_semaphore(const std::string &name);

        private:
            friend client::impl::hazelcast_client_instance_impl;
            client::spi::ClientContext &context_;
            raft_proxy_factory proxy_factory_;

            cp_subsystem(client::spi::ClientContext &context);
        };
    }
}

namespace std {
    template<>
    struct HAZELCAST_API hash<hazelcast::cp::raft_group_id> {
        std::size_t operator()(const hazelcast::cp::raft_group_id &group_id) const noexcept;
    };
}

namespace boost {
    template<>
    struct HAZELCAST_API hash<hazelcast::cp::raft_group_id> {
        std::size_t operator()(const hazelcast::cp::raft_group_id &group_id) const noexcept;
    };
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
