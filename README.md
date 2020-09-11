## Mini-ssdp

这是一个基于 UDP 和自定义协议实现的简单服务发现程序，也可于 UDP 广播的发送和接收测试。具体的使用用法可通过 `./mini-server-for-linux -h`来查看使用帮助

**自定义协议**

协议格式:
> $[cmd]?[data]$

**说明**

1. 以 `$` 符号作为开头，用来区分普通消息和协议消息；同时以 `$` 作为结尾，可用来作为消息完整性的验证。
2. [cmd] 部分则是命令数据，比如用`discover`来表示搜索局域网中存在的服务，可自行设置;
3. 以 `?` 符号作为参数内容的开头，表示该条消息中附带了具体的数据信息，如果不需要传递额外的数据内容，则不需要带上该符号;
4. [data] 部分则是传递的具体的数据内容，如果存在 `?` 符号，则后面必须携带这部分的内容。该部分的数据内容可以自定义，直接写入到 `ssdp.service`文件中即可，客户端在接收到消息后，自行解析该部分的内容即可。

**特点**

该自定义协议是在使用全Byte数据协议和标准完整SSDP协议之间取舍之后产生的，全Byte数据协议是通过字节位来传递一些命令或参数值，这种方式可以有效减少传输的数据量，但同时也会让开发者在不看协议文档的情况下完全看不懂数据的内容；而标准完整的SSDP协议则更全面，包含的信息也很多，比如协议名称、协议版本号之类的，这使得开发者在不看协议文档的情况下也能明白传递的数据内容，但也因此传输的数据量比全Byte协议的数据量大了很多。

本方案中的自定义协议则是兼顾传输数据量小和数据内容友好，以及易扩展的特点，去掉了标准SSDP协议中无关紧要的数据内容，只保留必要的数据组成部分，因此使得传输的数据量减少了很多。

### Build for Android

**1. NDK 下载**

从 https://developer.android.google.cn/ndk/downloads/index.html 地址下载对应自己系统的NDK包，下载完后解压。

**2. 生成编译工具链**

进入到解压后的文件夹的 `build/tools/` 目录中, 该目录下有两个用于构建gcc工具链的脚本：

```
make-standalone-toolchain.sh
make_standalone_tootlchain.py
```

这里使用python版本的脚本，按照官方的说明，shell版本的脚本将被淘汰，输入以下命令:

```bash
./make_standalone_toolchain.py --arch arm64 --api 21 --install-dir ~/android-build
```

参数说明:

 --arch : 交叉编译的目标平台，上述例子中只编译 arm64 的版本.

 --api  : 使用编译的 Android 系统版本，上述例子中是21，对应 Android 5.0, 请注意自己需要的系统版本.
 
 --install-dir : 生成的交叉编译工具链输出位置，可自行设置.

**3. 生成可执行文件**

生成好交叉编译工具链后，进入到该目录下的 `bin` 目录，比如按上述的命令生成的编译链，则进入到 `~/android-build/bin/` 目录中，也可先将该目录写入到环境变量中，环境变量因系统而异，具体怎么写入环境变量请自行查询。

进入到bin目录后，该目录下存在各种工具，根据你需要运行的 Android 版本和架构来决定使用哪个工具进行编译，比如 [release](https://github.com/zhongzilu/mini-ssdp/releases) 中编译的 Android 版本，则是依据 Android 5.1.1，armv7a架构进行编译的，编译命令如下:

```bash
./armv7a-linux-androideabi22-clang -pie -fPIE mini-server-for-linux.c -o mini-server-for-android-armv7a
```

如果你是使用 C++ 编写的源程序，则使用以 `++` 为结尾的编译工具即可，命令一样，最终生成的可执行文件为 `mini-server-for-android-armv7a`。

**4. 放入Android设备运行**

开启 Android 的开发者选项，开启USB调试功能，使用ADB工具将生成的可执行文件推送到设备中。

```bash
$ ./adb push [可执行文件的路径] /data
$ ./adb shell
$ cd data
$ ./mini-server-for-android-armv7a
```