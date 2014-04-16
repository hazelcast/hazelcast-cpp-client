////
//// Created by sancar koyunlu on 03/01/14.
////
//
//
//#ifndef HAZELCAST_EventHandler
//#define HAZELCAST_EventHandler
//
//#include "hazelcast/util/HazelcastDll.h"
//#include <memory>
//
//namespace hazelcast {
//    namespace client {
//        namespace serialization {
//                        namespace pimpl{
//class Data;
//}

//        }
//        namespace impl {
//            class HAZELCAST_API EventHandlerWrapper {
//            public:
//                struct Concept {
//                    virtual ~Concept() {
//
//                    }
//
//                    virtual void handle(const client::serialization::pimpl::Data &data) const = 0;
//                };
//                template<typename EventHandler> struct Model : Concept {
//                public:
//                    Model(EventHandler *eventHandler)
//                    : eventHandler(eventHandler) {
//
//                    }
//
//                    virtual void handle(const client::serialization::pimpl::Data &data) {
//                        eventHandler->handle(data);
//                    }
//
//                    std::auto_ptr<EventHandler> eventHandler;
//                };
//
//
//                template<typename EventHandler>
//                EventHandlerWrapper(EventHandler *eventHandler)
//                :eventHandler(new Model<EventHandler>(eventHandler)) {
//
//                };
//
//                void handle(const client::serialization::pimpl::Data &data);
//
//            private:
//                std::auto_ptr<Concept> eventHandler;
//            };
//        }
//    }
//}
//
//#endif //HAZELCAST_EventHandler

