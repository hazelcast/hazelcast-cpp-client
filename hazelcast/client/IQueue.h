//#ifndef HAZELCAST_IQUEUE
//#define HAZELCAST_IQUEUE
//
//#include <stdexcept>
//
//namespace hazelcast {
//    namespace client {
//
//        class impl::ClientService;
//
//        template<typename E>
//        class IQueue {
//        public:
//
//            IQueue(std::string instanceName, impl::ClientService& clientService) : instanceName(instanceName)
//            , clientService(clientService) {
//
//            };
//
//            IQueue(const IQueue& rhs) : instanceName(rhs.instanceName)
//            , clientService(rhs.clientService) {
//            };
//
//            ~IQueue() {
//
//            };
//
//            std::string getName() const {
//                return instanceName;
//            };
//
//        private:
//            std::string instanceName;
//            impl::ClientService& clientService;
//        };
//    }
//}
//
//#endif /* HAZELCAST_IQUEUE */