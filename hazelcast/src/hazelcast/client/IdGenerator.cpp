#include "hazelcast/client/IdGenerator.h"

namespace hazelcast {
    namespace client {

        IdGenerator::IdGenerator(const std::string &instanceName, spi::ClientContext *context)
        : DistributedObject("idGeneratorService", instanceName, context)
        , atomicLong("hz:atomic:idGenerator:" + instanceName, context)
        , local(new util::AtomicInt(-1))
        , residue(new util::AtomicInt(BLOCK_SIZE))
        , localLock(new util::Mutex) {

        }


        bool IdGenerator::init(long id) {
            if (id <= 0) {
                return false;
            }
            long step = (id / BLOCK_SIZE);

            util::LockGuard lg(*localLock);
            bool init = atomicLong.compareAndSet(0, step + 1);
            if (init) {
                *local = step;
                *residue = (id % BLOCK_SIZE) + 1;
            }
            return init;
        }

        long IdGenerator::newId() {
            int value = (*residue)++;
            if (value >= BLOCK_SIZE) {
                util::LockGuard lg(*localLock);
                value = *residue;
                if (value >= BLOCK_SIZE) {
                    *local = atomicLong.getAndIncrement();
                    *residue = 0;
                }
                return newId();

            }
            return int(*local) * BLOCK_SIZE + value;
        }

        void IdGenerator::onDestroy() {
            atomicLong.onDestroy();
        }

    }
}
