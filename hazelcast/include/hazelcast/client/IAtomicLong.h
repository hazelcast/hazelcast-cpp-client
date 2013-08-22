#ifndef HAZELCAST_ATOMIC_NUMBER
#define HAZELCAST_ATOMIC_NUMBER

#include "spi/ClientContext.h"
#include "spi/InvocationService.h"
#include "serialization/Data.h"
#include <string>


namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
        }

        class IAtomicLong {
            friend class HazelcastClient;

            friend class IdGenerator;

        public:

            /**
             * Returns the name of this IAtomicLong instance.
             *
             * @return name of this instance
             */
            std::string getName() const;

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
            void destroy();

        private:
            template<typename Response, typename Request>
            Response invoke(const Request& request) {
                return context->getInvocationService().template invokeOnKeyOwner<Response>(request, key);
            };

            IAtomicLong();

            void init(const std::string& instanceName, spi::ClientContext *clientContext);


            serialization::Data key;
            std::string instanceName;
            spi::ClientContext *context;
        };
    }
}

#endif /* HAZELCAST_ATOMIC_NUMBER */