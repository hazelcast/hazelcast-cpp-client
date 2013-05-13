//#include "ICountDownLatch.h"
//
//namespace hazelcast {
//    namespace client {
//
//        ICountDownLatch::ICountDownLatch(std::string instanceName, impl::ClientService& clientService) : instanceName(instanceName)
//        , clientService(clientService) {
//
//        };
//
//        ICountDownLatch::ICountDownLatch(const ICountDownLatch& rhs) : instanceName(rhs.instanceName)
//        , clientService(rhs.clientService) {
//        };
//
//        ICountDownLatch::~ICountDownLatch() {
//
//        };
//
//        std::string ICountDownLatch::getName() const {
//            return instanceName;
//        };
//
//    }
//}