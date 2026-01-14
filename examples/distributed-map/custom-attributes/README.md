# Custom Attributes Example

- Write the java class corresponding to the Car class and put that class or jar (`<my_factory>.jar`) into the classpath
  when starting the server and also add a server config for adding the serializer factory to the server.

- Start server:

```bash
java -Dhazelcast.config=examples/distributed-map/custom-attributes/hazelcast-attributes.xml -cp hazelcast-enterprise-<SERVER_VERSION>.jar:<my_factory>.jar com.hazelcast.core.server.HazelcastMemberStarter
```

- Run client tests:

```bash
./build/examples/distributed-map/custom-attributes/customerattribute
```
