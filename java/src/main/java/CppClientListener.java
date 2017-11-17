/*
* Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
import com.hazelcast.config.RingbufferConfig;
import com.hazelcast.config.SSLConfig;
import com.hazelcast.config.SerializerConfig;
import com.hazelcast.config.XmlConfigBuilder;
import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.logging.ILogger;
import com.hazelcast.logging.Logger;
import com.hazelcast.map.EntryBackupProcessor;
import com.hazelcast.map.EntryProcessor;
import com.hazelcast.map.MapInterceptor;
import com.hazelcast.nio.ObjectDataInput;
import com.hazelcast.nio.ObjectDataOutput;
import com.hazelcast.nio.serialization.DataSerializableFactory;
import com.hazelcast.nio.serialization.IdentifiedDataSerializable;
import com.hazelcast.nio.serialization.Portable;
import com.hazelcast.nio.serialization.PortableFactory;
import com.hazelcast.nio.serialization.PortableReader;
import com.hazelcast.nio.serialization.PortableWriter;
import com.hazelcast.nio.serialization.StreamSerializer;
import com.hazelcast.nio.ssl.TestKeyStoreUtil;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicInteger;

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

    // add all possible types
    byte by = 2;
    boolean bool = true;
    char c = 'c';
    short s = 4;
    int i = 2000;
    long l = 321324141;
    float f = 3.14f;
    double d = 3.14334;
    String str = "Hello world";
    String utfStr = "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";

    byte byteArray[] = {50, 100, (byte) 150, (byte) 200};
    char charArray[] = {'c', 'h', 'a', 'r'};
    boolean boolArray[] = {true, false, false, true};
    short shortArray[] = {3, 4, 5};
    int integerArray[] = {9, 8, 7, 6};
    long longArray[] = {0, 1, 5, 7, 9, 11};
    float floatArray[] = {0.6543f, -3.56f, 45.67f};
    double doubleArray[] = {456.456, 789.789, 321.321};

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

        writer.writeByte("b", by);
        writer.writeChar("c", c);
        writer.writeBoolean("bo", bool);
        writer.writeShort("s", s);
        writer.writeInt("i", i);
        writer.writeLong("l", l);
        writer.writeFloat("f", f);
        writer.writeDouble("d", d);
        writer.writeUTF("str", str);
        writer.writeUTF("utfstr", utfStr);

        writer.writeByteArray("bb", byteArray);
        writer.writeCharArray("cc", charArray);
        writer.writeBooleanArray("ba", boolArray);
        writer.writeShortArray("ss", shortArray);
        writer.writeIntArray("ii", integerArray);
        writer.writeFloatArray("ff", floatArray);
        writer.writeDoubleArray("dd", doubleArray);

        ObjectDataOutput out = writer.getRawDataOutput();
        out.writeObject(by);
        out.writeObject(c);
        out.writeObject(bool);
        out.writeObject(s);
        out.writeObject(i);
        out.writeObject(f);
        out.writeObject(d);
        out.writeObject(str);
        out.writeObject(utfStr);
    }

    public void readPortable(PortableReader reader) throws IOException {
        name = reader.readUTF("n");
        age = reader.readInt("a");

        by = reader.readByte("b");;
        c = reader.readChar("c");;
        bool = reader.readBoolean("bo");;
        s = reader.readShort("s");;
        i = reader.readInt("i");;
        l = reader.readLong("l");;
        f = reader.readFloat("f");;
        d = reader.readDouble("d");;
        str = reader.readUTF("str");;
        utfStr = reader.readUTF("utfstr");;

        byteArray = reader.readByteArray("bb");;
        charArray = reader.readCharArray("cc");;
        boolArray = reader.readBooleanArray("ba");;
        shortArray = reader.readShortArray("ss");;
        integerArray = reader.readIntArray("ii");;
        floatArray = reader.readFloatArray("ff");;
        doubleArray = reader.readDoubleArray("dd");;

        ObjectDataInput in = reader.getRawDataInput();
        by = in.readObject();
        c = in.readObject();
        bool = in.readObject();
        s = in.readObject();
        i = in.readObject();
        f = in.readObject();
        d = in.readObject();
        str = in.readObject();
        utfStr = in.readObject();
    }

    public int getAge() {
        return age;
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

class KeyMultiplier implements IdentifiedDataSerializable, EntryProcessor<Integer, Employee> {
    private int multiplier;

    @Override
    public int getFactoryId() {
        return 666;
    }

    @Override
    public int getId() {
        return 3;
    }

    @Override
    public void writeData(ObjectDataOutput out)
            throws IOException {
        out.writeInt(multiplier);
    }

    @Override
    public void readData(ObjectDataInput in)
            throws IOException {
        multiplier = in.readInt();
    }

    @Override
    public Object process(Map.Entry<Integer, Employee> entry) {
        if (null == entry.getValue()) {
            return -1;
        }
        return multiplier * entry.getKey();
    }

    @Override
    public EntryBackupProcessor<Integer, Employee> getBackupProcessor() {
        return null;
    }
}

class WaitMultiplierProcessor
        implements IdentifiedDataSerializable, EntryProcessor<Integer, Employee> {
    private int waiTimeInMillis;
    private int multiplier;

    @Override
    public int getFactoryId() {
        return 666;
    }

    @Override
    public int getId() {
        return 8;
    }

    @Override
    public void writeData(ObjectDataOutput out)
            throws IOException {
        out.writeInt(waiTimeInMillis);
        out.writeInt(multiplier);
    }

    @Override
    public void readData(ObjectDataInput in)
            throws IOException {
        waiTimeInMillis = in.readInt();
        multiplier = in.readInt();
    }

    @Override
    public Object process(Map.Entry<Integer, Employee> entry) {
        try {
            Thread.sleep(waiTimeInMillis);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        if (null == entry.getValue()) {
            return -1;
        }
        return multiplier * entry.getKey();
    }

    @Override
    public EntryBackupProcessor<Integer, Employee> getBackupProcessor() {
        return null;
    }
}

class KeyMultiplierWithNullableResult extends KeyMultiplier {
    @Override
    public int getFactoryId() {
        return 666;
    }

    @Override
    public int getId() {
        return 7;
    }

    @Override
    public Object process(Map.Entry<Integer, Employee> entry) {
        if (null == entry.getValue()) {
            return null;
        }
        return super.process(entry);
    }
}

class PartitionAwareInt implements IdentifiedDataSerializable {
    private int value;

    @Override
    public int getFactoryId() {
        return 666;
    }

    @Override
    public int getId() {
        return 9;
    }

    @Override
    public void writeData(ObjectDataOutput objectDataOutput)
            throws IOException {
        objectDataOutput.writeInt(value);
    }

    @Override
    public void readData(ObjectDataInput objectDataInput)
            throws IOException {
        value = objectDataInput.readInt();
    }
}

/**
 * Compares based on the employee age
 */
class EmployeeEntryComparator implements IdentifiedDataSerializable, Comparator<Map.Entry<Integer, Employee>> {
    private int multiplier;

    @Override
    public int getFactoryId() {
        return 666;
    }

    @Override
    public int getId() {
        return 4;
    }

    @Override
    public void writeData(ObjectDataOutput out)
            throws IOException {
    }

    @Override
    public void readData(ObjectDataInput in)
            throws IOException {
    }

    @Override
    public int compare(Map.Entry<Integer, Employee> lhs, Map.Entry<Integer, Employee> rhs) {
        Employee lv;
        Employee rv;
        try {
            lv = lhs.getValue();
            rv = rhs.getValue();
        } catch (ClassCastException e) {
            return -1;
        }

        if (null == lv && null == rv) {
            // order by key
            int leftKey = lhs.getKey();
            int rightKey = rhs.getKey();

            if (leftKey == rightKey) {
                return 0;
            }

            if (leftKey < rightKey) {
                return -1;
            }

            return 1;
        }

        if (null == lv) {
            return -1;
        }

        if (null == rv) {
            return 1;
        }

        Integer la = lv.getAge();
        Integer ra = rv.getAge();

        return la.compareTo(ra);
    }
}

class EmployeeEntryKeyComparator extends EmployeeEntryComparator {
    @Override
    public int getId() {
        return 5;
    }

    @Override
    public int compare(Map.Entry<Integer, Employee> lhs, Map.Entry<Integer, Employee> rhs) {
        Integer key1 = lhs.getKey();
        Integer key2 = rhs.getKey();

        if (null == key1) {
            return -1;
        }

        if (null == key2) {
            return 1;
        }

        if (key1 == key2) {
            return 0;
        }

        if (key1 < key2) {
            return -1;
        }

        return 1;
    }
}

class UTFValueValidatorProcessor
        implements EntryProcessor<String, String>, IdentifiedDataSerializable {
    @Override
    public Object process(Map.Entry<String, String> entry) {
        return entry.getKey().equals("myutfkey") && entry.getValue().equals("xyzä123 イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム");
    }

    @Override
    public EntryBackupProcessor<String, String> getBackupProcessor() {
        return null;
    }

    @Override
    public int getFactoryId() {
        return 666;
    }

    @Override
    public int getId() {
        return 9;
    }

    @Override
    public void writeData(ObjectDataOutput objectDataOutput)
            throws IOException {
    }

    @Override
    public void readData(ObjectDataInput objectDataInput)
            throws IOException {
    }
}

class MapGetInterceptor implements MapInterceptor, IdentifiedDataSerializable {
    private String prefix;

    @Override
    public Object interceptGet(Object value) {
        if (null == value) {
            return prefix;
        }

        String val = (String)value;
        return prefix + val;
    }

    @Override
    public void afterGet(Object value) {
    }

    @Override
    public Object interceptPut(Object oldValue, Object newValue) {
        return null;
    }

    @Override
    public void afterPut(Object value) {
    }

    @Override
    public Object interceptRemove(Object removedValue) {
        return null;
    }

    @Override
    public void afterRemove(Object value) {
    }

    @Override
    public int getFactoryId() {
        return 666;
    }

    @Override
    public int getId() {
        return 6;
    }

    @Override
    public void writeData(ObjectDataOutput out)
            throws IOException {
            out.writeUTF(prefix);
    }

    @Override
    public void readData(ObjectDataInput in)
            throws IOException {
        prefix = in.readUTF();
    }
}

class BaseCustom {
    private int value;

    public BaseCustom() {}

    public BaseCustom(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    public void setValue(int value) {
        this.value = value;
    }
};

class Derived1Custom extends BaseCustom {
    public Derived1Custom() {
    }

    public Derived1Custom(int value) {
        super(value);
    }
}

class Derived2Custom extends Derived1Custom {
    public Derived2Custom() {
    }

    public Derived2Custom(int value) {
        super(value);
    }
}

class BaseDataSerializable implements IdentifiedDataSerializable {
    @Override
    public int getFactoryId() {
        return 666;
    }

    @Override
    public int getId() {
        return 10;
    }

    @Override
    public void writeData(ObjectDataOutput objectDataOutput)
            throws IOException {
    }

    @Override
    public void readData(ObjectDataInput objectDataInput)
            throws IOException {
    }
}

class Derived1DataSerializable extends BaseDataSerializable {
    @Override
    public int getId() {
        return 11;
    }
}

class Derived2DataSerializable extends Derived1DataSerializable {
    @Override
    public int getId() {
        return 12;
    }
}

class BasePortable implements Portable  {
    @Override
    public int getFactoryId() {
        return 666;
    }

    @Override
    public int getClassId() {
        return 3;
    }

    @Override
    public void writePortable(PortableWriter portableWriter)
            throws IOException {
    }

    @Override
    public void readPortable(PortableReader portableReader)
            throws IOException {
    }
}

class Derived1Portable extends BasePortable {
    @Override
    public int getClassId() {
        return 4;
    }
}

class Derived2Portable extends BasePortable {
    @Override
    public int getClassId() {
        return 5;
    }
}

public class CppClientListener {

    static final int OK = 5678;
    static final int FAIL = -1;
    static final int END = 1;
    static final int START = 2;
    static final int SHUTDOWN = 3;
    static final int SHUTDOWN_ALL = 4;
    static final int START_SSL = 5;

    public static int CAPACITY = 10;

    public static void main(String args[]) throws IOException {
        final Map<Integer, HazelcastInstance> map = new HashMap<Integer, HazelcastInstance>();
        final Config config = prepareConfig();
        System.setProperty("hazelcast.enterprise.license.key", System.getenv("HAZELCAST_ENTERPRISE_KEY"));
        final AtomicInteger atomicInteger = new AtomicInteger(0);
        final ServerSocket welcomeSocket = new ServerSocket(6543);
        System.out.println(welcomeSocket.getLocalSocketAddress());
        final Socket socket = welcomeSocket.accept();
        final DataInputStream dataInputStream = new DataInputStream(socket.getInputStream());
        final DataOutputStream dataOutputStream = new DataOutputStream(socket.getOutputStream());

        ILogger logger = Logger.getLogger("CppClientListener");
        while (true) {
            final int command = dataInputStream.readInt();
            switch (command) {
                case START:
                    System.out.println("START command received: NEW INSTANCE OPEN ");
                    try {
                        final int id = atomicInteger.incrementAndGet();
                        map.put(id, getInstance(config));
                        dataOutputStream.writeInt(id);
                    } catch (Exception e) {
                        logger.warning("START command failed. Error:" + e);
                        dataOutputStream.writeInt(FAIL);
                    }
                    break;
                case START_SSL:
                    System.out.println("START_SSL command received: NEW INSTANCE OPEN ");
                    try {
                        final int id = atomicInteger.incrementAndGet();
                        map.put(id, getInstance(getSSLConfig()));
                        dataOutputStream.writeInt(id);
                    } catch (Exception e) {
                        logger.warning("START_SSL command failed. Error:" + e);
                        dataOutputStream.writeInt(FAIL);
                    }
                    break;
                case SHUTDOWN:
                    logger.info("SHUTDOWN command received");
                    int id2 = -1;
                    try {
                        id2 = dataInputStream.readInt();

                        logger.info("SHUTDOWN command for instance " + id2);

                        final HazelcastInstance instance = map.get(id2);
                        if (instance == null) {
                            dataOutputStream.writeInt(OK);
                            continue;
                        }
                        instance.getLifecycleService().shutdown();
                        dataOutputStream.writeInt(OK);

                        logger.info("SHUTDOWN for instance " + id2 + " is completed.");
                    } catch (Exception e) {
                        logger.warning("SHUTDOWN failed for instance " + id2 + ". Error:" + e);
                        dataOutputStream.writeInt(FAIL);
                    }
                    break;
                case SHUTDOWN_ALL:
                    logger.info("SHUTDOWN_ALL command received");
                    try {
                        Hazelcast.shutdownAll();
                        map.clear();
                        dataOutputStream.writeInt(OK);
                    } catch (Exception e) {
                        logger.warning("SHUTDOWN_ALL command failed. Error:" + e);
                        dataOutputStream.writeInt(FAIL);
                    }

                    break;
                case END:
                    System.exit(0);
                    break;
            }
        }
    }

    private static Config getSSLConfig()
            throws IOException {
        Config config = prepareConfig();
        Properties props = TestKeyStoreUtil.createSslProperties();
        config.getNetworkConfig().setSSLConfig(new SSLConfig().setEnabled(true).setProperties(props));
        return config;
    }

    private static Config prepareConfig()
            throws IOException {
        final Config config = new XmlConfigBuilder().build();

        config.getSerializationConfig().addPortableFactory(666, new PortableFactory() {
            public Portable create(int classId) {
                if (classId == 1) {
                    return new SampleRunnableTask();
                } else if (classId == 2) {
                    return new Employee();
                } else if (classId == 3) {
                    return new BasePortable();
                } else if (classId == 4) {
                    return new Derived1Portable();
                } else if (classId == 5) {
                    return new Derived2Portable();
                }
                return null;
            }
        });
        config.getSerializationConfig().addDataSerializableFactory(666, new DataSerializableFactory() {
            public IdentifiedDataSerializable create(int typeId) {
                switch(typeId) {
                    case 1:
                        return new SampleFailingTask();
                    case 2:
                        return new SampleCallableTask();
                    case 3:
                        return new KeyMultiplier();
                    case 4:
                        return new EmployeeEntryComparator();
                    case 5:
                        return new EmployeeEntryKeyComparator();
                    case 6:
                        return new MapGetInterceptor();
                    case 7:
                        return new KeyMultiplierWithNullableResult();
                    case 8:
                        return new WaitMultiplierProcessor();
                    case 9:
                        return new UTFValueValidatorProcessor();
                    case 10:
                        return new BaseDataSerializable();
                    case 11:
                        return new Derived1DataSerializable();
                    case 12:
                        return new Derived2DataSerializable();
                    default:
                        return null;
                }
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

            public void write(ObjectDataOutput out, Person object)
                    throws IOException {
                out.writeInt(999);
                out.writeUTF(object.getName());
                out.writeInt(999);
            }

            public Person read(ObjectDataInput in)
                    throws IOException {
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

        final SerializerConfig baseCustomSerializerConfig = new SerializerConfig();
        baseCustomSerializerConfig.setImplementation(new StreamSerializer<BaseCustom>() {
            @Override
            public int getTypeId() {
                return 3;
            }

            public void destroy() {
            }

            @Override
            public void write(ObjectDataOutput objectDataOutput, BaseCustom baseCustom) throws IOException {
                objectDataOutput.writeInt(baseCustom.getValue());
            }

            public BaseCustom read(ObjectDataInput in) throws IOException {
                return new BaseCustom(in.readInt());
            }
        });
        baseCustomSerializerConfig.setTypeClass(BaseCustom.class);
        config.getSerializationConfig().addSerializerConfig(baseCustomSerializerConfig);

        final SerializerConfig derived1CustomSerializerConfig = new SerializerConfig();
        derived1CustomSerializerConfig.setImplementation(new StreamSerializer<Derived1Custom>() {
            @Override
            public int getTypeId() {
                return 4;
            }

            public void destroy() {
            }

            @Override
            public void write(ObjectDataOutput objectDataOutput, Derived1Custom object) throws IOException {
                objectDataOutput.writeInt(object.getValue());
            }

            public Derived1Custom read(ObjectDataInput in) throws IOException {
                return new Derived1Custom(in.readInt());
            }
        });
        derived1CustomSerializerConfig.setTypeClass(Derived1Custom.class);
        config.getSerializationConfig().addSerializerConfig(derived1CustomSerializerConfig);

        final SerializerConfig derived2CustomSerializerConfig = new SerializerConfig();
        derived2CustomSerializerConfig.setImplementation(new StreamSerializer<Derived2Custom>() {
            @Override
            public int getTypeId() {
                return 5;
            }

            public void destroy() {
            }

            @Override
            public void write(ObjectDataOutput objectDataOutput, Derived2Custom object) throws IOException {
                objectDataOutput.writeInt(object.getValue());
            }

            public Derived2Custom read(ObjectDataInput in) throws IOException {
                return new Derived2Custom(in.readInt());
            }
        });
        derived2CustomSerializerConfig.setTypeClass(Derived2Custom.class);
        config.getSerializationConfig().addSerializerConfig(derived2CustomSerializerConfig);

        config.addRingBufferConfig(new RingbufferConfig("rb*").setCapacity(CAPACITY));

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
