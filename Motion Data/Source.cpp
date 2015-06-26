#include <iostream>
#include <ctime>
#include <WinSock2.h>
#include <chrono>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT            48001
#define DEFAULT_COUNT           1000
#define DEFAULT_CHAR            '0'
#define DEFAULT_BUFFER_LENGTH   48

BOOL  bConnect = FALSE;                 // Connect to recipient first
int   iPort = DEFAULT_PORT;          // Port to send data to
char  cChar = DEFAULT_CHAR;          // Character to fill buffer
DWORD dwCount = DEFAULT_COUNT,         // Number of messages to send
dwLength = DEFAULT_BUFFER_LENGTH; // Length of buffer to send
char  szRecipient[128] = "127.0.0.1";   // Recipient's IP or hostname
float packet[12];
double freq, amplitude;
int direction;

float forwardVelocity, verticalAccel, forwardAccel, lateralAccel, pitchAngle, rollAngle, heading, yawVelocity, onGroundIndicator;

//auto start = std::std::chrono::high_resolution_clock::now();//time_t start = time(0);


void getData()
{
	packet[0] = 0;

	std::cout << "Enter oscillation direction, 0 for pitch, 1 for roll: ";
	std::cin >> direction; //Oscillation Direction
	while ( std::cin.fail() || direction > 1 || direction < 0 )
	{
		std::cout << "Enter valid oscillation direction, 0 for pitch, 1 for roll: ";
		std::cin.clear();
		std::cin.ignore(256, '\n');
		std::cin >> direction;
	}

	std::cout << "Frequency (Hz): ";
	std::cin >> freq; //Oscillation Frequency
	while ( std::cin.fail() || freq > 5 || freq < 0 )
	{
		std::cout << "Enter valid frequency ( [0-5] Hz): ";
		std::cin.clear();
		std::cin.ignore(256, '\n');
		std::cin >> freq;
	}

	std::cout << "Angle (Deg): ";
	std::cin >> amplitude; //Oscillation Maximum Angle
	while ( std::cin.fail() || amplitude > 20 || amplitude < 0 )
	{
		std::cout << "Enter valid angle ( [0 - 20] Deg): ";
		std::cin.clear();
		std::cin.ignore(256, '\n');
		std::cin >> amplitude;
	}


	/*std::cout << "Forward Velocity (m/s, +ve front): ";
	std::cin >> packet[1];//forwardVelocity;
	std::cout << "Vertical Acceleration (G's, +ve up): ";
	std::cin >> packet[2];//verticalAccel;
	std::cout << "Forward Acceleration (G's, +ve front): ";
	std::cin >> packet[3];//forwardAccel;
	std::cout << "Lateral Acceleration (G's, +ve right): ";
	std::cin >> packet[4];//lateralAccel;
	std::cout << "Pitch Angle (Degrees, +ve nose up): ";
	std::cin >> packet[5];//pitchAngle;
	std::cout << "Roll Angle (Degrees, +ve bank right): ";
	std::cin >> packet[6];//rollAngle;
	std::cout << "Heading (Degrees, 0 = North, clockwise): ";
	std::cin >> packet[7];//heading;
	std::cout << "Yaw Velocity (m/s, +ve yaw right): ";
	std::cin >> packet[8];//yawVelocity;
	std::cout << "On Ground Indicator: (1 or 0): ";
	std::cin >> packet[9];//onGroundIndicator;*/
}

//
// Function: main
//
// Description:
//    Main thread of execution. Initialize Winsock, parse the command
//    line arguments, create a socket, connect to the remote IP
//    address if specified, and then send datagram messages to the
//    recipient.
//
int main()
{
	WSADATA        wsd;
	SOCKET         s;
	char          *sendbuf = NULL;
	int            ret;
	unsigned int   i;
	SOCKADDR_IN    recipient;

	//getData();
	/*std::cout << "Enter any key to begin transmission\n";
	int k;
	std::cin >> k;*/

	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		printf("WSAStartup failed!\n");
		return 1;
	}
	// Create the socket
	//
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET)
	{
		printf("socket() failed; %d\n", WSAGetLastError());
		return 1;
	}
	// Resolve the recipient's IP address or hostname
	//
	recipient.sin_family = AF_INET;
	recipient.sin_port = htons((short)iPort);
	if ((recipient.sin_addr.s_addr = inet_addr(szRecipient)) == INADDR_NONE)
	{
		struct hostent *host = NULL;

		host = gethostbyname(szRecipient);
		if (host)
			CopyMemory(&recipient.sin_addr, host->h_addr_list[0], host->h_length);
		else
		{
			printf("gethostbyname() failed: %d\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
	}
	// Allocate the send buffer
	//
	sendbuf = (char*)GlobalAlloc(GMEM_FIXED, dwLength);
	if (!sendbuf)
	{
		printf("GlobalAlloc() failed: %d\n", GetLastError());
		return 1;
	}
	//memset(sendbuf, cChar, dwLength);
	getData();
	memcpy(sendbuf, packet, sizeof(packet));
	//
	// If the connect option is set, "connect" to the recipient
	// and send the data with the send() function
	//
	if (bConnect)
	{
		if (connect(s, (SOCKADDR *)&recipient,
			sizeof(recipient)) == SOCKET_ERROR)
		{
			printf("connect() failed: %d\n", WSAGetLastError());
			GlobalFree(sendbuf);
			WSACleanup();
			return 1;
		}
		for (i = 0; i < dwCount; i++)
		{
			ret = send(s, sendbuf, dwLength, 0);
			if (ret == SOCKET_ERROR)
			{
				printf("send() failed: %d\n", WSAGetLastError());
				break;
			}
			else if (ret == 0)
				break;
			//std::cout << "!";
		}
	}
	else
	{
		// Otherwise, use the sendto() function
		//
		while (1)
		{
			std::cout << "Sending data... Press Escape to stop." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			auto start = std::chrono::high_resolution_clock::now();//time_t start = time(0);

			while (GetAsyncKeyState( VK_ESCAPE ) == 0)//for(i = 0; i < dwCount; i++)
			{
				double timeStamp = (double)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) / 1000; //Time Stamp (Seconds since Epoch)
				memcpy(sendbuf, &timeStamp, 4);
				int timer = (int)timeStamp;
				double angle = amplitude*sin(2 * 3.14159*freq*timeStamp);
				memcpy(&sendbuf[20 + direction * 4], &angle, 4);
				ret = sendto(s, sendbuf, dwLength, 0,
					(SOCKADDR *)&recipient, sizeof(recipient));
				if (ret == SOCKET_ERROR)
				{
					printf("sendto() failed; %d\n", WSAGetLastError());
					break;
				}
				else if (ret == 0)
					break;


				/*float f;
				memcpy(&f,&sendbuf,4);*/
				//std::cout<< timeStamp <<std::endl;

				/*for (int n=0; n < dwLength/4; n++)
				{
				float f;
				memcpy(&f,&sendbuf[4*n],4);
				std::cout << f << ", ";
				}*/
				std::this_thread::sleep_for(std::chrono::milliseconds(1)); //50Hz=20ms
			}

			char response;
			std::cout << "Send new data? (y/n)" << std::endl;
			std::cin >> response;
			if (response == 'n') break;

			getData();
			memcpy(sendbuf, packet, sizeof(packet));
		}
	}
	closesocket(s);

	GlobalFree(sendbuf);
	WSACleanup();

	//std::cout << "Enter any key to exit\n";
	//int j;
	//std::cin >> j;
	return 0;
}