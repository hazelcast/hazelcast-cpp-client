package map;

import com.hazelcast.config.Config;
import com.hazelcast.config.XmlConfigBuilder;
import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.map.AbstractEntryProcessor;
import com.hazelcast.nio.ObjectDataInput;
import com.hazelcast.nio.ObjectDataOutput;
import com.hazelcast.nio.serialization.DataSerializableFactory;
import com.hazelcast.nio.serialization.IdentifiedDataSerializable;

import java.io.IOException;
import java.util.Map;

class Employee implements IdentifiedDataSerializable {
    private int salary;

    public Employee () {

    }
    public Employee (int salary) {
        this.salary= salary;
    }

    public int getFactoryId()  {
        return 1;
    }

    public int getId()  {
        return 5;
    }

    public void writeData(ObjectDataOutput out) throws IOException {
        out.writeInt(salary);
    }

    public void readData(ObjectDataInput in) throws IOException {
        salary = in.readInt();
    }

    public void incSalary(int amount) {
        salary += amount;
    }

    public int getSalary() {
        return salary;
    }
}

class EmployeeRaiseEntryProcessor extends AbstractEntryProcessor<String, Employee> implements IdentifiedDataSerializable {
    @Override
    public Object process(Map.Entry<String, Employee> entry) {
        Employee value = entry.getValue();
        value.incSalary(10);
        entry.setValue(value);
        return value.getSalary();
    }

    public int getFactoryId()  {
        return 1;
    }

    public int getId()  {
        return 6;
    }

    public void writeData(ObjectDataOutput out) throws IOException {

    }

    public void readData(ObjectDataInput in) throws IOException {

    }
}

public class EntryProcessorEmployeeServer{

    public static void main(String args[]) {
        final Config config = new XmlConfigBuilder().build();
        config.getSerializationConfig().addDataSerializableFactory(1, new DataSerializableFactory() {
            @Override
            public IdentifiedDataSerializable create(int typeId) {
                if(typeId == 5)
                    return new Employee();
                else
                    return new EmployeeRaiseEntryProcessor();
            }
        });
        HazelcastInstance instance = Hazelcast.newHazelcastInstance(config);
    }
}
