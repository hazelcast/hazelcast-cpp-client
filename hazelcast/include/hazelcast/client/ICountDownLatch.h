#ifndef HAZELCAST_ICOUNT_DOWN_LATCH
#define HAZELCAST_ICOUNT_DOWN_LATCH

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/proxy/DistributedObject.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/exception/IException.h"
#include <string>
#include <stdexcept>


namespace hazelcast {
    namespace client {

        namespace spi {
            class ClientContext;
        }

        class HAZELCAST_API ICountDownLatch : public proxy::DistributedObject {
            friend class HazelcastClient;

        public:

            /**
             * Causes the current thread to wait until the latch has counted down to
             * zero, an exception is thrown, or the specified waiting time elapses.
             * <p/>
             * <p>If the current count is zero then this method returns immediately
             * with the value {@code true}.
             * <p/>
             * <p>If the current count is greater than zero then the current
             * thread becomes disabled for thread scheduling purposes and lies
             * dormant until one of five things happen:
             * <ul>
             * <li>The count reaches zero due to invocations of the
             * {@link #countDown} method;
             * <li>This ICountDownLatch instance is destroyed;
             * <li>The countdown owner becomes disconnected;
             * <li>Some other thread {@linkplain Thread#interrupt interrupts}
             * the current thread; or
             * <li>The specified waiting time elapses.
             * </ul>
             * <p/>
             * <p>If the count reaches zero then the method returns with the
             * value {@code true}.
             * <p/>
             * <p/>If the countdown owner becomes disconnected while waiting then
             * {@link MemberLeftException} will be thrown.
             * <p>If the current thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or
             * <li>is {@linkplain Thread#interrupt interrupted} while waiting,
             * </ul>
             * then {@link InterruptedException} is thrown and the current thread's
             * interrupted status is cleared.
             * <p>If the specified waiting time elapses then the value {@code false}
             * is returned.  If the time is less than or equal to zero, the method
             * will not wait at all.
             *
             * @param timeout the maximum time to wait
             * @param unit    the time unit of the {@code timeout} argument
             * @return {@code true} if the count reached zero and {@code false}
             *         if the waiting time elapsed before the count reached zero
             * @throws MemberLeftException        if the countdown owner becomes disconnected while waiting
             * @throws InterruptedException       if the current thread is interrupted
             * @throws IllegalStateException      if hazelcast instance is shutdown while waiting
             */
            bool await(long timeoutInMillis);

            /**
             * Decrements the count of the latch, releasing all waiting threads if
             * the count reaches zero.
             * <p/>
             * If the current count is greater than zero then it is decremented.
             * If the new count is zero:
             * <ul>
             * <li>All waiting threads are re-enabled for thread scheduling purposes; and
             * <li>Countdown owner is set to {@code null}.
             * </ul>
             * <p/>
             * If the current count equals zero then nothing happens.
             */
            void countDown();

            /**
             * Returns the current count.
             *
             * @return current count
             */
            int getCount();

            /**
             * Sets the count to the given value if the current count is zero. The calling
             * cluster member becomes the owner of the countdown and is responsible for
             * staying connected to the cluster until the count reaches zero.
             * <p/>If the owner becomes disconnected before the count reaches zero:
             * <ul>
             * <li>Count will be set to zero;
             * <li>Countdown owner will be set to {@code null}; and
             * <li>All awaiting threads will be thrown a {@link MemberLeftException}.
             * </ul>
             * <p/>If count is not zero then this method does nothing and returns {@code false}.
             *
             * @param count the number of times {@link #countDown} must be invoked
             *              before threads can pass through {@link #await}
             * @return {@code true} if the new count was set or {@code false} if the current
             *         count is not zero
             * @throws IllegalArgumentException if {@code count} is negative
             */
            bool trySetCount(int count);

            /**
             * Destroys this object cluster-wide.
             * Clears and releases all resources for this object.
             */
            void onDestroy();


        private:
            template<typename Response, typename Request>
            boost::shared_ptr<Response> invoke(const Request &request) {
                return getContext().getInvocationService().template invokeOnKeyOwner<Response>(request, key);
            };

            ICountDownLatch(const std::string &instanceName, spi::ClientContext *clientContext);

            serialization::Data key;
        };
    }
}

#endif /* HAZELCAST_ICOUNT_DOWN_LATCH */