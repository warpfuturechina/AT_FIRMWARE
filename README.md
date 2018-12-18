# 烧录固件
---

### 1. 准备

git clone 项目，解压<strong>at_firmware_build.zip</strong>

解压后可获得以下文件:

* bootloader.bin
* flash.sh
* partitions_two_ota.bin
* wf_at_firmware.bin

### 2. 修改烧录参数

打开`flash.sh`文件，如下所示：

`python2 $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp8266 --port /dev/ttyUSB0 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 8MB 0x0000 bootloader.bin 0x10000 wf_at_firmware.bin 0x8000 partitions_two_ota.bin`

修改串口：

根据模块当前所在环境，获取串口并修改 `--port /dev/ttyUSB0`

<br>

> 查看模块串口，可以在命令行工具输入:
> <br>
> `cd /dev`
> <br>
> `ls -a`

查看模块串口，假设模块串口为`tty.SLAB_USBtoUART`, 将 `flash.sh` 修改为：

`python2 $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp8266 --port /dev/tty.SLAB_USBtoUART --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 8MB 0x0000 bootloader.bin 0x10000 wf_at_firmware.bin 0x8000 partitions_two_ota.bin`

保持并退出。

### 3. 烧录固件

先切换并进入到`at_firmware_build`所在文件夹，即`flash.sh`文件所在目录

> cd xxx/sdk/at_firmware_build

执行烧录脚本

> ./flash.sh

即完成模块烧录工作。
