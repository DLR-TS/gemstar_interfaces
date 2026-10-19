FROM ghcr.io/epicgames/unreal-engine:dev-5.7.4

#build protobuf for gemstar_interfaces
RUN sudo apt update\
&& sudo apt install wget cmake ninja-build -y\
&& wget https://github.com/protocolbuffers/protobuf/releases/download/v21.12/protobuf-all-21.12.tar.gz\
&& tar -xzf protobuf-all-21.12.tar.gz

RUN cd protobuf-21.12/ \
&& export TOOLCHAIN=/home/ue4/UnrealEngine/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v26_clang-20.1.8-rockylinux8/x86_64-unknown-linux-gnu \
&& export CC=$TOOLCHAIN/bin/clang \
&& export CXX=$TOOLCHAIN/bin/clang++ \
&& export PATH=$TOOLCHAIN/bin:$PATH \
&& export CXXFLAGS="-O2 -stdlib=libc++" \
&& export LDFLAGS="-stdlib=libc++" \
&& export LIBS="-lc++abi" \
&& ./configure --disable-shared --enable-static --with-pic CC=$CC CXX=$CXX \
&& make -j$(nproc) \
&& make install prefix=$HOME/thirdparty/protobuf \
&& sudo ldconfig
# && cd ..
# && rm -rf protobuf-21.12 protobuf-all-21.12.tar.gz

#build osi for gemstar_interfaces
RUN git clone https://github.com/OpenSimulationInterface/osi-cpp.git \
&& cd osi-cpp \
&& git checkout v3.8.0 \
&& git submodule update --init

RUN cd osi-cpp \
&& export TOOLCHAIN=/home/ue4/UnrealEngine/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v26_clang-20.1.8-rockylinux8/x86_64-unknown-linux-gnu \
&& export CC=$TOOLCHAIN/bin/clang \
&& export CXX=$TOOLCHAIN/bin/clang++ \
&& export PATH=$TOOLCHAIN/bin:$PATH \
&& export CXXFLAGS="-O2 -stdlib=libc++" \
&& export LDFLAGS="-stdlib=libc++" \
&& export LIBS="-lc++abi" \
&& cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DProtobuf_INCLUDE_DIR=$HOME/thirdparty/protobuf/include \
  -DProtobuf_LIBRARY=$HOME/thirdparty/protobuf/lib/libprotobuf.a \
  -DProtobuf_PROTOC_EXECUTABLE=$HOME/thirdparty/protobuf/bin/protoc \
&& ninja -C build -j$(nproc)
