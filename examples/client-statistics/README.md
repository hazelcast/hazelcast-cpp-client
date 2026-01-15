# Client Statistics Example

- Start server:

```bash
java -jar hazelcast-<SERVER VERSION>.jar    
```

- Run client test:

```bash
./build/examples/client-statistics/clientStatistics 
```

- Management Center
- Start MC using the instructions at https://docs.hazelcast.com/management-center/latest/getting-started/install
- Open MC in your browser at http://localhost:8080
- Connect to the Hazelcast cluster
- Go to the "Clients" section and observe the client statistics being reported for the test client.
