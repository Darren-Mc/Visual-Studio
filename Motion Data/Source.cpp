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

using namespace std;

BOOL  bConnect = FALSE;                 // Connect to recipient first
int   iPort = DEFAULT_PORT;          // Port to send data to
char  cChar = DEFAULT_CHAR;          // Character to fill buffer
DWORD dwCount = DEFAULT_COUNT,         // Number of messages to send
dwLength = DEFAULT_BUFFER_LENGTH; // Length of buffer to send
char  szRecipient[128] = "127.0.0.1";   // Recipient's IP or hostname
double packet[12];
double freq, amplitude;
int direction;

double forwardVelocity, verticalAccel, forwardAccel, lateralAccel, pitchAngle, rollAngle, heading, yawVelocity, onGroundIndicator;

//auto start = std::chrono::high_resolution_clock::now();//time_t start = time(0);


void getData()
{
	packet[0] = 0;
	do {
		cout << "Enter oscillation direction, 0 for pitch, 1 for roll: ";
		cin >> direction;//Oscillation Direction
	} while (direction > 1 || direction < 0);
	do {
		if (freq > 5 || freq < 0) cout << "Enter valid frequency ( [0-5] Hz): ";
		else cout << "Frequency (Hz): ";
		cin >> freq;//Oscillation Frequency
	} while (freq > 5 || freq < 0);
	do {
		if (amplitude > 20 || amplitude < 0) cout << "Enter valid angle ( [0 - 20] Deg): ";
		else cout << "Angle (Deg): ";
		cin >> amplitude;//Oscillation Angle
	} while (amplitude > 20 || amplitude < 0);
	/*cout << "Forward Velocity (m/s, +ve front): ";
	cin >> packet[1];//forwardVelocity;
	cout << "Vertical Acceleration (G's, +ve up): ";
	cin >> packet[2];//verticalAccel;
	cout << "Forward Acceleration (G's, +ve front): ";
	cin >> packet[3];//forwardAccel;
	cout << "Lateral Acceleration (G's, +ve right): ";
	cin >> packet[4];//lateralAccel;
	cout << "Pitch Angle (Degrees, +ve nose up): ";
	cin >> packet[5];//pitchAngle;
	cout << "Roll Angle (Degrees, +ve bank right): ";
	cin >> packet[6];//rollAngle;
	cout << "Heading (Degrees, 0 = North, clockwise): ";
	cin >> packet[7];//heading;
	cout << "Yaw Velocity (m/s, +ve yaw right): ";
	cin >> packet[8];//yawVelocity;
	cout << "On Ground Indicator: (1 or 0): ";
	cin >> packet[9];//onGroundIndicator;*/
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
	int            ret,
		i;
	SOCKADDR_IN    recipient;

	//getData();
	/*cout << "Enter any key to begin transmission\n";
	int k;
	cin >> k;*/

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
	if ((recipient.sin_addr.s_addr = inet_addr(szRecipient))
		== INADDR_NONE)
	{
		struct hostent *host = NULL;

		host = gethostbyname(szRecipient);
		if (host)
			CopyMemory(&recipient.sin_addr, host->h_addr_list[0],
				host->h_length);
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
			//cout << "!";
		}
	}
	else
	{
		// Otherwise, use the sendto() function
		//
		cout << "Sending data..." << endl;
		auto start = std::chrono::high_resolution_clock::now();//time_t start = time(0);

		while (1) //GetAsyncKeyState(VK_LCONTROL) == 0) //for(i = 0; i < dwCount; i++)
		{
			double timeStamp = (double)(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count()) / 1000; //Time Stamp (Seconds since Epoch)
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
			//cout<< timeStamp <<endl;

			/*for (int n=0; n < dwLength/4; n++)
			{
			float f;
			memcpy(&f,&sendbuf[4*n],4);
			cout << f << ", ";
			}*/
			this_thread::sleep_for(chrono::milliseconds(1)); //50Hz=20ms
		}
	}
	closesocket(s);

	GlobalFree(sendbuf);
	WSACleanup();

	//cout << "Enter any key to exit\n";
	//int j;
	//cin >> j;
	return 0;
}