#include "hazelcast/client/IAtomicLong.h"
#include "hazelcast/client/atomiclong/AddAndGetRequest.h"
#include "hazelcast/client/atomiclong/CompareAndSetRequest.h"
#include "hazelcast/client/atomiclong/GetAndAddRequest.h"
#include "hazelcast/client/atomiclong/GetAndSetRequest.h"
#include "hazelcast/client/atomiclong/SetRequest.h"

namespace hazelcast {
    namespace client {


        IAtomicLong::IAtomicLong(const std::string& objectName, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:atomicLongService", objectName, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&objectName);
            partitionId = getPartitionId(keyData);
        }

        long IAtomicLong::addAndGet(long delta) {
            atomiclong::AddAndGetRequest *request = new atomiclong::AddAndGetRequest(getName(), delta);
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, long);
            return *result;
        }

        bool IAtomicLong::compareAndSet(long expect, long update) {
            atomiclong::CompareAndSetRequest *request = new atomiclong::CompareAndSetRequest(getName(), expect, update);
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, bool);
            return *result;
        }

        long IAtomicLong::decrementAndGet() {
            return addAndGet(-1);
        }

        long IAtomicLong::get() {
            return getAndAdd(0);
        }

        long IAtomicLong::getAndAdd(long delta) {
            atomiclong::GetAndAddRequest *request = new atomiclong::GetAndAddRequest(getName(), delta);
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, long);
            return *result;
        }

        long IAtomicLong::getAndSet(long newValue) {
            atomiclong::GetAndSetRequest *request = new atomiclong::GetAndSetRequest(getName(), newValue);
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, long);
            return *result;
        }

        long IAtomicLong::incrementAndGet() {
            return addAndGet(1);
        }

        long IAtomicLong::getAndIncrement() {
            return getAndAdd(1);
        }

        void IAtomicLong::set(long newValue) {
            atomiclong::SetRequest *request = new atomiclong::SetRequest(getName(), newValue);
            invoke(request, partitionId);
        }
    }
}
