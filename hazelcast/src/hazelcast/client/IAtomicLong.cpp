#include "hazelcast/client/IAtomicLong.h"
#include "hazelcast/client/atomiclong/AddAndGetRequest.h"
#include "hazelcast/client/atomiclong/CompareAndSetRequest.h"
#include "hazelcast/client/atomiclong/GetAndAddRequest.h"
#include "hazelcast/client/atomiclong/GetAndSetRequest.h"
#include "hazelcast/client/atomiclong/SetRequest.h"

namespace hazelcast {
    namespace client {


        IAtomicLong::IAtomicLong(const std::string &instanceName, spi::ClientContext *context)
        :DistributedObject("hz:impl:atomicLongService", instanceName, context) {
            serialization::Data keyData = getContext().getSerializationService().toData<std::string>(&instanceName);
            partitionId = getPartitionId(keyData);
        };

        long IAtomicLong::addAndGet(long delta) {
            atomiclong::AddAndGetRequest *request = new atomiclong::AddAndGetRequest(getName(), delta);
            boost::shared_ptr<long> response = invoke<long>(request, partitionId);
            return *response;
        };

        bool IAtomicLong::compareAndSet(long expect, long update) {
            atomiclong::CompareAndSetRequest *request = new atomiclong::CompareAndSetRequest(getName(), expect, update);
            boost::shared_ptr<bool> response = invoke<bool>(request, partitionId);
            return *response;
        };

        long IAtomicLong::decrementAndGet() {
            return addAndGet(-1);
        };

        long IAtomicLong::get() {
            return getAndAdd(0);
        };

        long IAtomicLong::getAndAdd(long delta) {
            atomiclong::GetAndAddRequest *request = new atomiclong::GetAndAddRequest(getName(), delta);
            boost::shared_ptr<long> response = invoke<long>(request, partitionId);
            return *response;
        };

        long IAtomicLong::getAndSet(long newValue) {
            atomiclong::GetAndSetRequest *request = new atomiclong::GetAndSetRequest(getName(), newValue);
            boost::shared_ptr<long> response = invoke<long>(request, partitionId);
            return *response;
        };

        long IAtomicLong::incrementAndGet() {
            return addAndGet(1);
        };

        long IAtomicLong::getAndIncrement() {
            return getAndAdd(1);
        };

        void IAtomicLong::set(long newValue) {
            atomiclong::SetRequest *request = new atomiclong::SetRequest(getName(), newValue);
            invoke<bool>(request, partitionId);
        };

        void IAtomicLong::onDestroy() {
        };


    }
}