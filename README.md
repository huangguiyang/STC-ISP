STC-ISP
=======

A ROM burner for STC90C516RD+ MCU.

This MCU is Intel 80C51 compatible. But the ISP (In System Programming) download protocol is non-public. 

I hack on this just for fun.

####Note
***

1. Windows version can work so far.

2. Have some problems under Linux, now in process...

3. Only tested for STC90C516RD+.

4. Only support .bin format now.


####TODO
***

* Finish the command line user interface.

* Support .hex format.

* Support Linux.

* Power-on download support.

####License
***

MIT License. See LICENSE.


####For Chinese Users
***

这是一款 STC90C516RD+ 单片机的烧录程序。由于 STC 官方的实在惨不忍睹，加上个人兴趣，就根据监听工具分析了它的协议。

目前关键部分的协议差不多已经分析完毕，至少烧录是没问题了。

由于在 Linux 下调试串口多日无果，因此先做 Windows 版本。

目前 Windows 版本已经能可以烧录。