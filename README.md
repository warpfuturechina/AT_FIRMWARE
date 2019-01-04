# 烧录固件
---

### 1. 准备

git clone 项目，解压<strong>at_firmware_build.zip</strong>

解压后可获得以下文件:

* flash.sh
* bootloader.bin
* partitions_two_ota.bin
* wf_at_firmware.bin

### 2. 修改烧录参数

<strong>打开`flash.sh`文件，如下：</strong>

`python2 $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp8266 --port /dev/ttyUSB0 --baud 9600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 2MB 0x0000 bootloader.bin 0x10000 wf_at_firmware.bin 0x8000 partitions_two_ota.bin`


<strong>串口</strong>:  `--port /dev/ttyUSB0`

<strong>波特率</strong>: `--baud 9600`

<strong>flash</strong>: `--flash_size 2MB`

<strong>bootloader.bin</strong>:  `0x0000`

<strong>wf_at_firmware.bin</strong>:  `0x10000`

<strong>partitions_two_ota.bin</strong>:  `0x8000`


烧录查看模块串口，命令行工具输入:
<br>
> `cd /dev`
> <br>
> `ls -a`

查看模块串口，假设模块串口为`tty.SLAB_USBtoUART`, 将 `flash.sh` 修改为：

`python2 $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp8266 --port /dev/tty.SLAB_USBtoUART --baud 9600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 2MB 0x0000 bootloader.bin 0x10000 wf_at_firmware.bin 0x8000 partitions_two_ota.bin`

保持并退出。

### 3. 烧录固件

先切换并进入到`at_firmware_build`所在文件夹，即`flash.sh`文件所在目录

> cd ~/project/at_firmware_build

执行烧录脚本

> ./flash.sh

即完成模块烧录工作。
