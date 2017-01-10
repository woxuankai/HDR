# HDR
基于人视觉构造高性能实现(移植)HDR算法
## 环境配置
### 刷写系统
1. 打开[官方论坛](http://www.cubie.cc/forum.php), 在第一行找到 “国内下载”

2. 点开链接后进入百度云盘界面，依次选择
Model > CubieBoard4 > Image > Ununtu-linaro-desktop
下载linaro-desktop-cb4-emmc-hdmi-v1.1.img.7z文件

	* 选择该镜像，是因为桌面版对硬件的支持更好
	* 镜像名称含义
	
		+ linaro  ->  发型版的名称
		+ desktop ->  桌面版
		+ cb4     ->  运行的硬件设备，cb4 就是 CC-A80
		+ emmc    ->  都是卡固件，card 指的是 tf 卡固件，emmc 指的是 tf 卡刷 emmc 卡固件
		+ hdmi    ->  显示模式，有 HDMI 和 VGA2 种显示模式
		+ v1.1    ->  固件的版本号，迭代开发固件，请使用最新的版本号
		+ img     ->  解压后的固件文件格式，是 2 进制文件
		+ 7z      ->  压缩后固件的文件格式，7z 

3. 解压文件、校验并烧录到tf卡中 *注意，所有TF卡中的数据将丢失且无法恢复！！*
	* Windows用户
	
  		1. 下载安装7-zip解压文件(WinRAR应该也可以)
		2. 下载安装win32diskimager制作启动卡
			* 该软件可以计算md5值,与百度云盘同一文件夹中的linaro-desktop-cb4-emmc-hdmi-v1.1.img.7z.md5进行比较
			
	* Linux用户
	
		```shell
		$ 7z x -so linaro-desktop-cb4-emmc-hdmi-v1.1.img.7z | md5sum
		$ 7z x -so linaro-desktop-cb4-emmc-hdmi-v1.1.img.7z | sudo dd of=/dev/sdx status=progress
		$ sync
		```

4. 烧写emmc *注意，所有emmc卡中的数据将丢失且无法恢复，请事先备份*
	1. 将制作好刷 emmc 的 TF 启动卡插入 CC-A80 的 TF-CARD 卡槽
	2. 用官方标配的直流电源或者电池上电启动
	3. 红色和绿色 LED 灯持续闪烁表示系统烧写正常
	4. 如果量产过程出现异常，红色 LED 灯将会由闪烁变成长亮
	5. 刷写完成后，红色和绿色 LED 灯都熄灭,系统自动关机,烧写完成
	6. *_拔掉 TF 卡_*,进入下一步系统开机测试

### 安装依赖	
