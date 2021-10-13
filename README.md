# qemu_stm32_rtos_example

在qemu上的stm32芯片上写的一个rtos例子。

## 工具依赖

编译：arm-none-eabi-gcc arm-none-eabi-newlib

使用各linux发行版对应的包管理器安装

仿真：qemu-system-gnuarmeclipse

qemu官方没有支持stm32，[qemu-system-gnuarmeclipse](https://xpack.github.io/qemu-arm/) 是xpack社区开发的支持部分stm32型号的版本，基本用法和qemu类似。

官方推荐的方法是使用xpm安装，我建议直接手动[下载二进制包](https://github.com/xpack-dev-tools/qemu-arm-xpack/releases/) 安装到/opt并添加路径到PATH环境变量

```shell
cd /opt
sudo tar xvf ~/Downloads/xpack-qemu-arm-2.8.0-12-linux-x64.tar.gz
# Need relogin
echo "export PATH=\$PATH:/opt/xpack-qemu-arm-2.8.0-12/bin" | sudo tee -a /etc/profile
```

### 集成开发环境clion（可选）

我使用clion测试和开发这个项目，.run目录中的配置是clion的运行配置，已经配置好了gdb调试的运行项。若使用其他工具或命令行，请参考start_qemu.sh等脚本自行配置。

