#	cubieboard4(cc-a80)配置  
##	环境配置
###	刷写系统
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

3. 解压文件、校验并烧录到tf卡中 ** 注意，所有TF卡中的数据将丢失且无法恢复！！ **
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

4. 烧写emmc ** 注意，所有emmc卡中的数据将丢失且无法恢复，请事先备份 **

	1. 将制作好刷 emmc 的 TF 启动卡插入 CC-A80 的 TF-CARD 卡槽
	2. 用官方标配的直流电源或者电池上电启动
	3. 红色和绿色 LED 灯持续闪烁表示系统烧写正常
	4. 如果量产过程出现异常，红色 LED 灯将会由闪烁变成长亮
	5. 刷写完成后，红色和绿色 LED 灯都熄灭,系统自动关机,烧写完成
	6. ** 拔掉 TF 卡 ** ，拔掉cc-a80电源线

### 安装依赖

1. 登录系统

	1. 需要一个能够直接上网的路由器，给cc-80插上网线	
	
	2. 获取cc-a80的ip地址 
	
		* 如果有路由器管理员权限,则给cc-a80供电，直接登录路由器查看cc-80的ip地址
		
		* 没有管理员权限，或者无法由路由器获取cc-a80的ip地址的,使用串口登录（此方法也可以在没有网络的情况下临时登录，但不宜长期使用）	
		
			1. 将串口转USB设备插入电脑USB接口，另一端插入cc-a80对应端口
			
				* cc-a80上,在VGA接口旁边，有一排排针（注意，是1排4个，不是2排12个的排针），在排针下pcb，找GND，TX，RX，NC
				* 将黑线插入GND，白线插入RX，绿线插入TX, NC *不连接*	
				
			2. 电脑端配置	
	 
				* linux用户
	
					```shell
					sudo apt-get install minicom #安装minicom
					sudo apt-get install tigervnc-viewer #安装VNC client 方便在没有显示器的情况下使用图形界面
					sudo minicom -s #配置minicom(/dev/USBtty0,115200,8n1)
					sudo minicom
					```
		
				* windows用户
	
					1. 下载putty, 下载VNC Viewer软件
					2. 进入设备管理器，查看ch341的串口号
					3. putty设置为相应的串口号,启动软件	
					
			3. 插入cc-a80电源线，启动，可以看到由信息输出
			
			4. 启动完成后，使用用户名linaro密码linaro登录	
					
			5. 登陆后，输入ip addr | grep -w inet 得到cc-a80的ip地址	
			
	3. windows用户使用putty软件，用户名lianro密码linaro，登录cc-a80的ip地址，linux使用ssh linaro@cc-a80ip地址 登录	

3. 更新软件	

	```shell
	sudo apt-get update # 更新源	
	# 删除一些永不到的且更新费时间的包
	sudo apt-get --purge --auto-remove libreoffice* chromiun*
	sudo apt-get upgrade -y # 更新程序	
	## 重启系统，跟换发行版，升级系统	
	#sudo reboot
	#sudo cat <<EOF > /etc/apt/sources.list  
	#deb http://ports.ubuntu.com/ubuntu-ports/ xenial main universe
	#deb-src http://ports.ubuntu.com/ubuntu-ports/ xenial main universe
	#EOF
	## 更新更换过的源  
	#sudo apt-get update
	## nfs-kernel-server在更新时出点小问题，所以事先删除，需要的时候再安装  
	#sudo apt-get --purge --auto-remove remove nfs-kernel-server
	```

2. 安装所需软件	

	```shell
	sudo apt-get install build-essential cmake make git # 安装开发工具 # 安装时如果遇到提示是否替换某文件，一律选否	
	sudo apt-get install libtbb-dev # 安装tbb		
	sudo apt-get install libboost-all-dev # 安装boost  
	sudo apt-get install libopencv-dev # 安装opencv
	sudo apt-get install bash-completion byobu # 一些小工具
	sudo apt-get install tightvncserver # vnc server
	sudo reboot
	```
	
## 测试		

1. 下载实例程序

	```shell
	git clone https://github.com/woxuankai/HDR.git
	```

2. 编译

	```shell
	cd hdr
	mkdir build
	cd build
	cmake ..
	cmake -DCMAKE_BUILD_TYPE=Release .
	make -j4
	vncserver # 第一次运行改命令时会提示设置密码，请记住设置的密码
	```

3. 运行		

	1. 将USB摄像头插入cc-a80  
	2. 打开图形界面
		1. 有鼠标键盘显示器的：将鼠标键盘显示器插入cc-a80，登录图形界面
		2. 无鼠标键盘显示器的：运行VNC viewer（其它VNC client也可），用ip地址加:1登录（ 例如192.168.1.123:1 ），输入之前设置的密码登录图形界面
		3. 登录图形界面后，在桌面上双击运行LXTerminal程序
	3. 在打开的LXTerminal中运行以下命令
		```shell
		cd ~/HDR/hdr/build
		./dohdr #  运行之前插上摄像头，且须在图形界面上运行	
		```
