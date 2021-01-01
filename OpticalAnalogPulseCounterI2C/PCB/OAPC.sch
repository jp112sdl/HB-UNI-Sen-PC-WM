EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Arduino_Pro_Mini:Arduino_Pro_Mini_Sparkfun X1
U 1 1 5FEC6850
P 4270 3430
F 0 "X1" H 4270 2449 60  0000 C CNN
F 1 "Arduino_Pro_Mini" H 4270 2343 60  0000 C CNN
F 2 "additional:Arduino_Pro_Mini" H 4270 2237 60  0001 C CNN
F 3 "" H 4270 2237 60  0000 C CNN
	1    4270 3430
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 5FEC7D3C
P 5840 3080
F 0 "R2" V 5750 3080 50  0000 C CNN
F 1 "270" V 5840 3080 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5770 3080 50  0001 C CNN
F 3 "~" H 5840 3080 50  0001 C CNN
	1    5840 3080
	0    1    1    0   
$EndComp
Text GLabel 5070 3180 2    50   Input ~ 0
SCL
Text GLabel 5080 3380 2    50   BiDi ~ 0
SDA
Wire Wire Line
	4920 3380 5080 3380
Wire Wire Line
	5070 3180 4920 3180
$Comp
L power:GND #PWR02
U 1 1 5FEC8B2C
P 6840 3860
F 0 "#PWR02" H 6840 3610 50  0001 C CNN
F 1 "GND" H 6845 3687 50  0000 C CNN
F 2 "" H 6840 3860 50  0001 C CNN
F 3 "" H 6840 3860 50  0001 C CNN
	1    6840 3860
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR01
U 1 1 5FECA047
P 5570 2530
F 0 "#PWR01" H 5570 2380 50  0001 C CNN
F 1 "+5V" H 5585 2703 50  0000 C CNN
F 2 "" H 5570 2530 50  0001 C CNN
F 3 "" H 5570 2530 50  0001 C CNN
	1    5570 2530
	1    0    0    -1  
$EndComp
Wire Wire Line
	5570 2530 5570 3080
Wire Wire Line
	5570 3080 4920 3080
Connection ~ 5570 3080
$Comp
L Device:LED D1
U 1 1 5FECC773
P 6140 3080
F 0 "D1" H 6140 2990 50  0000 C CNN
F 1 "LED green" H 6140 3190 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 6140 3080 50  0001 C CNN
F 3 "~" H 6140 3080 50  0001 C CNN
	1    6140 3080
	-1   0    0    1   
$EndComp
$Comp
L Device:R R1
U 1 1 5FECDE42
P 5570 3380
F 0 "R1" V 5480 3380 50  0000 C CNN
F 1 "100k" V 5570 3380 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5500 3380 50  0001 C CNN
F 3 "~" H 5570 3380 50  0001 C CNN
	1    5570 3380
	-1   0    0    1   
$EndComp
Wire Wire Line
	5570 3080 5570 3230
Wire Wire Line
	5690 3080 5570 3080
Wire Wire Line
	5570 3530 5570 3680
Wire Wire Line
	5570 3680 4920 3680
$Comp
L Sensor_Optical:SFH309 Q1
U 1 1 5FED028A
P 6160 3780
F 0 "Q1" V 6488 3780 50  0000 C CNN
F 1 "SFH309" V 6397 3780 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 6640 3640 50  0001 C CNN
F 3 "http://www.osram-os.com/Graphics/XPic2/00101811_0.pdf/SFH%20309,%20SFH%20309%20FA,%20Lead%20(Pb)%20Free%20Product%20-%20RoHS%20Compliant.pdf" H 6160 3780 50  0001 C CNN
	1    6160 3780
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5570 3680 5960 3680
Connection ~ 5570 3680
Wire Wire Line
	6290 3080 6840 3080
Wire Wire Line
	6360 3680 6840 3680
Wire Wire Line
	6840 3080 6840 3680
Connection ~ 6840 3680
Wire Wire Line
	6840 3680 6840 3860
Wire Wire Line
	4920 2880 6840 2880
Wire Wire Line
	6840 2880 6840 3080
Connection ~ 6840 3080
NoConn ~ 4920 4280
NoConn ~ 4920 4180
NoConn ~ 4920 4080
NoConn ~ 4920 3980
NoConn ~ 4920 3880
NoConn ~ 4920 3780
NoConn ~ 4920 3580
NoConn ~ 4920 3480
NoConn ~ 4920 3280
NoConn ~ 4920 2980
NoConn ~ 4920 2780
NoConn ~ 3620 2780
NoConn ~ 3620 2880
NoConn ~ 3620 2980
NoConn ~ 3620 3080
NoConn ~ 3620 3180
NoConn ~ 3620 3280
NoConn ~ 3620 3380
NoConn ~ 3620 3480
NoConn ~ 3620 3580
NoConn ~ 3620 3680
NoConn ~ 3620 3780
NoConn ~ 3620 3880
NoConn ~ 4020 2280
NoConn ~ 4120 2280
NoConn ~ 4220 2280
NoConn ~ 4320 2280
NoConn ~ 4420 2280
NoConn ~ 4520 2280
$EndSCHEMATC
