////
//// Created by sancar koyunlu on 03/01/14.
////
//
//#include "hazelcast/client/exception/TargetDisconnectedException.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace exception {
//
//            TargetDisconnectedException::TargetDisconnectedException(const Address& address)
//            : IException("", "")
//            , address(address){
//
//            }
//
//            TargetDisconnectedException::~TargetDisconnectedException() throw(){
//
//            }
//
//            char const *TargetDisconnectedException::what() const throw(){
//                return IException::what();
//            }
//        }
//    }
//}