//#ifndef HAZELCAST_ISEMAPHORE
//#define HAZELCAST_ISEMAPHORE
//
//#include <string>
//#include <stdexcept>
//#include "HazelcastException.h"
//#include "ClientService.h"
//
//namespace hazelcast {
//    namespace client {
//
//        class impl::ClientService;
//
//        class ISemaphore {
//        public:
//
//            ISemaphore(std::string instanceName, impl::ClientService& clientService);
//
//            ISemaphore(const ISemaphore& rhs);
//
//            ~ISemaphore();
//
//            std::string getName() const;
//
//            /**
//             * Try to initialize this ISemaphore instance with given permit count
//             *
//             * @return true if initialization success
//             */
//            bool init(int permits);
//
//            /**
//             * <p>Acquires a permit, if one is available and returns immediately,
//             * reducing the number of available permits by one.
//             * <p/>
//             * <p>If no permit is available then the current thread becomes
//             * disabled for thread scheduling purposes and lies dormant until
//             * one of three things happens:
//             * <ul>
//             * <li>Some other thread invokes one of the {@link #release} methods for this
//             * semaphore and the current thread is next to be assigned a permit;
//             * <li>This ISemaphore instance is destroyed; or
//             * <li>Some other thread {@linkplain Thread#interrupt interrupts}
//             * the current thread.
//             * </ul>
//             * <p>If the current thread:
//             * <ul>
//             * <li>has its interrupted status set on entry to this method; or
//             * <li>is {@linkplain Thread#interrupt interrupted} while waiting
//             * for a permit,
//             * </ul>
//             * then {@link InterruptedException} is thrown and the current thread's
//             * interrupted status is cleared.
//             *
//             * @throws InterruptedException       if the current thread is interrupted
//             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
//             */
//            void acquire()throw ( hazelcast::client::HazelcastException);
//
//            /**
//             * <p>Acquires the given number of permits, if they are available,
//             * and returns immediately, reducing the number of available permits
//             * by the given amount.
//             * <p/>
//             * <p>If insufficient permits are available then the current thread becomes
//             * disabled for thread scheduling purposes and lies dormant until
//             * one of three things happens:
//             * <ul>
//             * <li>Some other thread invokes one of the {@link #release() release}
//             * methods for this semaphore, the current thread is next to be assigned
//             * permits and the number of available permits satisfies this request;
//             * <li>This ISemaphore instance is destroyed; or
//             * <li>Some other thread {@linkplain Thread#interrupt interrupts}
//             * the current thread.
//             * </ul>
//             * <p/>
//             * <p>If the current thread:
//             * <ul>
//             * <li>has its interrupted status set on entry to this method; or
//             * <li>is {@linkplain Thread#interrupt interrupted} while waiting
//             * for a permit,
//             * </ul>
//             * then {@link InterruptedException} is thrown and the current thread's
//             * interrupted status is cleared.
//             *
//             * @param permits the number of permits to acquire
//             * @throws InterruptedException       if the current thread is interrupted
//             * @throws IllegalArgumentException   if {@code permits} is negative
//             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
//             */
//            void acquire(int permits)throw ( hazelcast::client::HazelcastException);
//
//
//            /**
//             * Returns the current number of permits currently available in this semaphore.
//             * <p/>
//             * <ul><li>This method is typically used for debugging and testing purposes.
//             * </ul>
//             *
//             * @return the number of permits available in this semaphore
//             */
//            int availablePermits();
//
//
//            /**
//             * Acquires and returns all permits that are immediately available.
//             *
//             * @return the number of permits drained
//             */
//            int drainPermits();
//
//            /**
//             * Shrinks the number of available permits by the indicated
//             * reduction. This method differs from {@code acquire} in that it does not
//             * block waiting for permits to become available.
//             *
//             * @param reduction the number of permits to remove
//             * @throws IllegalArgumentException if {@code reduction} is negative
//             */
//            void reducePermits(int reduction);
//
//            /**
//             * Releases a permit, increasing the number of available permits by
//             * one.  If any threads in the cluster are trying to acquire a permit,
//             * then one is selected and given the permit that was just released.
//             * <p/>
//             * There is no requirement that a thread that releases a permit must
//             * have acquired that permit by calling one of the {@link #acquire() acquire} methods.
//             * Correct usage of a semaphore is established by programming convention
//             * in the application.
//             */
//            void release();
//
//            /**
//             * Releases the given number of permits, increasing the number of
//             * available permits by that amount.
//             * <p/>
//             * There is no requirement that a thread that releases a permit must
//             * have acquired that permit by calling one of the {@link #acquire() acquire} methods.
//             * Correct usage of a semaphore is established by programming convention
//             * in the application.
//             *
//             * @param permits the number of permits to release
//             * @throws IllegalArgumentException if {@code permits} is negative
//             */
//            void release(int permits);
//
//            /**
//             * Acquires a permit, if one is available and returns immediately,
//             * with the value {@code true},
//             * reducing the number of available permits by one.
//             * <p/>
//             * If no permit is available then this method will return
//             * immediately with the value {@code false}.
//             *
//             * @return {@code true} if a permit was acquired and {@code false}
//             *         otherwise
//             */
//            bool tryAcquire();
//
//            /**
//             * Acquires the given number of permits, if they are available, and
//             * returns immediately, with the value {@code true},
//             * reducing the number of available permits by the given amount.
//             * <p/>
//             * <p>If insufficient permits are available then this method will return
//             * immediately with the value {@code false} and the number of available
//             * permits is unchanged.
//             *
//             * @param permits the number of permits to acquire
//             * @return {@code true} if the permits were acquired and
//             *         {@code false} otherwise
//             * @throws IllegalArgumentException if {@code permits} is negative
//             */
//            bool tryAcquire(int permits);
//
//            /**
//             * Acquires a permit from this semaphore, if one becomes available
//             * within the given waiting time and the current thread has not
//             * been {@linkplain Thread#interrupt interrupted}.
//             * <p/>
//             * Acquires a permit, if one is available and returns immediately,
//             * with the value {@code true},
//             * reducing the number of available permits by one.
//             * <p/>
//             * If no permit is available then the current thread becomes
//             * disabled for thread scheduling purposes and lies dormant until
//             * one of three things happens:
//             * <ul>
//             * <li>Some other thread invokes the {@link #release} method for this
//             * semaphore and the current thread is next to be assigned a permit; or
//             * <li>Some other thread {@linkplain Thread#interrupt interrupts}
//             * the current thread; or
//             * <li>The specified waiting time elapses.
//             * </ul>
//             * <p/>
//             * If a permit is acquired then the value {@code true} is returned.
//             * <p/>
//             * If the specified waiting time elapses then the value {@code false}
//             * is returned.  If the time is less than or equal to zero, the method
//             * will not wait at all.
//             * <p/>
//             * <p>If the current thread:
//             * <ul>
//             * <li>has its interrupted status set on entry to this method; or
//             * <li>is {@linkplain Thread#interrupt interrupted} while waiting
//             * for a permit,
//             * </ul>
//             * then {@link InterruptedException} is thrown and the current thread's
//             * interrupted status is cleared.
//             *
//             * @param timeout the maximum time to wait for a permit
//             * @param unit    the time unit of the {@code timeout} argument
//             * @return {@code true} if a permit was acquired and {@code false}
//             *         if the waiting time elapsed before a permit was acquired
//             * @throws InterruptedException       if the current thread is interrupted
//             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
//             */
//            bool tryAcquire(long timeoutInMillis) throw ( hazelcast::client::HazelcastException);
//
//            /**
//             * Acquires the given number of permits, if they are available and
//             * returns immediately, with the value {@code true},
//             * reducing the number of available permits by the given amount.
//             * <p/>
//             * If insufficient permits are available then
//             * the current thread becomes disabled for thread scheduling
//             * purposes and lies dormant until one of three things happens:
//             * <ul>
//             * <li>Some other thread invokes one of the {@link #release() release}
//             * methods for this semaphore, the current thread is next to be assigned
//             * permits and the number of available permits satisfies this request; or
//             * <li>Some other thread {@linkplain Thread#interrupt interrupts}
//             * the current thread; or
//             * <li>The specified waiting time elapses.
//             * </ul>
//             * <p/>
//             * If the permits are acquired then the value {@code true} is returned.
//             * <p/>
//             * If the specified waiting time elapses then the value {@code false}
//             * is returned.  If the time is less than or equal to zero, the method
//             * will not wait at all.
//             * <p/>
//             * <p>If the current thread:
//             * <ul>
//             * <li>has its interrupted status set on entry to this method; or
//             * <li>is {@linkplain Thread#interrupt interrupted} while waiting
//             * for a permit,
//             * </ul>
//             * then {@link InterruptedException} is thrown and the current thread's
//             * interrupted status is cleared.
//             *
//             * @param permits the number of permits to acquire
//             * @param timeout the maximum time to wait for the permits
//             * @param unit    the time unit of the {@code timeout} argument
//             * @return {@code true} if all permits were acquired and {@code false}
//             *         if the waiting time elapsed before all permits could be acquired
//             * @throws InterruptedException       if the current thread is interrupted
//             * @throws IllegalArgumentException   if {@code permits} is negative
//             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
//             */
//            bool tryAcquire(int permits, long timeoutInMillis) throw ( hazelcast::client::HazelcastException);
//
//
//        private:
//            std::string instanceName;
//            impl::ClientService& clientService;
//        };
//    }
//}
//
//#endif /* HAZELCAST_ISEMAPHORE */