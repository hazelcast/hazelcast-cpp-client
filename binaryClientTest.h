//#include "Credentials.h"
//#include "ClientAuthenticationRequest.h"
//#include "Socket.h"
//#include "SerializationService.h"
//#include "ClassDefinitionBuilder.h"
//#include "OutputSocketStream.h"
//
//using namespace hazelcast::client::serialization;
//
//
//
//class ClientPrincipal : public Portable {
//public:
//
//    int getFactoryId() {
//        return -3;
//    };
//
//    int getClassId() {
//        return 3;
//    };
//
//    void writePortable(PortableWriter & writer) {
//        writer.writeUTF("uuid", uuid);
//        writer.writeUTF("ownerUuid", ownerUuid);
//
//    };
//
//    void readPortable(PortableReader & reader) {
//        uuid = reader.readUTF("uuid");
//        ownerUuid = reader.readUTF("ownerUuid");
//    };
//
//private:
//    std::string uuid;
//    std::string ownerUuid;
//};
//
//class MessagePortable : public Portable {
//public:
//
//    int getFactoryId() {
//        return 9;
//    };
//
//    int getClassId() {
//        return 3;
//    };
//
//    void writePortable(PortableWriter & writer) {
//        writer.writeUTF("message", message);
//    };
//
//    void readPortable(PortableReader & reader) {
//        message = reader.readUTF("message");
//    };
//
//private:
//    std::string message;
//};
//
//class MyFactory : public PortableFactory {
//
//    Portable *create(int i) const {
//        if (i == 3) {
//            return new MessagePortable();
//        }
//    }
//};
//
//class ClientFactory : public PortableFactory {
//
//    Portable *create(int i) const {
//        if (i == 2) {
//            return new ClientAuthenticationRequest();
//        } else if (i == 3) {
//            return new Credentials();
//        }
//    };
//};
//
//class SpiFactory : public PortableFactory {
//
//    Portable *create(int i) const {
//        return new Credentials();
//    };
//};
//
//
//void testBinaryClient() {
//    hazelcast::client::Address address(SERVER_ADDRESS, SERVER_PORT);
//    hazelcast::client::protocol::Socket socket(address);
//    std::map< int, PortableFactory const *  > x;
//    x[-1] = new SpiFactory();
//    x[-3] = new ClientFactory();
//    x[9] = new MyFactory();
//
//    SerializationService service(0, x);
//    ClassDefinitionBuilder cd(-3, 3);
//    boost::shared_ptr<ClassDefinition> ptr = cd.addUTFField("uuid").addUTFField("ownerUuid").build();
//    service.getSerializationContext()->registerClassDefinition(ptr);
//    socket.connect();
//    socket.send("CB1", 3);
//    ClientAuthenticationRequest c;
//    Data data = service.toData(c);
//    BufferedDataOutput output(new OutputSocketStream(socket));
//    data.writeData(output);
//
//    void buffer[1024];
//    socket.receive(buffer, 1024);
//    BufferedDataInput input;
//    data.setSerializationContext(service.getSerializationContext());
//    data.readData(input);
//}