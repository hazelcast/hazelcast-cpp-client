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
        :DistributedObject("hz:impl:semaphoreService", name, context) {
            serialization::Data keyData = context->getSerializationService().toData<std::string>(&name);
            partitionId = getPartitionId(keyData);
        };

        bool ISemaphore::init(int permits) {
            checkNegative(permits);
            semaphore::InitRequest *request = new semaphore::InitRequest(getName(), permits);
            return invoke<bool>(request, partitionId);
        };

        void ISemaphore::acquire() {
            acquire(1);
        };

        void ISemaphore::acquire(int permits) {
            checkNegative(permits);
            semaphore::AcquireRequest *request = new semaphore::AcquireRequest(getName(), permits, -1);
            invoke<bool>(request, partitionId);
        };

        int ISemaphore::availablePermits() {
            semaphore::AvailableRequest *request = new semaphore::AvailableRequest(getName());
            boost::shared_ptr<int> response = invoke<int>(request, partitionId);
            return *response;
        };

        int ISemaphore::drainPermits() {
            semaphore::DrainRequest *request = new semaphore::DrainRequest(getName());
            boost::shared_ptr<int> response = invoke<int>(request, partitionId);
            return *response;
        };

        void ISemaphore::reducePermits(int reduction) {
            checkNegative(reduction);
            semaphore::ReduceRequest *request = new semaphore::ReduceRequest(getName(), reduction);
            invoke<bool>(request, partitionId);
        };

        void ISemaphore::release() {
            release(1);
        };

        void ISemaphore::release(int permits) {
            semaphore::ReleaseRequest *request = new semaphore::ReleaseRequest(getName(), permits);
            invoke<bool>(request, partitionId);
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
            semaphore::AcquireRequest *request = new semaphore::AcquireRequest(getName(), permits, timeoutInMillis);
            return *(invoke<bool>(request, partitionId));
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