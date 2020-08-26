@echo off
setlocal EnableDelayedExpansion

set HZ_VERSION=4.0
set HAZELCAST_TEST_VERSION=%HZ_VERSION%
set HAZELCAST_ENTERPRISE_VERSION=%HZ_VERSION%
set HAZELCAST_RC_VERSION="0.8-SNAPSHOT"
set SNAPSHOT_REPO=https://oss.sonatype.org/content/repositories/snapshots
set RELEASE_REPO=http://repo1.maven.apache.org/maven2
set ENTERPRISE_RELEASE_REPO=https://repository.hazelcast.com/release/
set ENTERPRISE_SNAPSHOT_REPO=https://repository.hazelcast.com/snapshot/

if not "x%HZ_VERSION:SNAPSHOT=%"=="x%HZ_VERSION%" (
    set REPO=%SNAPSHOT_REPO%
	set ENTERPRISE_REPO=%ENTERPRISE_SNAPSHOT_REPO%
) else (
	set REPO=%RELEASE_REPO%
	set ENTERPRISE_REPO=%ENTERPRISE_RELEASE_REPO%
)

echo Downlading jars for remote controller server

if exist "hazelcast-remote-controller-%HAZELCAST_RC_VERSION%.jar" (
    echo "hazelcast-remote-controller-%HAZELCAST_RC_VERSION%.jar already exist, not downloading from maven."
) else (
    echo "Downloading: remote-controller jar com.hazelcast:hazelcast-remote-controller:%HAZELCAST_RC_VERSION%"
    call mvn -q dependency:get -DrepoUrl=%SNAPSHOT_REPO% -Dartifact=com.hazelcast:hazelcast-remote-controller:%HAZELCAST_RC_VERSION% -Ddest=hazelcast-remote-controller-%HAZELCAST_RC_VERSION%.jar || (
        echo "Failed download remote-controller jar com.hazelcast:hazelcast-remote-controller:%HAZELCAST_RC_VERSION%" 
        exit /b 1
    )
)

if exist  "hazelcast-%HAZELCAST_TEST_VERSION%-tests.jar" (
    echo "hazelcast-%HAZELCAST_TEST_VERSION%-tests.jar already exists, not downloading from maven."
) else (
    echo "Downloading: hazelcast test jar com.hazelcast:hazelcast:%HAZELCAST_TEST_VERSION%:jar:tests"
    call mvn -q dependency:get -DrepoUrl=%SNAPSHOT_REPO% -Dartifact=com.hazelcast:hazelcast:%HAZELCAST_TEST_VERSION%:jar:tests -Ddest=hazelcast-%HAZELCAST_TEST_VERSION%-tests.jar || (
        echo "Failed download hazelcast test jar com.hazelcast:hazelcast:%HAZELCAST_TEST_VERSION%:jar:tests"
        exit /b 1
    )
)

if exist "hazelcast-enterprise-%HAZELCAST_ENTERPRISE_VERSION%.jar" (
    echo "hazelcast-enterprise-%HAZELCAST_ENTERPRISE_VERSION%.jar already exists, not downloading from maven."
) else (
    echo "Downloading: hazelcast enterprise jar com.hazelcast:hazelcast-enterprise:%HAZELCAST_ENTERPRISE_VERSION%"
    call mvn -q dependency:get -DrepoUrl=%ENTERPRISE_REPO% -Dartifact=com.hazelcast:hazelcast-enterprise:%HAZELCAST_ENTERPRISE_VERSION% -Ddest=hazelcast-enterprise-%HAZELCAST_ENTERPRISE_VERSION%.jar || (
        echo "Failed download hazelcast enterprise jar com.hazelcast:hazelcast-enterprise:%HAZELCAST_ENTERPRISE_VERSION%"
        exit /b 1
    )
)

set CLASSPATH="hazelcast-remote-controller-%HAZELCAST_RC_VERSION%.jar;hazelcast-enterprise-%HAZELCAST_ENTERPRISE_VERSION%.jar;hazelcast-%HAZELCAST_TEST_VERSION%-tests.jar"
echo "Starting Remote Controller ... enterprise ...Using classpath: %CLASSPATH%"

echo "Starting hazelcast-remote-controller"
start "hazelcast-remote-controller" /MIN cmd /c "java -Dhazelcast.enterprise.license.key=%HAZELCAST_ENTERPRISE_KEY% -Dhazelcast.phone.home.enabled=false -cp %CLASSPATH% com.hazelcast.remotecontroller.Main --use-simple-server"
