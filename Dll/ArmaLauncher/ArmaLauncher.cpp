// ArmaLauncher.cpp: определяет экспортированные функции для приложения DLL.
//

// ArmaLauncher.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"

using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Runtime::InteropServices;
using namespace System::Diagnostics;

using namespace System::IO;

/*! Функция возвращающая подробную информацию о сервере
* in:	const char* h				- Адресс сервера
*		int port					- Порт сервера
*		const int timeout			- Максимальное время ответа от сервера (в милисекундах)
*		const unsigned char* str	- Отправляемая строка
*		const int len				- Длина отправляемой строки
*---------------------------------------------------------------------------------------------
* out:	unsigned char*				- Принятая строка от сервера
*		int &ping					- Время ответа от сервера
*       int &rLen					- Длина возращаемого байтового массива
*/
extern "C" __declspec(dllexport)
unsigned char* exchangeDataWithServer(const char* host, int port, const int timeout, const unsigned char* str, const int len, int &ping, int &rLen) {
	// Масив выходных данных
	unsigned char* data;
	// Даем понять что сервер ещё не пинговался, дабы можно было отследить, потеряно ли соеденение с сервером
	ping=-1;

	/*
	 * Обмен данными с сервером
	 */
	//..объявляем Udp сокет
	UdpClient^ client=gcnew UdpClient(56800);
	try {
		/*
		 * Подключаем сокет к серверу и настраивваем его
		 */
		// Получаем конечную точку сервера
		IPEndPoint^ remoteIpEndpoint=gcnew IPEndPoint(IPAddress::Parse(gcnew String(host)), port);
		// Устанавливаем timeout
		client->Client->ReceiveTimeout=timeout;
		// Подключение к серверу
		client->Connect(remoteIpEndpoint);
		// Готовим строку к отправке
		array<Byte>^ sendBytes=gcnew array<Byte>(len);
		for (int i=0; i < len; i++) {
			sendBytes[i]=str[i];
		}
		// Готовимся считать пинг
		Stopwatch^ stopwatch=gcnew Stopwatch();
		/*
		 * Начинаем обмен данными с сервером
		 */
		// Отправка строки серверу
		client->Send(sendBytes, sendBytes->Length);
		// Начинаем считать время до примема ответа
		stopwatch->Start();
		// Получаем ответ от сервера
		array<Byte>^ response=client->Receive(remoteIpEndpoint);
		// Заканчиваем считать время 
		stopwatch->Stop();
		// Закрываем сокет
		client->Close();
		/*
		 * Обрабатываем полученные данные
		 */
		//..приводя их к unsigned char
		pin_ptr<System::Byte> ptr=&response[0];
		data=ptr;
		//..и отправляем результат
		ping=(int)stopwatch->ElapsedMilliseconds;
		rLen=(int)response->Length;
		delete client;
		return data;
	} catch (Exception^ e) {
		// Закрываем сокет
		client->Close();
		delete client;
		return 0;
	}
}