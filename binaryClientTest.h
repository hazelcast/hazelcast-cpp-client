#include "Credentials.h"
#include "ClientAuthenticationRequest.h"
#include "Socket.h"
#include "SerializationService.h"
#include "ClassDefinitionBuilder.h"
#include "OutputSocketStream.h"

using namespace hazelcast::client::serialization;
class ClientFactory : public PortableFactory{

    Portable *create(int i) const{
        if(i == 2){
            return new ClientAuthenticationRequest() ;
        } else if(i == 3){
            return new Credentials();
        }
    };
};

class SpiFactory : public PortableFactory{

    Portable *create(int i) const{
        return new Credentials();
    };
};


void testBinaryClient(){
    hazelcast::client::Address adress("192.168.2.6","5701");
    hazelcast::client::protocol::Socket socket(adress);
    std::map< int, PortableFactory const *  >  x;
    x[-1] = new SpiFactory();
    x[-3] = new ClientFactory();
    SerializationService service(0,x);
    ClassDefinitionBuilder cd(-3,3);
    boost::shared_ptr<ClassDefinition> ptr = cd.addUTFField("uuid").addUTFField("ownerUuid").build();
    service.getSerializationContext()->registerClassDefinition(ptr);
    socket.sendData("CB1", 3);
    ClientAuthenticationRequest c;
    Data data = service.toData(c);
    DataOutput output(new OutputSocketStream(socket));
    data.writeData(output);
}