/*
* Copyright (c) 2008-2013, Hazelcast, Inc. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

import com.hazelcast.config.Config;
import com.hazelcast.config.SerializerConfig;
import com.hazelcast.config.XmlConfigBuilder;
import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.nio.ObjectDataInput;
import com.hazelcast.nio.ObjectDataOutput;
import com.hazelcast.nio.serialization.DataSerializableFactory;
import com.hazelcast.nio.serialization.IdentifiedDataSerializable;
import com.hazelcast.nio.serialization.Portable;
import com.hazelcast.nio.serialization.PortableFactory;
import com.hazelcast.nio.serialization.PortableReader;
import com.hazelcast.nio.serialization.PortableWriter;
import com.hazelcast.nio.serialization.StreamSerializer;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicInteger;

/**
* User: sancar
* Date: 8/26/13
* Time: 1:51 PM
*/

class Person {
    private String name;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }
}

class Employee implements Portable {
    private String name;
    private int age;

    public Employee() {

    }

    public Employee(int age, String name) {
        this.age = age;
        this.name = name;
    }

    public int getFactoryId() {
        return 666;
    }

    public int getClassId() {
        return 2;
    }

    public void writePortable(PortableWriter writer) throws IOException {
        writer.writeUTF("n", name);
        writer.writeInt("a", age);
    }

    public void readPortable(PortableReader reader) throws IOException {
        name = reader.readUTF("n");
        age = reader.readInt("a");
    }

}

class SampleFailingTask implements Callable, IdentifiedDataSerializable {

    public SampleFailingTask() {
    }

    public int getFactoryId() {
        return 666;
    }

    public int getId() {
        return 1;
    }

    public String call() throws Exception {
        throw new IllegalStateException();
    }

    public void writeData(ObjectDataOutput out) throws IOException {
    }

    public void readData(ObjectDataInput in) throws IOException {
    }
}

class SampleRunnableTask implements Portable, Runnable {
    private String name;

    public SampleRunnableTask() {

    }

    public void run() {
        System.out.println("Running " + name);
    }

    public int getFactoryId() {
        return 666;
    }

    public int getClassId() {
        return 1;
    }

    public void writePortable(PortableWriter writer) throws IOException {
        writer.writeUTF("n", name);
    }

    public void readPortable(PortableReader reader) throws IOException {
        name = reader.readUTF("n");
    }
}

class SampleCallableTask implements IdentifiedDataSerializable, Callable {
    private String param;

    public SampleCallableTask() {

    }

    public Object call() throws Exception {
        return param + ":result";
    }

    public int getFactoryId() {
        return 666;
    }

    public int getId() {
        return 2;
    }

    public void writeData(ObjectDataOutput out) throws IOException {
        out.writeUTF(param);
    }

    public void readData(ObjectDataInput in) throws IOException {
        param = in.readUTF();
    }
}

public class CppClientListener {
    static final int OK = 5678;
    static final int END = 1;
    static final int START = 2;
    static final int SHUTDOWN = 3;
    static final int SHUTDOWN_ALL = 4;

    public static void main(String args[]) throws IOException {
        final Map<Integer, HazelcastInstance> map = new HashMap<Integer, HazelcastInstance>();
        final Config config = prepareConfig();
        final AtomicInteger atomicInteger = new AtomicInteger(0);
        final ServerSocket welcomeSocket = new ServerSocket(6543);
        System.out.println(welcomeSocket.getLocalSocketAddress());
        final Socket socket = welcomeSocket.accept();
        final DataInputStream dataInputStream = new DataInputStream(socket.getInputStream());
        final DataOutputStream dataOutputStream = new DataOutputStream(socket.getOutputStream());
        while (true) {
            final int command = dataInputStream.readInt();
            switch (command) {
                case START:
                    System.out.println("NEW INSTANCE OPEN ");
                    final int id = atomicInteger.incrementAndGet();
                    map.put(id, getInstance(config));
                    dataOutputStream.writeInt(id);
                    break;
                case SHUTDOWN:
                    final int id2 = dataInputStream.readInt();
                    final HazelcastInstance instance = map.get(id2);
                    if (instance == null) {
                        dataOutputStream.writeInt(OK);
                        continue;
                    }
                    instance.getLifecycleService().shutdown();
                    dataOutputStream.writeInt(OK);
                    break;
                case SHUTDOWN_ALL:
                    Hazelcast.shutdownAll();
                    map.clear();
                    dataOutputStream.writeInt(OK);
                    break;
                case END:
                    System.exit(0);
                    break;
            }
        }
    }

    private static Config prepareConfig() throws FileNotFoundException {
        final Config config = new XmlConfigBuilder("/Users/msk/IdeaProjects/testEnv/src/main/resources/hazelcast.xml").build();
        config.getSerializationConfig().addPortableFactory(666, new PortableFactory() {
            public Portable create(int classId) {
                if (classId == 1) {
                    return new SampleRunnableTask();
                } else if (classId == 2) {
                    return new Employee();
                }
                return null;
            }
        });
        config.getSerializationConfig().addDataSerializableFactory(666, new DataSerializableFactory() {
            public IdentifiedDataSerializable create(int typeId) {
                if (typeId == 1) {
                    return new SampleFailingTask();
                } else if (typeId == 2) {
                    return new SampleCallableTask();
                }
                return null;
            }
        });

        final SerializerConfig serializerConfig = new SerializerConfig();
        serializerConfig.setImplementation(new StreamSerializer<Person>() {
            @Override
            public int getTypeId() {
                return 999;
            }

            public void destroy() {

            }

            public void write(ObjectDataOutput out, Person object) throws IOException {
                out.writeInt(999);
                out.writeUTF(object.getName());
                out.writeInt(999);
            }

            public Person read(ObjectDataInput in) throws IOException {
                if (in.readInt() != 999) {
                    throw new IOException(" wrong value is read expected 999 ");
                }
                final Person person = new Person();
                person.setName(in.readUTF());
                if (in.readInt() != 999) {
                    throw new IOException(" wrong value is read expected 999 ");
                }
                return person;
            }
        });
        serializerConfig.setTypeClass(Person.class);
        config.getSerializationConfig().addSerializerConfig(serializerConfig);
        return config;
    }

    private static HazelcastInstance getInstance(Config config) {
        final HazelcastInstance instance = Hazelcast.newHazelcastInstance(config);
        instance.getCluster().getLocalMember().setIntAttribute("intAttr", 211);
        instance.getCluster().getLocalMember().setBooleanAttribute("boolAttr", true);
        instance.getCluster().getLocalMember().setByteAttribute("byteAttr", (byte) 7);
        instance.getCluster().getLocalMember().setDoubleAttribute("doubleAttr", 2);
        instance.getCluster().getLocalMember().setFloatAttribute("floatAttr", 1.2f);
        instance.getCluster().getLocalMember().setShortAttribute("shortAttr", (short) 3);
        instance.getCluster().getLocalMember().setStringAttribute("strAttr", "strAttr");
        return instance;
    }
}
