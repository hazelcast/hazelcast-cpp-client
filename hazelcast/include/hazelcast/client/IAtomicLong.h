#ifndef HAZELCAST_ATOMIC_NUMBER
#define HAZELCAST_ATOMIC_NUMBER

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/proxy/DistributedObject.h"
#include <string>


namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
        }

        class IAtomicLong : public proxy::DistributedObject {
            friend class HazelcastClient;

            friend class IdGenerator;

        public:

            /**
             * Atomically adds the given value to the current value.
             *
             * @param delta the value to add
             * @return the updated value
             */
            long addAndGet(long delta);

            /**
             * Atomically sets the value to the given updated value
             * only if the current value {@code ==} the expected value.
             *
             * @param expect the expected value
             * @param update the new value
             * @return true if successful; or false if the actual value
             *         was not equal to the expected value.
             */
            bool compareAndSet(long expect, long update);

            /**
             * Atomically decrements the current value by one.
             *
             * @return the updated value
             */
            long decrementAndGet();

            /**
             * Gets the current value.
             *
             * @return the current value
             */
            long get();

            /**
             * Atomically adds the given value to the current value.
             *
             * @param delta the value to add
             * @return the old value before the add
             */
            long getAndAdd(long delta);

            /**
             * Atomically sets the given value and returns the old value.
             *
             * @param newValue the new value
             * @return the old value
             */
            long getAndSet(long newValue);

            /**
             * Atomically increments the current value by one.
             *
             * @return the updated value
             */
            long incrementAndGet();

            /**
             * Atomically increments the current value by one.
             *
             * @return the old value
             */
            long getAndIncrement();

            /**
             * Atomically sets the given value.
             *
             * @param newValue the new value
             */
            void set(long newValue);

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

            IAtomicLong(const std::string &instanceName, spi::ClientContext *context);

            serialization::Data key;
        };
    }
}

#endif /* HAZELCAST_ATOMIC_NUMBER */