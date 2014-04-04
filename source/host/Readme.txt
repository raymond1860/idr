  
    Usage: %s [-p </dev/ttySx>] [-b <baudrate>] \n\n"
    "Specify config in command line\n"
		"For linux,port name should be /dev/ttySx\n"
		"For win32,port name should be COMx\n"		
		"For example:\n"
		"%s -p /dev/ttyUSB0 \n"
		"%s -p /dev/ttyUSB0 -b 115200\n"
		"%s -p COM4 \n"
		"%s -p COM4 -b 115200\n"
		"\nSpecify config in config file id2reader.cfg\n"
		"Example config file:\n"
		"port=COM4\n"
		"baudrate=115200\n",		
