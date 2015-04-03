#include <iostream>
#include <string>
#include <cstdlib>
#include "lynxmotion_ssc32/ssc32.h"

bool is_int( char *str )
{
	int i = 0;
	int len = strlen( str );

	if( len <= 0 )
		return false;

	if( str[i] != '-' && ( str[i] < '0' || str[i] > '9' ) )
		return false;

	i++;
	while( i < len )
	{
		if( str[i] < '0' || str[i] > '9' )
			return false;

		i++;
	}

	return true;
}

bool check_arg_is_int( char *str )
{
	if( !is_int( str ) )
	{
		std::cout << "Invalid argument '" << str << "'" << std::endl;
		return false;
	}

	return true;
}

int main( int argc, char **argv )
{
	std::string port;
	int baud;
	lynxmotion_ssc32::SSC32 ssc32_device;
	std::string version;
	int i = 1;

	if( argc < 2 )
	{
		std::cout << argv[0] << ": You must specify a command" << std::endl;
		std::cout << "Try `" << argv[0] << " --help' for more information." << std::endl;
		return 1;
	}

	// Check if port was provided as an argument
	if( strcmp( argv[i], "-p" ) == 0 )
	{
		if( argc < 4)
		{
			std::cout << argv[0] << ": You must specify a command" << std::endl;
			std::cout << "Try `" << argv[0] << " --help' for more information." << std::endl;
			return 1;
		}

		i++;
		port = argv[i];
		i++;
	}
	else
	{
		std::cout << "port not specified -- defaulting to /dev/ttyUSB0" << std::endl;
		port = "/dev/ttyUSB0";
	}

	// Check if baud was provided as an argument
	if( strcmp( argv[i], "-b" ) == 0 )
	{
		if( i + 2 >= argc )
		{
			std::cout << argv[0] << ": You must specify a command" << std::endl;
			std::cout << "Try `" << argv[0] << " --help' for more information." << std::endl;
			return 1;
		}

		i++;
		if( !check_arg_is_int( argv[i] ) )
			return 1;
		baud = atoi( argv[i] );
		i++;
	}
	else
	{
		std::cout << "baud not specified -- defaulting to 115200" << std::endl;
		baud = 115200;
	}

	if( strcmp( argv[i], "--help" ) == 0 || strcmp( argv[i], "-h" ) == 0 )
	{
		std::cout << "Usage: " << argv[0] << " [-p PORT] [-b BAUD] COMMAND" << std::endl;
		std::cout << std::endl;
		std::cout << "If provided, PORT and BAUD must be given in the specified order above." << std::endl;
		std::cout << std::endl;
		std::cout << "Possible commands:" << std::endl;
		std::cout << "  -a                     Read inputs as analog." << std::endl
			  << "  -c                     Cancel the current command." << std::endl
			  << "  -d CH LVL              Send discrete output to channel CH. LVL must be 0 or 1." << std::endl
			  << "  -i                     Read inputs as digital." << std::endl
			  << "  -m CH PW [S] [T]       Move a servo on channel CH to the given pulse width PW." << std::endl
			  << "  -o CH OFFSET           Offsets channel CH by OFFSET amount." << std::endl
			  << "  -s                     Checks if any servos are currently moving." << std::endl
			  << "  -v                     Get the software version." << std::endl;
	}
	else if( strcmp( argv[i], "-v" ) == 0 )
	{
		if( !ssc32_device.open_port( port.c_str( ), baud ) )
			return -1;

		std::cout << "Getting software version information..." << std::endl;

		version = ssc32_device.get_version( );

		std::cout << "SSC32 Software Version: " << version << std::endl;
	}
	else if( strcmp( argv[i], "-c" ) == 0 )
	{
		if( !ssc32_device.open_port( port.c_str( ), baud ) )
			return -1;

		std::cout << "Cancelling current command..." << std::endl;

		if( !ssc32_device.cancel_command( ) )
			return -1;

		std::cout << "Done" << std::endl;
	}
	else if( strcmp( argv[i], "-m" ) == 0 )
	{
		if( i + 2 > argc )
		{
			std::cout << "Usage: " << argv[0] << " [-p PORT] [-b BAUD] -m CHANNEL PULSE_WIDTH [SPEED] [TIME]" << std::endl;
			return 1;
		}

		lynxmotion_ssc32::SSC32::ServoCommand cmd;

		if( !check_arg_is_int( argv[i + 1] ) )
			return 1;
		cmd.ch = atoi( argv[i + 1] );

		if( !check_arg_is_int( argv[i + 2] ) )
			return 1;
		cmd.pw = atoi( argv[i + 2] );

		int time = -1;

		if( i + 3 < argc )
		{
			if( !check_arg_is_int( argv[i + 3] ) )
				return 1;
			cmd.spd = atoi( argv[i + 3] );
		}

		if( i + 4 < argc )
		{
			if( !check_arg_is_int( argv[i + 4] ) )
				return 1;
			time = atoi( argv[i + 4] );
		}

		if( !ssc32_device.open_port( port.c_str( ), baud ) )
			return 1;

		std::cout << "Sending command to move servo " << cmd.ch << " to pulse width " << cmd.pw << std::endl;

		if( !ssc32_device.move_servo( cmd, time ) )
			return 1;
	}
	else if( strcmp( argv[i], "-o" ) == 0 || strcmp( argv[i], "--offset" ) == 0 )
	{
		if( i + 2 >= argc )
		{
			std::cout << "Usage: " << argv[0] << " [-p PORT] [-b BAUD] -o CHANNEL OFFSET" << std::endl;
			return 1;
		}

		if( !check_arg_is_int( argv[i + 1] ) )
			return 1;
		int ch = atoi( argv[i + 1] );

		if( !check_arg_is_int( argv[i + 2] ) )
			return 1;
		int offset = atoi( argv[i + 2] );

		if( !ssc32_device.open_port( port.c_str( ), baud ) )
			return 1;

		std::cout << "Sending command to offset servo " << ch << " by pulse width " << offset << std::endl;

		if( !ssc32_device.pulse_offset( ch, offset ) )
			return 1;
	}
	else if( strcmp( argv[i], "-s" ) == 0 || strcmp( argv[i], "--status" ) == 0 )
	{
		if( !ssc32_device.open_port( port.c_str( ), baud ) )
			return 1;

		if( ssc32_device.query_movement_status( ) )
			std::cout << "Servos are currently moving." << std::endl;
		else
			std::cout << "No servos are currently moving." << std::endl;
	}
	else if( strcmp( argv[i], "-d" ) == 0 )
	{
		if( i + 2 >= argc )
		{
			std::cout << "Usage: " << argv[0] << " [-p PORT] [-b BAUD] -d CHANNEL LEVEL" << std::endl;
			return 1;
		}

		if( !check_arg_is_int( argv[i + 1] ) )
			return 1;
		int ch = atoi( argv[i + 1] );

		lynxmotion_ssc32::SSC32::LogicLevel level;
		if( strcmp( argv[i + 2], "0" ) == 0 )
			level = lynxmotion_ssc32::SSC32::Low;
		else if( strcmp( argv[i + 2], "1" ) == 0 )
			level = lynxmotion_ssc32::SSC32::High;
		else
		{
			std::cout << "Level must be 0 (0V) or 1 (5V)" << std::endl;
			return 1;
		}

		if( !ssc32_device.open_port( port.c_str( ), baud ) )
			return 1;

		if( !ssc32_device.discrete_output( ch, level ) )
			return 1;
	}
	else if( strcmp( argv[i], "-a" ) == 0 )
	{
		lynxmotion_ssc32::SSC32::Inputs inputs[4];
		float data[4];

		inputs[0] = lynxmotion_ssc32::SSC32::PinA;
		inputs[1] = lynxmotion_ssc32::SSC32::PinB;
		inputs[2] = lynxmotion_ssc32::SSC32::PinC;
		inputs[3] = lynxmotion_ssc32::SSC32::PinD;

		if( !ssc32_device.open_port( port.c_str( ), baud ) )
			return 1;

		if( !ssc32_device.read_analog_inputs( inputs, data, 4 ) )
			return 1;

		for( int j = 0; j < 4; j++ )
			std::cout << "Pin " << ( ( char )j + 'A' ) << ": " << data[j] << std::endl;
	}
	else if( strcmp( argv[i], "-i" ) == 0 )
	{
		lynxmotion_ssc32::SSC32::Inputs inputs[4];
		unsigned int data[4];

		inputs[0] = lynxmotion_ssc32::SSC32::PinA;
		inputs[1] = lynxmotion_ssc32::SSC32::PinB;
		inputs[2] = lynxmotion_ssc32::SSC32::PinC;
		inputs[3] = lynxmotion_ssc32::SSC32::PinD;

		if( !ssc32_device.open_port( port.c_str( ), baud ) )
			return 1;

		if( !ssc32_device.read_digital_inputs( inputs, data, 4 ) )
			return 1;

		for( int j = 0; j < 4; j++ )
			std::cout << "Pin " << ( ( char )j + 'A' ) << ": " << data[j] << std::endl;
	}
        else if( strcmp( argv[i], "--dk_test" ) == 0 )
        {
	    if( !ssc32_device.open_port( port.c_str( ), baud ) )
	    	return 1;

	    lynxmotion_ssc32::SSC32::ServoCommand cmd[3];
            cmd[0].ch = 0;
            cmd[1].ch = 1;
            cmd[2].ch = 2;

	    lynxmotion_ssc32::SSC32::ServoCommand cmd2[3];
            cmd2[0].ch = 12;
            cmd2[1].ch = 13;
            cmd2[2].ch = 14;
            for (int i = 0; i <3; i++) {
                cmd[0].pw = 1977;
                cmd[1].pw = 1891;
                cmd[2].pw = 1520;
                ssc32_device.move_servo(cmd, 3);
                cmd2[0].pw = 1103;
                cmd2[1].pw = 1049;
                cmd2[2].pw = 1480;
                ssc32_device.move_servo(cmd2, 3);
                sleep(1);
                cmd[0].pw = 1877;
                cmd[1].pw = 1791;
                cmd[2].pw = 1420;
                ssc32_device.move_servo(cmd, 3);
                cmd2[0].pw = 1103+100;
                cmd2[1].pw = 1049+100;
                cmd2[2].pw = 1480+100;
                ssc32_device.move_servo(cmd2, 3);
                sleep(1);
            }
            
        }
	else
	{
		std::cout << argv[0] << ": invalid option -- '" << argv[i] << "'" << std::endl;
		std::cout << "Try `" << argv[0] << " --help' for more information." << std::endl;
                return 1;
	}

	return 0;
}
