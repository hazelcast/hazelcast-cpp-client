#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/semaphore/InitRequest.h"
#include "hazelcast/client/semaphore/AcquireRequest.h"
#include "hazelcast/client/semaphore/AvailableRequest.h"
#include "hazelcast/client/semaphore/DrainRequest.h"
#include "hazelcast/client/semaphore/ReduceRequest.h"
#include "hazelcast/client/semaphore/ReleaseRequest.h"
#include "hazelcast/client/exception/InterruptedException.h"


namespace hazelcast {
    namespace client {

        ISemaphore::ISemaphore(const std::string &name, spi::ClientContext *context)
        :DistributedObject("hz:impl:semaphoreService", name, context)
        , key(context->getSerializationService().toData<std::string>(&name)) {

        };

        bool ISemaphore::init(int permits) {
            checkNegative(permits);
            semaphore::InitRequest request(getName(), permits);
            return invoke<bool>(request);
        };

        void ISemaphore::acquire() {
            acquire(1);
        };

        void ISemaphore::acquire(int permits) {
            checkNegative(permits);
            semaphore::AcquireRequest request(getName(), permits, -1);
            invoke<bool>(request);
        };

        int ISemaphore::availablePermits() {
            semaphore::AvailableRequest request(getName());
            boost::shared_ptr<int> response = invoke<int>(request);
            return *response;
        };

        int ISemaphore::drainPermits() {
            semaphore::DrainRequest request(getName());
            boost::shared_ptr<int> response = invoke<int>(request);
            return *response;
        };

        void ISemaphore::reducePermits(int reduction) {
            checkNegative(reduction);
            semaphore::ReduceRequest request(getName(), reduction);
            invoke<bool>(request);
        };

        void ISemaphore::release() {
            release(1);
        };

        void ISemaphore::release(int permits) {
            semaphore::ReleaseRequest request(getName(), permits);
            invoke<bool>(request);
        };

        bool ISemaphore::tryAcquire() {
            return tryAcquire(1);
        };

        bool ISemaphore::tryAcquire(int permits) {
            checkNegative(permits);
            try {
                return tryAcquire(permits, 0);
            } catch (exception::InterruptedException &e) {
                return false;
            }
        };

        bool ISemaphore::tryAcquire(long timeoutInMillis) {
            return tryAcquire(1, timeoutInMillis);
        };

        bool ISemaphore::tryAcquire(int permits, long timeoutInMillis) {
            checkNegative(permits);
            semaphore::AcquireRequest request(getName(), permits, timeoutInMillis);
            return *(invoke<bool>(request));
        };

        void ISemaphore::checkNegative(int permits) {
            if (permits < 0) {
                throw exception::IException("ISemaphore::checkNegative", "Permits cannot be negative!");
            }
        };

        void ISemaphore::onDestroy() {
        }


    }
}