/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

import com.hazelcast.map.EntryBackupProcessor;
import com.hazelcast.map.EntryProcessor;
import com.hazelcast.nio.ObjectDataInput;
import com.hazelcast.nio.ObjectDataOutput;
import com.hazelcast.nio.serialization.IdentifiedDataSerializable;

import java.io.IOException;
import java.util.Map;

/**
 * Created by ihsan on 27/10/15.
 */
public class MultiplierDataSerializableEntryProcessor
        implements EntryProcessor<String, Integer>, IdentifiedDataSerializable {
    public final int FACTORY_ID = 1;
    public final int CLASS_ID = 15;

    private int multiplier;

    @Override
    public int getFactoryId() {
        return FACTORY_ID;
    }

    @Override
    public int getId() {
        return CLASS_ID;
    }

    @Override
    public void writeData(ObjectDataOutput objectDataOutput)
            throws IOException {
        objectDataOutput.writeInt(multiplier);
    }

    @Override
    public void readData(ObjectDataInput objectDataInput)
            throws IOException {
        multiplier = objectDataInput.readInt();
    }

    @Override
    public Object process(Map.Entry<String, Integer> entry) {
        return entry.getValue() * multiplier;
    }

    @Override
    public EntryBackupProcessor<String, Integer> getBackupProcessor() {
        return null;
    }
}
