	此程序在ubuntu16.04-32位下进行交叉编译，编译工具是arm-linux-gnueabihf，程序中已有makefile，在linux系统下，进入目录，进行make，即可生成可执行文件new，输入make distclean即可删除可执行文件和所有.o文件。
	通过tftp工具将new文件下载至ARM，添加可执行权限，即可执行程序。
	

	bell：{"cmd":"bell","value":{"time":3,"on":1000,"off":1000}}
	
	send：{"cmd":"send","value":{"data":[20,30]}}

	接受串口数据：{"cmd":"rev","value":{"data":[48, 32, 16, 64, 80]}}

	errorcode:
	0  OK
	1  open serial is empty!
	2  the name of serial is wrong
	3  the serial is  open
	4  serial is  close
	5  the data is empty
	6  the  parameter of command is error!

	注意：在多个TCP连接建立的情况下，串口具有惟一性，也就是说不能在多个TCP连接中打开同一个串口，否则会报错（errorcode：4）。
	
	串口芯片为FTDI232，需要将驱动加入开机自启动程序。


	盒子默认ip： 192.168.0.15   10.10.80.15
