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
        /**
         * ISemaphore is a backed-up distributed alternative to the java.util.concurrent.Semaphore.
         * <p/>
         * ISemaphore is a cluster-wide counting semaphore.  Conceptually,
         * it maintains a set of permits.  Each acquire() blocks if necessary until
         * a permit is available, and then takes it.  Each release() adds a permit,
         * potentially releasing a blocking acquirer. However, no actual permit objects are
         * used; the semaphore just keeps a count of the number available and acts accordingly.
         * <p/>The Hazelcast distributed semaphore implementation guarantees that
         * threads invoking any of the acquire methods are selected
         * to obtain permits in the order in which their invocation of those methods
         * was processed(first-in-first-out; FIFO).  Note that FIFO ordering necessarily
         * applies to specific internal points of execution within the cluster.  So,
         * it is possible for one member to invoke acquire before another, but reach
         * the ordering point after the other, and similarly upon return from the method.
         * <p/>This class also provides convenience methods to 
         * acquire and #release(int) release multiple
         * permits at a time.  Beware of the increased risk of indefinite
         * postponement when using the multiple acquire.  If a single permit is
         * released to a semaphore that is currently blocking, a thread waiting
         * for one permit will acquire it before a thread waiting for multiple
         * permits regardless of the call order.
         * <p/>
         * <ul>
         * <li>Correct usage of a semaphore is established by programming convention in the application.
         * </ul>
         *
         */
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
             * <li>Some other thread invokes one of the #release methods for this
             * semaphore and the current thread is next to be assigned a permit;
             * <li>This ISemaphore instance is destroyed; or
             * <li>Some other thread interrupts
             * the current thread.
             * </ul>
             * <p>If the current thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or
             * <li>is interrupted while waiting
             * for a permit,
             * </ul>
             * then InterruptedException is thrown and the current thread's
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
             * <li>Some other thread invokes one of the #release() release
             * methods for this semaphore, the current thread is next to be assigned
             * permits and the number of available permits satisfies this request;
             * <li>This ISemaphore instance is destroyed; or
             * <li>Some other thread interrupts
             * the current thread.
             * </ul>
             *
             * <p>If the current thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or
             * <li>is interrupted while waiting
             * for a permit,
             * </ul>
             * then InterruptedException is thrown and the current thread's
             * interrupted status is cleared.
             *
             * @param permits the number of permits to acquire
             * @throws InterruptedException       if the current thread is interrupted
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
             * reduction. This method differs from acquire in that it does not
             * block waiting for permits to become available.
             *
             * @param reduction the number of permits to remove
             * @throws IllegalArgumentException if reduction is negative
             */
            void reducePermits(int reduction);

            /**
             * Releases a permit, increasing the number of available permits by
             * one.  If any threads in the cluster are trying to acquire a permit,
             * then one is selected and given the permit that was just released.
             *
             * There is no requirement that a thread that releases a permit must
             * have acquired that permit by calling one of the #acquire() acquire methods.
             * Correct usage of a semaphore is established by programming convention
             * in the application.
             */
            void release();

            /**
             * Releases the given number of permits, increasing the number of
             * available permits by that amount.
             *
             * There is no requirement that a thread that releases a permit must
             * have acquired that permit by calling one of the #acquire() acquire methods.
             * Correct usage of a semaphore is established by programming convention
             * in the application.
             *
             * @param permits the number of permits to release
             */
            void release(int permits);

            /**
             * Acquires a permit, if one is available and returns immediately,
             * with the value true,
             * reducing the number of available permits by one.
             *
             * If no permit is available then this method will return
             * immediately with the value false.
             *
             * @return true if a permit was acquired and false
             *         otherwise
             */
            bool tryAcquire();

            /**
             * Acquires the given number of permits, if they are available, and
             * returns immediately, with the value true,
             * reducing the number of available permits by the given amount.
             *
             * <p>If insufficient permits are available then this method will return
             * immediately with the value false and the number of available
             * permits is unchanged.
             *
             * @param permits the number of permits to acquire
             * @return true if the permits were acquired and
             *         false otherwise
             */
            bool tryAcquire(int permits);

            /**
             * Acquires a permit from this semaphore, if one becomes available
             * within the given waiting time and the current thread has not
             * been interrupted.
             *
             * Acquires a permit, if one is available and returns immediately,
             * with the value true,
             * reducing the number of available permits by one.
             *
             * If no permit is available then the current thread becomes
             * disabled for thread scheduling purposes and lies dormant until
             * one of three things happens:
             * <ul>
             * <li>Some other thread invokes the #release method for this
             * semaphore and the current thread is next to be assigned a permit; or
             * <li>Some other thread interrupts
             * the current thread; or
             * <li>The specified waiting time elapses.
             * </ul>
             *
             * If a permit is acquired then the value true is returned.
             *
             * If the specified waiting time elapses then the value false
             * is returned.  If the time is less than or equal to zero, the method
             * will not wait at all.
             *
             * <p>If the current thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or
             * <li>is interrupted while waiting
             * for a permit,
             * </ul>
             * then InterruptedException is thrown and the current thread's
             * interrupted status is cleared.
             *
             * @param timeout the maximum time to wait for a permit
             * @param unit    the time unit of the timeout argument
             * @return true if a permit was acquired and false
             *         if the waiting time elapsed before a permit was acquired
             * @throws InterruptedException       if the current thread is interrupted
             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
             */
            bool tryAcquire(long timeoutInMillis);

            /**
             * Acquires the given number of permits, if they are available and
             * returns immediately, with the value true,
             * reducing the number of available permits by the given amount.
             *
             * If insufficient permits are available then
             * the current thread becomes disabled for thread scheduling
             * purposes and lies dormant until one of three things happens:
             * <ul>
             * <li>Some other thread invokes one of the #release() release
             * methods for this semaphore, the current thread is next to be assigned
             * permits and the number of available permits satisfies this request; or
             * <li>Some other thread interrupts
             * the current thread; or
             * <li>The specified waiting time elapses.
             * </ul>
             *
             * If the permits are acquired then the value true is returned.
             *
             * If the specified waiting time elapses then the value false
             * is returned.  If the time is less than or equal to zero, the method
             * will not wait at all.
             *
             * <p>If the current thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or
             * <li>is interrupted while waiting
             * for a permit,
             * </ul>
             * then InterruptedException is thrown and the current thread's
             * interrupted status is cleared.
             *
             * @param permits the number of permits to acquire
             * @param timeout the maximum time to wait for the permits
             * @param unit    the time unit of the timeout argument
             * @return true if all permits were acquired and false
             *         if the waiting time elapsed before all permits could be acquired
             * @throws InterruptedException       if the current thread is interrupted
             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
             */
            bool tryAcquire(int permits, long timeoutInMillis);

        private:

            void onDestroy();

            ISemaphore(const std::string &instanceName, spi::ClientContext *context);

            int partitionId;
        };
    }
}

#endif /* HAZELCAST_ISEMAPHORE */