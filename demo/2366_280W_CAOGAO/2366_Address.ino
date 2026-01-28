/*
寄存器名称                            I2C 地址    寄存器地址功能简述
SYS_CTL0（充电使能寄存器）             0XEA 0x00 控制充电相关功能使能 
SYS_CTL2（Vset 充满电压设定）          0XEA 0x02 设置单节电池充满电压 
SYS_CTL3（Iset 充电功率或电流设置）    0XEA 0x03 设定电池端电流最大限制 
SYS_CTL6（涓流充电电流设置）           0XEA 0x06 设置涓流充电电流 
SYS_CTL8（停充电流和再充电阈值设置）    0XEA 0x08 配置停充电流和再充电阈值 
SYS_CTL9（待机使能和低电电压设置）      0XEA 0x09 控制待机功能和低电电压相关设置 
SYS_CTL10（电池低电电压设置）           0XEA 0x0A 设置电池低电电压 
SYS_CTL11（输出使能寄存器）            0XEA 0x0B 控制放电输出及快充使能 
SYS_CTL12（输出最大功率选择寄存器）     0XEA 0x0C 选择 Vbus1 输出 / 输入功率 
SELECT_PDO（选择充电 PDO 档位）        0XEA 0X0D 选择充电 PDO 档位 
TypeC_CTL8（TYPE-C 模式控制寄存器）   0XEA 0x22 设置 Vbus CC 模式 
TypeC_CTL9（输出 Pdo 电流设置寄存器） 0XEA 0x23 控制各电压档位 Pdo 电流设置使能 
TypeC_CTL10（5VPdo 电流设置寄存器）   0XEA 0x24 设置 5VPdo 电流 
TypeC_CTL11（9VPdo 电流设置寄存器）   0XEA 0x25 设置 9VPdo 电流 
TypeC_CTL12（12VPdo 电流设置寄存器）  0XEA 0x26 设置 12VPdo 电流 
TypeC_CTL13（15VPdo 电流设置寄存器）  0XEA 0x27 设置 15VPdo 电流 
TypeC_CTL14（20VPdo 电流设置寄存器）  0XEA 0x28 设置 20VPdo 电流 
TypeC_CTL23（Pps1 Pdo 电流设置寄存器） 0XEA 0x29 设置 Pps1 Pdo 电流 
TypeC_CTL24（Pps2 Pdo 电流设置寄存器） 0XEA 0x2A 设置 Pps2 Pdo 电流 
TypeC_CTL17（输出 Pdo 设置寄存器）     0XEA 0x2B 控制各电压档位 Pdo 使能 
TYPEC_CTL18（PDO 加 10mA 电流使能）   0XEA 0x2C 使能各电压档位 PDO 加 10mA 电流 

STATE_CTL0（充电状态控制寄存器）       0XEA  0X31 指示充电、放电等状态 
STATE_CTL1（充电状态控制寄存器）       0XEA 0X32 补充充电状态信息 
STATE_CTL2（输入 Pd 状态控制寄存器）   0XEA 0X33 指示输入 Pd 相关状态 
TypeC_STATE（系统状态指示寄存器）      0XEA 0X34 显示 TypeC 输入输出连接及快充状态 
RECEIVED_PDO（接收 PDO 档位）         0XEA 0X35 指示接收到的 PDO 档位 
STATE_CTL3（系统过流指示寄存器）       0XEA 0X38 指示系统过流和短路状态 
BATVADC_DAT0（VBAT 电压寄存器）       0XEA 0X50 存储 VBAT 电压低 8 位数据 
BATVADC_DAT1（VBAT 电压寄存器）       0XEA 0X51 存储 VBAT 电压高 8 位数据 
VsysVADC_DAT0（Vsys 电压寄存器）      0XEA 0X52 存储 Vsys 电压低 8 位数据 
VsysVADC_DAT1（Vsys 电压寄存器）      0XEA 0X53 存储 Vsys 电压高 8 位数据 
IBATIADC_DAT0（BAT 端电流寄存器）     0XEA 0x6E 存储 BAT 端电流低 8 位数据 
IBATIADC_DAT1（BAT 端电流寄存器）    0XEA 0x6F 存储 BAT 端电流高 8 位数据 
ISYS_IADC_DAT0（Isys 端电流寄存器）  0XEA 0x70 存储 Isys 端电流低 8 位数据 
Isys_IADC_DAT1（Isys 端电流寄存器）  0XEA 0x71 存储 Isys 端电流高 8 位数据 
Vsys_POW_DAT0（Vsys 端功率寄存器）  	0XEA	0X74	存储 Vsys 端功率低 8 位数据
Vsys_POW_DAT1（Vsys 端功率寄存器）	 0XEA	0X75	存储 Vsys 端功率高 8 位数据
INTC_IADC_DAT0（NTC 输出电流寄存器）	0XEA	0X77	指示 NTC 输出电流
VGPIO0_NTC_DAT0（VGPIO0_NTC_ADC 电压寄存器）	0XEA	0X78	存储 VGPIO0_NTC_ADC 电压低 8 位数据
VGPIO0_NTC_DAT1（VGPIO0_NTC_ADC 电压寄存器）	0XEA	0X79	存储 VGPIO0_NTC_ADC 电压高 8 位数据

TIMENODE1（时间戳寄存器第一位）       0XEA 0x69 存储时间戳第一个 ASCII 符号值 
TIMENODE2（时间戳寄存器第二位）       0XEA 0x6A 存储时间戳第二个 ASCII 符号值 
TIMENODE3（时间戳寄存器第三位）       0XEA 0x6B 存储时间戳第三个 ASCII 符号值 
TIMENODE4（时间戳寄存器第四位）       0XEA 0x6C 存储时间戳第四个 ASCII 符号值 
TIMENODE5（时间戳寄存器第五位）       0XEA 0x6D 存储时间戳第五个 ASCII 符号值 
*/