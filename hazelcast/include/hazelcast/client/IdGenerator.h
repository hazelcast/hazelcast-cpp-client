#ifndef HAZELCAST_ID_GENERATOR
#define HAZELCAST_ID_GENERATOR

#include "hazelcast/client/IAtomicLong.h"
#include <string>
#include <boost/atomic.hpp>
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
        }

        namespace impl {
            class IdGeneratorSupport;
        }

        /**
         * Cluster-wide unique id generator.
         */
        class HAZELCAST_API IdGenerator : public DistributedObject {
            friend class HazelcastClient;

        public:
            enum {
                BLOCK_SIZE = 1000
            };

            /**
             * Try to initialize this IdGenerator instance with given id
             *
             * @return true if initialization success
             */
            bool init(long id);

            /**
             * Generates and returns cluster-wide unique id.
             * Generated ids are guaranteed to be unique for the entire cluster
             * as long as the cluster is live. If the cluster restarts then
             * id generation will start from 0.
             *
             * @return cluster-wide new unique id
             */
            long newId();

        private:

            IAtomicLong atomicLong;
            boost::shared_ptr< boost::atomic<int> > local;
            boost::shared_ptr< boost::atomic<int> > residue;
            boost::shared_ptr< boost::mutex > localLock;
            IdGenerator(const std::string &instanceName, spi::ClientContext *context);

            void onDestroy();

        };
    }
}

#endif /* HAZELCAST_ID_GENERATOR */