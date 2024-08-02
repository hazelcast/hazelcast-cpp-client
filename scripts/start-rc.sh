#!/bin/bash

function version_greater_equal()
{
    printf '%s\n%s\n' "$2" "$1" | sort --check=quiet --version-sort
}

function downloadFromMaven()
{
  remoteRepository=$1
  artifact=$2
  echo "Downloading ${artifact} from remote repository: ${remoteRepository}"
  mvn -q dependency:get -Dtransitive=false -DremoteRepositories=${remoteRepository} -Dartifact=${artifact}
  mvn -q dependency:copy -Dartifact=${artifact} -DoutputDirectory=.
  if [ $? -ne 0 ]; then
    echo "Failed download to download ${artifact} from remote repository: ${remoteRepository}"
    exit 1
  else
    echo "Downloaded ${artifact} from remote repository: ${remoteRepository}"
  fi
}

function cleanup {
    echo "cleanup is being performed."
    if [ "x${rcPid}" != "x" ]
    then
        echo "Killing remote controller server with pid ${rcPid}"
        kill -9 ${rcPid} || true
    fi
    exit
}

# Disables printing security sensitive data to the logs
set +x

trap cleanup EXIT

HZ_VERSION=${HZ_VERSION:-5.5.0}
HAZELCAST_TEST_VERSION=${HZ_VERSION}
HAZELCAST_ENTERPRISE_VERSION=${HZ_VERSION}
HAZELCAST_RC_VERSION=0.8-SNAPSHOT
SNAPSHOT_REPO="https://oss.sonatype.org/content/repositories/snapshots"
RELEASE_REPO="http://repo1.maven.apache.org/maven2"
ENTERPRISE_RELEASE_REPO="https://repository.hazelcast.com/release/"
ENTERPRISE_SNAPSHOT_REPO="https://repository.hazelcast.com/snapshot/"

if [[ ${HZ_VERSION} == *-SNAPSHOT ]]
then
	REPO=${SNAPSHOT_REPO}
	ENTERPRISE_REPO=${ENTERPRISE_SNAPSHOT_REPO}
else
	REPO=${RELEASE_REPO}
	ENTERPRISE_REPO=${ENTERPRISE_RELEASE_REPO}
fi

if [ -f "hazelcast-remote-controller-${HAZELCAST_RC_VERSION}.jar" ]; then
    echo "remote controller already exist, not downloading from maven."
else
    downloadFromMaven ${SNAPSHOT_REPO} "com.hazelcast:hazelcast-remote-controller:${HAZELCAST_RC_VERSION}"
fi

if [ -f "hazelcast-${HAZELCAST_TEST_VERSION}-tests.jar" ]; then
    echo "hazelcast-test.jar already exists, not downloading from maven."
else
    downloadFromMaven ${SNAPSHOT_REPO} "com.hazelcast:hazelcast:${HAZELCAST_TEST_VERSION}:jar:tests"
fi

version_greater_equal ${HZ_VERSION} 4.2.0

if [[ $? -eq "0" ]]; then
    INCLUDE_SQL="1"
else
    INCLUDE_SQL="0"
fi

if [[ ${INCLUDE_SQL} -eq "1" ]]; then
    if [ -f "hazelcast-sql-${HZ_VERSION}.jar" ]; then
        echo "hazelcast-sql-${HZ_VERSION}.jar already exists, not downloading from maven."
    else
        downloadFromMaven ${SNAPSHOT_REPO} "com.hazelcast:hazelcast-sql:${HZ_VERSION}:jar"
    fi
fi

if [ -f "hazelcast-enterprise-${HAZELCAST_ENTERPRISE_VERSION}.jar" ]; then
echo "hazelcast-enterprise.jar already exists, not downloading from maven."
else
    downloadFromMaven ${ENTERPRISE_REPO} "com.hazelcast:hazelcast-enterprise:${HAZELCAST_ENTERPRISE_VERSION}:jar"
fi

if [ -f "hazelcast-enterprise-${HAZELCAST_ENTERPRISE_VERSION}-tests.jar" ]; then
echo "hazelcast-enterprise-tests.jar already exists, not downloading."
else
    echo "Downloading: hazelcast enterprise test jar ${HAZELCAST_ENTERPRISE_VERSION}"
    git clone git@github.com:hazelcast/private-test-artifacts.git

    if [ $? -eq 0 ]; then
        cd private-test-artifacts
        git checkout data
        cp certs.jar ../hazelcast-enterprise-${HAZELCAST_ENTERPRISE_VERSION}-tests.jar
        cd ..
        rm -rf private-test-artifacts
    else
        echo "Failed download hazelcast enterprise test jar hazelcast-enterprise-${HAZELCAST_ENTERPRISE_VERSION}-tests.jar"
        echo "Make sure you have access permission to 'github.com/hazelcast/private-test-artifacts repo'"
        echo "Make sure that you added you ssh-key to your github account."

        exit 1
    fi
fi

CLASSPATH="\
hazelcast-remote-controller-${HAZELCAST_RC_VERSION}.jar:\
hazelcast-enterprise-${HAZELCAST_ENTERPRISE_VERSION}.jar:\
hazelcast-enterprise-${HAZELCAST_ENTERPRISE_VERSION}-tests.jar:\
hazelcast-${HAZELCAST_TEST_VERSION}-tests.jar"

if [[ ${INCLUDE_SQL} -eq "1" ]]; then
    CLASSPATH=$CLASSPATH:\:hazelcast-sql-${HZ_VERSION}.jar
fi

# necessary arguments for Java 9+
JAVA_MAJOR_VERSION=$(java -version 2>&1 | head -n 1 | awk -F '"' '{print $2}' | awk -F '.' '{print $1}')
if [ "$JAVA_MAJOR_VERSION" != "1" ]; then
    MODULE_ARGUMENTS="--add-exports java.base/jdk.internal.ref=ALL-UNNAMED --add-opens java.base/java.nio=ALL-UNNAMED"
fi

echo "Starting Remote Controller ... enterprise ..."
java -cp ${CLASSPATH} \
     -Dhazelcast.enterprise.license.key=${HAZELCAST_ENTERPRISE_KEY} \
     -Dhazelcast.phone.home.enabled=false \
     $MODULE_ARGUMENTS \
     com.hazelcast.remotecontroller.Main --use-simple-server &
rcPid=$!
wait ${rcPid}
exit $?
