# Table of Contents

* [Soak Test Description](#soak-test-description)
* [Success Criteria](#success-criteria)

# SoakTest Test Description

The test setup at Hazelcast lab environment is:

1. Use 4 member cluster with 1 JVM per VM 
2. The client test program exercises the following API calls from 32 threads in a random manner in a busy loop (See [SoakTest.cpp](./SoakTest.cpp)):
    + Put/Gets (number of keys: 1000, value size of 1 byte)
    + Predicates
    + MapListeners
    + EntryProcessors
    <p>The test code captures any exception and logs to a file. 
    
3. Run 10 clients on one lab machine and this machine only runs clients (i.e. no server at this machine)

4. Run the tests for 24 hours. Verify that: 
    + Make sure that all the client processes are up and running before killing the clients after 24 hours.
    + Analyse the output file: Make sure that there are no exceptions printed.
    
# Success Criteria
1. No exceptions printed.
2. All client processes are up and running after 48 hours with no problem.