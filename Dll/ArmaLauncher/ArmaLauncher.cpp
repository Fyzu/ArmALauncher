// ArmaLauncher.cpp: ���������� ���������������� ������� ��� ���������� DLL.
//

// ArmaLauncher.cpp: ���������� ���������������� ������� ��� ���������� DLL.
//

#include "stdafx.h"

using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Runtime::InteropServices;
using namespace System::Diagnostics;

using namespace System::IO;

/*! ������� ������������ ��������� ���������� � �������
* in:	const char* h				- ������ �������
*		int port					- ���� �������
*		const int timeout			- ������������ ����� ������ �� ������� (� ������������)
*		const unsigned char* str	- ������������ ������
*		const int len				- ����� ������������ ������
*---------------------------------------------------------------------------------------------
* out:	unsigned char*				- �������� ������ �� �������
*		int &ping					- ����� ������ �� �������
*       int &rLen					- ����� ������������ ��������� �������
*/
extern "C" __declspec(dllexport)
unsigned char* exchangeDataWithServer(const char* host, int port, const int timeout, const unsigned char* str, const int len, int &ping, int &rLen) {
	// ����� �������� ������
	unsigned char* data;
	// ���� ������ ��� ������ ��� �� ����������, ���� ����� ���� ���������, �������� �� ���������� � ��������
	ping=-1;

	/*
	 * ����� ������� � ��������
	 */
	//..��������� Udp �����
	UdpClient^ client=gcnew UdpClient(56800);
	try {
		/*
		 * ���������� ����� � ������� � ������������ ���
		 */
		// �������� �������� ����� �������
		IPEndPoint^ remoteIpEndpoint=gcnew IPEndPoint(IPAddress::Parse(gcnew String(host)), port);
		// ������������� timeout
		client->Client->ReceiveTimeout=timeout;
		// ����������� � �������
		client->Connect(remoteIpEndpoint);
		// ������� ������ � ��������
		array<Byte>^ sendBytes=gcnew array<Byte>(len);
		for (int i=0; i < len; i++) {
			sendBytes[i]=str[i];
		}
		// ��������� ������� ����
		Stopwatch^ stopwatch=gcnew Stopwatch();
		/*
		 * �������� ����� ������� � ��������
		 */
		// �������� ������ �������
		client->Send(sendBytes, sendBytes->Length);
		// �������� ������� ����� �� ������� ������
		stopwatch->Start();
		// �������� ����� �� �������
		array<Byte>^ response=client->Receive(remoteIpEndpoint);
		// ����������� ������� ����� 
		stopwatch->Stop();
		// ��������� �����
		client->Close();
		/*
		 * ������������ ���������� ������
		 */
		//..������� �� � unsigned char
		pin_ptr<System::Byte> ptr=&response[0];
		data=ptr;
		//..� ���������� ���������
		ping=(int)stopwatch->ElapsedMilliseconds;
		rLen=(int)response->Length;
		delete client;
		return data;
	} catch (Exception^ e) {
		// ��������� �����
		client->Close();
		delete client;
		return 0;
	}
}