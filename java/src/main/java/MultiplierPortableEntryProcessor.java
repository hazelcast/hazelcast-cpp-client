import com.hazelcast.map.EntryBackupProcessor;
import com.hazelcast.map.EntryProcessor;
import com.hazelcast.nio.serialization.Portable;
import com.hazelcast.nio.serialization.PortableReader;
import com.hazelcast.nio.serialization.PortableWriter;

import java.io.IOException;
import java.util.Map;

/**
 * Created by ihsan on 27/10/15.
 */
public class MultiplierPortableEntryProcessor
        implements EntryProcessor<String, Integer>, Portable {
    public final int FACTORY_ID = 1;
    public final int CLASS_ID = 16;

    @Override
    public Object process(Map.Entry<String, Integer> entry) {
        return entry.getValue() * multiplier;
    }

    @Override
    public EntryBackupProcessor<String, Integer> getBackupProcessor() {
        return null;
    }

    private int multiplier;

    @Override
    public int getFactoryId() {
        return FACTORY_ID;
    }

    @Override
    public int getClassId() {
        return CLASS_ID;
    }

    @Override
    public void writePortable(PortableWriter portableWriter)
            throws IOException {
        portableWriter.writeInt("m", multiplier);
    }

    @Override
    public void readPortable(PortableReader portableReader)
            throws IOException {
        multiplier = portableReader.readInt("m");
    }
}
