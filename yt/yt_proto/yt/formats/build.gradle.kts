import com.google.protobuf.gradle.*

val buildProtoDir = File("${buildDir}", "__proto__")

plugins {
    id("java-library")
    id("com.google.protobuf") version "0.8.19"
}


repositories {
    mavenCentral()
}

dependencies {
    api("com.google.protobuf:protobuf-java:3.21.12")

    protobuf(files(buildProtoDir))
}


val prepareProto = tasks.register<Copy>("prepareProto") {
    from(rootDir) {
        include("yt/yt_proto/yt/formats/extension.proto")
        include("yt/yt_proto/yt/formats/yamr.proto")
    }
    into(buildProtoDir)
}

afterEvaluate {
    tasks.getByName("extractProto").dependsOn(prepareProto)
}
