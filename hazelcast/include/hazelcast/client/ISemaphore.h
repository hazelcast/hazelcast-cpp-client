#ifndef HAZELCAST_ISEMAPHORE
#define HAZELCAST_ISEMAPHORE

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/exception/IException.h"
#include <string>
#include <stdexcept>


namespace hazelcast {
    namespace client {

        class HAZELCAST_API ISemaphore : public DistributedObject {
            friend class HazelcastClient;

        public:

            /**
             * Try to initialize this ISemaphore instance with given permit count
             *
             * @return true if initialization success
             */
            bool init(int permits);

            /**
             * <p>Acquires a permit, if one is available and returns immediately,
             * reducing the number of available permits by one.
             *
             * <p>If no permit is available then the current thread becomes
             * disabled for thread scheduling purposes and lies dormant until
             * one of three things happens:
             * <ul>
             * <li>Some other thread invokes one of the {@link #release} methods for this
             * semaphore and the current thread is next to be assigned a permit;
             * <li>This ISemaphore instance is destroyed; or
             * <li>Some other thread {@linkplain Thread#interrupt interrupts}
             * the current thread.
             * </ul>
             * <p>If the current thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or
             * <li>is {@linkplain Thread#interrupt interrupted} while waiting
             * for a permit,
             * </ul>
             * then {@link InterruptedException} is thrown and the current thread's
             * interrupted status is cleared.
             *
             * @throws InterruptedException       if the current thread is interrupted
             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
             */
            void acquire();

            /**
             * <p>Acquires the given number of permits, if they are available,
             * and returns immediately, reducing the number of available permits
             * by the given amount.
             *
             * <p>If insufficient permits are available then the current thread becomes
             * disabled for thread scheduling purposes and lies dormant until
             * one of three things happens:
             * <ul>
             * <li>Some other thread invokes one of the {@link #release() release}
             * methods for this semaphore, the current thread is next to be assigned
             * permits and the number of available permits satisfies this request;
             * <li>This ISemaphore instance is destroyed; or
             * <li>Some other thread {@linkplain Thread#interrupt interrupts}
             * the current thread.
             * </ul>
             *
             * <p>If the current thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or
             * <li>is {@linkplain Thread#interrupt interrupted} while waiting
             * for a permit,
             * </ul>
             * then {@link InterruptedException} is thrown and the current thread's
             * interrupted status is cleared.
             *
             * @param permits the number of permits to acquire
             * @throws InterruptedException       if the current thread is interrupted
             * @throws IllegalArgumentException   if {@code permits} is negative
             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
             */
            void acquire(int permits);


            /**
             * Returns the current number of permits currently available in this semaphore.
             *
             * <ul><li>This method is typically used for debugging and testing purposes.
             * </ul>
             *
             * @return the number of permits available in this semaphore
             */
            int availablePermits();


            /**
             * Acquires and returns all permits that are immediately available.
             *
             * @return the number of permits drained
             */
            int drainPermits();

            /**
             * Shrinks the number of available permits by the indicated
             * reduction. This method differs from {@code acquire} in that it does not
             * block waiting for permits to become available.
             *
             * @param reduction the number of permits to remove
             * @throws IllegalArgumentException if {@code reduction} is negative
             */
            void reducePermits(int reduction);

            /**
             * Releases a permit, increasing the number of available permits by
             * one.  If any threads in the cluster are trying to acquire a permit,
             * then one is selected and given the permit that was just released.
             *
             * There is no requirement that a thread that releases a permit must
             * have acquired that permit by calling one of the {@link #acquire() acquire} methods.
             * Correct usage of a semaphore is established by programming convention
             * in the application.
             */
            void release();

            /**
             * Releases the given number of permits, increasing the number of
             * available permits by that amount.
             *
             * There is no requirement that a thread that releases a permit must
             * have acquired that permit by calling one of the {@link #acquire() acquire} methods.
             * Correct usage of a semaphore is established by programming convention
             * in the application.
             *
             * @param permits the number of permits to release
             * @throws IllegalArgumentException if {@code permits} is negative
             */
            void release(int permits);

            /**
             * Acquires a permit, if one is available and returns immediately,
             * with the value {@code true},
             * reducing the number of available permits by one.
             *
             * If no permit is available then this method will return
             * immediately with the value {@code false}.
             *
             * @return {@code true} if a permit was acquired and {@code false}
             *         otherwise
             */
            bool tryAcquire();

            /**
             * Acquires the given number of permits, if they are available, and
             * returns immediately, with the value {@code true},
             * reducing the number of available permits by the given amount.
             *
             * <p>If insufficient permits are available then this method will return
             * immediately with the value {@code false} and the number of available
             * permits is unchanged.
             *
             * @param permits the number of permits to acquire
             * @return {@code true} if the permits were acquired and
             *         {@code false} otherwise
             * @throws IllegalArgumentException if {@code permits} is negative
             */
            bool tryAcquire(int permits);

            /**
             * Acquires a permit from this semaphore, if one becomes available
             * within the given waiting time and the current thread has not
             * been {@linkplain Thread#interrupt interrupted}.
             *
             * Acquires a permit, if one is available and returns immediately,
             * with the value {@code true},
             * reducing the number of available permits by one.
             *
             * If no permit is available then the current thread becomes
             * disabled for thread scheduling purposes and lies dormant until
             * one of three things happens:
             * <ul>
             * <li>Some other thread invokes the {@link #release} method for this
             * semaphore and the current thread is next to be assigned a permit; or
             * <li>Some other thread {@linkplain Thread#interrupt interrupts}
             * the current thread; or
             * <li>The specified waiting time elapses.
             * </ul>
             *
             * If a permit is acquired then the value {@code true} is returned.
             *
             * If the specified waiting time elapses then the value {@code false}
             * is returned.  If the time is less than or equal to zero, the method
             * will not wait at all.
             *
             * <p>If the current thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or
             * <li>is {@linkplain Thread#interrupt interrupted} while waiting
             * for a permit,
             * </ul>
             * then {@link InterruptedException} is thrown and the current thread's
             * interrupted status is cleared.
             *
             * @param timeout the maximum time to wait for a permit
             * @param unit    the time unit of the {@code timeout} argument
             * @return {@code true} if a permit was acquired and {@code false}
             *         if the waiting time elapsed before a permit was acquired
             * @throws InterruptedException       if the current thread is interrupted
             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
             */
            bool tryAcquire(long timeoutInMillis);

            /**
             * Acquires the given number of permits, if they are available and
             * returns immediately, with the value {@code true},
             * reducing the number of available permits by the given amount.
             *
             * If insufficient permits are available then
             * the current thread becomes disabled for thread scheduling
             * purposes and lies dormant until one of three things happens:
             * <ul>
             * <li>Some other thread invokes one of the {@link #release() release}
             * methods for this semaphore, the current thread is next to be assigned
             * permits and the number of available permits satisfies this request; or
             * <li>Some other thread {@linkplain Thread#interrupt interrupts}
             * the current thread; or
             * <li>The specified waiting time elapses.
             * </ul>
             *
             * If the permits are acquired then the value {@code true} is returned.
             *
             * If the specified waiting time elapses then the value {@code false}
             * is returned.  If the time is less than or equal to zero, the method
             * will not wait at all.
             *
             * <p>If the current thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or
             * <li>is {@linkplain Thread#interrupt interrupted} while waiting
             * for a permit,
             * </ul>
             * then {@link InterruptedException} is thrown and the current thread's
             * interrupted status is cleared.
             *
             * @param permits the number of permits to acquire
             * @param timeout the maximum time to wait for the permits
             * @param unit    the time unit of the {@code timeout} argument
             * @return {@code true} if all permits were acquired and {@code false}
             *         if the waiting time elapsed before all permits could be acquired
             * @throws InterruptedException       if the current thread is interrupted
             * @throws IllegalArgumentException   if {@code permits} is negative
             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
             */
            bool tryAcquire(int permits, long timeoutInMillis);

            /**
             * Destroys this object cluster-wide.
             * Clears and releases all resources for this object.
             */
            void onDestroy();

        private:
            void checkNegative(int permits);

            ISemaphore(const std::string &instanceName, spi::ClientContext *context);

            int partitionId;
        };
    }
}

#endif /* HAZELCAST_ISEMAPHORE */