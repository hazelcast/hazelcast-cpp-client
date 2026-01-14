# enableBackPressure

- Start server:

```bash
java -Dhazelcast.config=examples/backpressure/hazelcast-backpressure.xml -cp hazelcast-enterprise-<SERVER VERSION>.jar:hazelcast-<SERVER VERSION>-tests.jar com.hazelcast.core.server.HazelcastMemberStarter    
```

- Run client test:

```bash
./build/examples/backpressure/enableBackPressure 
```
