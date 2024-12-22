#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <vector>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <format>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "iphlpapi.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "69420"


namespace GlobalValues {
	HANDLE ConsoleHandle;

	
}

class Clients
{
public:
	static void SendClients(std::string SenderIP, std::string Msg) {
		for (size_t i = 0; i < ClientList.size(); i++)
		{
			if (ClientList[i]->Ip == SenderIP)continue;
			ClientList[i]->SendClientMsg(Msg);
		}
	}

	void SendClientMsg(std::string Msg) {

	}

	static void ClientDatas() {

	}



	Clients(std::string IP) {

	};
	~Clients();

private:
	static std::vector<Clients*> ClientList;

	std::string MacAddr = "";
	std::string Ip = "";
	std::string Port = "";
	std::string Name = "";
};

class Server
{
private:
	struct Message_ {
		static std::vector<Message_> EncryptedQueue;

		struct ByteType {
			int Size;
			std::vector<BYTE> Data_;




			ByteType(std::vector<BYTE>* DataReference, ULONGLONG& DataCounter)
			{

				std::vector<BYTE> DataCopy;

				this->Size = 0;

				memcpy(&this->Size, DataReference->data() + DataCounter, sizeof(int));


				if (this->Size >= DEFAULT_BUFLEN) {
					std::cout << "[LOG] Data is faulty, cleaning up application";

					Sleep(4000);

					WSACleanup();
					exit(-1);
				}

				DataCounter += 4;

				for (int i = 0; i < this->Size; i++, DataCounter++)
				{
					DataCopy.push_back(DataReference[0][DataCounter]);
				}


				this->Data_ = DataCopy;
			}

			std::string GetString()
			{
				std::string output__;

				for (size_t i = 0; i < this->Size; i++)
				{
					output__ += Data_[i];
				}

				return output__;
			}

			bool GetBool()
			{
				if (Size < 1)throw "Type mismatch";

				bool out;

				memcpy(&out, Data_.data(), sizeof(bool));

				return out;
			}

			int GetInt()
			{
				if (Size < 4)throw "Type mismatch";

				int out;

				memcpy(&out, Data_.data(), sizeof(int));

				return out;
			}

			ULONGLONG GetULONGLONG()
			{
				if (Size < sizeof(ULONGLONG))throw "Type mismatch";

				ULONGLONG out;

				memcpy(&out, Data_.data(), sizeof(ULONGLONG));

				return out;
			}
		};


	public:
		std::string Username;
		std::string Message;
		ULONGLONG EncryptionKey = 0;
		size_t PackedMsgSize = 0;
		bool ShouldUseMessage = false;

	private:
		void* MsgCreated = nullptr;

	public:

		void UnpackMessage(const char* MessagePacked, size_t PackedMsgSize_)
		{
			this->PackedMsgSize = PackedMsgSize_;

			Username = "";
			Message = "";

			ULONGLONG DataCounter = 0;
			std::vector<BYTE> bytearray;

			size_t len = PackedMsgSize;

			for (size_t i = 0; i < len; i++)
			{
				bytearray.push_back(MessagePacked[i]);
			}

			auto type = ByteType(&bytearray, DataCounter);

			Username = type.GetString();

			type = ByteType(&bytearray, DataCounter);

			Message = type.GetString();

			if (DataCounter < bytearray.size() && Message == "") {
				type = ByteType(&bytearray, DataCounter);

				EncryptionKey = type.GetULONGLONG() - 19202941;
			}

			DecryptMessage();
		}

		const char* PackMessage() {
			std::string PackedData = "";

			std::vector<BYTE> Data;

			Data.reserve(4);

			{//String 1

				int Size__ = Username.size();

				auto bytesize = (char*)&Size__;

				for (size_t i = 0; i < 4; i++)
				{
					Data.push_back(bytesize[i]);
				}

				for (size_t i = 0; i < Data.size(); i++)
				{
					PackedData += (char)Data[i];
				}


				for (size_t i = 0; i < Username.length(); i++)
				{
					PackedData += Username[i];
				}

				// PackedData = Username.c_str();
			}

			{//String 2
				Data.clear();


				int Size__ = Message.size();

				auto bytesize = (char*)&Size__;

				for (size_t i = 0; i < 4; i++)
				{
					Data.push_back(bytesize[i]);
				}

				for (size_t i = 0; i < Data.size(); i++)
				{
					PackedData += Data[i];
				}

				for (size_t i = 0; i < Message.length(); i++)
				{
					PackedData += Message[i];
				}

			}


			if (this->MsgCreated) {
				delete[](const char*)this->MsgCreated;
				this->MsgCreated = nullptr;
			}

			const char* output = new char[PackedData.length()];

			std::memcpy((void*)output, PackedData.data(), PackedData.length());

			this->MsgCreated = (void*)output;

			PackedMsgSize = PackedData.size();

			return output;
		}

		std::string XorString(const std::string& input, const ULONGLONG& key) {
			std::string result;
			for (size_t i = 0; i < input.size(); ++i) {
				result.push_back(input[i] ^ (key >> (i % sizeof(ULONGLONG))));
			}
			return result;
		}

		void DecryptMessage()
		{
			if (EncryptionKey != 0) {
				this->ShouldUseMessage = true;
				//Add Decrpyting algorithm here!

				Message = XorString(Message, EncryptionKey);
				Username = XorString(Username, EncryptionKey);
			}

		}

		void EncryptMessage()
		{
			if (EncryptionKey != 0) {
				//Add Encrypting algorithm here!

				Message = XorString(Message, EncryptionKey);
				Username = XorString(Username, EncryptionKey);
			}

		}

		~Message_()
		{
			if (this->MsgCreated)
				delete[](const char*)this->MsgCreated;
		}
	};




	Message_ msg;
	std::string Username = "";
	static Server* _thiss;


public:
	std::string IPServer = "";
	std::string HostIP = "";
	std::vector<std::string> Macs;

	SOCKET ConnectSocket;

	Server(std::string Username) {
		this->Username = Username;
		_thiss = this;
	};

	~Server();

	bool Ended = false;
	
	void MoveLines(int x, int y) {
		auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		auto consoleInfo = GetScreenBufferInfo(hStdout);

		SetConsoleCursorPosition(hStdout, COORD(consoleInfo.dwCursorPosition.X + x, consoleInfo.dwCursorPosition.Y + y));

	}

	CONSOLE_SCREEN_BUFFER_INFO GetScreenBufferInfo(HANDLE ConsoleHandle) {

		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		GetConsoleScreenBufferInfo(ConsoleHandle, &consoleInfo);

		return consoleInfo;
	}

	void SendServerLoop() {
		do
		{
			fflush(stdin);

			// Send an initial buffer
			std::string Usf;
	
			std::getline(std::cin, Usf);

			MoveLines(0, -1);

			for (size_t i = 0; i < Usf.length(); i++)
			{
				std::cout << " ";
			}

			MoveLines(-(int)Usf.length(), 0);
		    

			/*if (Usf == "Usercount") {
			
				Msg = Usf;
			}
			else
			{
				if (Usf.size() > DEFAULT_BUFLEN)
				{
					Msg = "[*] " + Username + ": Hi!\n";
				}
				else
				{
					Msg = "[*] " + Username + ": " + Usf + "\n";
				}
			}*/

			if (Usf.size() > DEFAULT_BUFLEN)
			{
				msg.Message = "Upssss bufferoverflow";
				msg.Username = Username;
			}
			else
			{
				msg.Message = Usf;
				msg.Username = Username;
				//Msg = "[*] " + Username + ": " + Usf + "\n";
			}

			const char* Msg = msg.PackMessage();

			
			auto iResult = send(ConnectSocket, Msg, msg.PackedMsgSize, 0);

			if (iResult == SOCKET_ERROR) {
				Ended = true;
				printf("Connection Lost....\n");
				closesocket(ConnectSocket);
				WSACleanup();
				exit(0);
			}

		} while (!Ended);
	}

	void ReceiveMsg() {
		// Receive until the peer closes the connection
		do {
			char Buffer[DEFAULT_BUFLEN];
			auto iResult = recv(ConnectSocket, Buffer, DEFAULT_BUFLEN, 0);

			if (iResult > DEFAULT_BUFLEN) {
				std::cout << "Couldnt Receive Msg: " << "BufferOverflow try detected\n";
			}

			if (iResult > 0)
			{
				const char* PackedData = Buffer;

				msg.UnpackMessage(PackedData, iResult);


				if (msg.ShouldUseMessage) {
					if (auto ConsoleWindow = GetConsoleWindow(); ConsoleWindow != GetForegroundWindow()) {
						SetForegroundWindow(ConsoleWindow);
					}

					auto queueSize = Message_::EncryptedQueue.size();

					if (queueSize != 0) {
						for (size_t i = 0; i < queueSize; i++)
						{
							auto msg_Enc = Message_::EncryptedQueue[i];

							msg_Enc.EncryptionKey = msg.EncryptionKey;

							msg_Enc.DecryptMessage();


							if(msg_Enc.Message != "")
							std::cout << std::format("[*] {}: {} \n", msg_Enc.Username, msg_Enc.Message);
						}

						Message_::EncryptedQueue.clear();
					}

					if (msg.Message != "")
					std::cout << std::format("[*] {}: {} \n", msg.Username, msg.Message);



				}				
				else
				{
					Message_::EncryptedQueue.push_back(msg);

					msg.Message = "";
					msg.Username = Username;
				    const char* Msg = msg.PackMessage();


				  auto iResult = send(ConnectSocket, Msg, msg.PackedMsgSize, 0);

				  if (iResult == SOCKET_ERROR) {
					Ended = true;
					printf("Connection Lost....\n");
					closesocket(ConnectSocket);
					WSACleanup();
					exit(0);
				  }
				}

			}
			else if (iResult == 0) {
				printf("Connection Lost....\n");
				Ended = true;
			}
			else {
				printf("Connection Lost....\n");
				Ended = true;
			}

		} while (!Ended);
	}

	void OpenServer() {
	}

	void GetMessages();
	void BindToServer();

	static void getMacAddress(std::string ipAddress) {
		ULONG macAddr[2];
		ULONG macAddrLen = 6;

		IPAddr destinationIpAddr = inet_addr(ipAddress.c_str());

		DWORD ret = SendARP(destinationIpAddr, 0, macAddr, &macAddrLen);

		if (ret == NO_ERROR) {
			//printf("MAC addr : %02X:%02X:%02X:%02X:%02X:%02X\n", ipAddress, static_cast<unsigned char>(macAddr[0] & 0xFF), static_cast<unsigned char>((macAddr[0] >> 8) & 0xFF), static_cast<unsigned char>((macAddr[0] >> 16) & 0xFF), static_cast<unsigned char>((macAddr[0] >> 24) & 0xFF), static_cast<unsigned char>((macAddr[1]) & 0xFF), static_cast<unsigned char>((macAddr[1] >> 8) & 0xFF));
			
			std::string MacAddr = ipAddress + ": ";

			{
				MacAddr += std::format("{:x}", static_cast<unsigned char>(macAddr[0] & 0xFF)) + ":";
				MacAddr += std::format("{:x}", static_cast<unsigned char>((macAddr[0] >> 8) & 0xFF)) + ":";
				MacAddr += std::format("{:x}", static_cast<unsigned char>((macAddr[0] >> 16) & 0xFF)) + ":";
				MacAddr += std::format("{:x}", static_cast<unsigned char>((macAddr[0] >> 24) & 0xFF)) + ":";
				MacAddr += std::format("{:x}", static_cast<unsigned char>((macAddr[1]) & 0xFF)) + ":";
				MacAddr += std::format("{:x}", static_cast<unsigned char>((macAddr[1] >> 8) & 0xFF));
			}


			_thiss->Macs.push_back(MacAddr);
		}
		

		
		
		return;
	}

	std::vector<std::thread*> ThreadList;
	std::vector<std::thread*> ThreadListCopy;

	bool ThreadsEmpty() {
		int Threads = 0;

		for (size_t i = 0; i < ThreadListCopy.size(); i++)
		{
			if (ThreadListCopy[i] == nullptr || ThreadListCopy[i]->joinable()) {
				Threads++;
				continue;
			}


			std::thread* TempPtr = ThreadListCopy[i];
			delete TempPtr;
		}

		for (size_t i = 0; i < ThreadList.size(); i++)
		{
			if (ThreadList[i] == nullptr || ThreadList[i]->joinable()) {
				Threads++;
				continue;
			}


			std::thread* TempPtr = ThreadList[i];
			delete TempPtr;
		}

		return Threads == 0;
	}

	template<typename Function, class... _Args>
	std::thread* CreateThread_s(Function* Location, _Args&&... _Ax) {

		for (size_t i = 0; i < ThreadListCopy.size(); i++)
		{
			if (ThreadListCopy[i] == nullptr || ThreadListCopy[i]->joinable())continue;


			std::thread* TempPtr = ThreadListCopy[i];
			delete TempPtr;
		}

		for (size_t i = 0; i < ThreadList.size(); i++)
		{
			if (ThreadList[i] == nullptr || ThreadList[i]->joinable()) {
				ThreadListCopy.push_back(ThreadList[i]);
				continue;
			}

			
			std::thread* TempPtr = ThreadList[i];
			delete TempPtr;
		}

		ThreadList.clear();


		std::thread* NewThread = new std::thread(Location, _Ax...);
		ThreadList.push_back(NewThread);

		return NewThread;
	}


	void FindDevices() {
		int dotState = 0;

		for (size_t i = 0; i < 255; i++)
		{
			if (dotState >= 3) {
				dotState = 0;
				MoveLines(-3, 0);
				std::cout << "   ";
				MoveLines(-3, 0);
			}
			else
			{
				std::cout << ".";
				dotState++;
			}

			std::string Ip = IPServer + "." + std::to_string(i);
			CreateThread_s(&Server::getMacAddress, Ip);
			Sleep(50);
		}


		return;
	}
};

Server* Server::_thiss;


void ToLower(std::string *this_) {

	for (size_t i = 0; i < this_->size(); i++)
	{
		this_[0][i] = std::tolower(this_[0][i]);
	}
}

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	std::string Username = "";
    std::cout << "Enter Username (11 Letters max):";
	std::getline(std::cin, Username);


	int StateUsername = ((Username.size() > 11) ? -1 : 0) + ((Username.size() <= 0) ? -2 : 0);

	while (StateUsername < 0) {

		std::cout << "Username was " << (StateUsername == -1 ? "Too long" : "Too short") << "\n";
		Sleep(1000);

		system("cls");
		std::cout << "Enter Username (11 Letters max):";
		std::getline(std::cin, Username);

		StateUsername = ((Username.size() > 11) ? -1 : 0) | ((Username.size() < 0) ? -2 : 0);
	}

	Server* server = new Server(Username);
	
	char ac[80];
	gethostname(ac, sizeof(ac));

	struct hostent* phe = gethostbyname(ac);
	if (phe == 0) {
		return 1;
	}

	std::string NewIP = "";
	for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		server->IPServer = inet_ntoa(addr);
		int Length = server->IPServer.find_last_of(".");
		
		for (size_t i = 0; i < Length; i++)
		{
			NewIP += server->IPServer[i];
		}
	}

	server->IPServer = NewIP;

	std::cout << "Searching for Host";
	server->FindDevices();
	while (server->ThreadsEmpty())
	{
		Sleep(200);
	}

	std::cout << "\n";

	for (size_t i = 0; i < server->Macs.size(); i++)
	{
		if (server->Macs[i].find("dc:46:28:a1:ed:84") != std::string::npos) {
			int length = server->Macs[i].find(":") - 1;

			std::string IP = "";

			for (size_t i2 = 0; i2 < length + 1; i2++)
			{
				IP += server->Macs[i][i2];
			}
			
			server->HostIP = IP;

			std::cout << "Host Found: " << IP << "\n";

			break;
		}
	}

	if (server->HostIP == "") {
		std::cout << "Host wasn't Found, Enter the IP: ";

		std::string Input;

		std::getline(std::cin, Input);

		server->HostIP = Input;
	}


	system("cls");


	server->ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;



	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	auto iResult = getaddrinfo(server->HostIP.c_str(), DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		WSACleanup();
		return 1;
	}


	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		server->ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (server->ConnectSocket == INVALID_SOCKET) {
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(server->ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(server->ConnectSocket);
			server->ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}


	freeaddrinfo(result);

	if (server->ConnectSocket == INVALID_SOCKET) {
		WSACleanup();
		return 1;
	}


	std::thread GetMsgThread(&Server::ReceiveMsg, server);
	std::thread SendMsgThread(&Server::SendServerLoop, server);
	


	GetMsgThread.join();


	system("pause");

	WSACleanup();
	return 1;

}




std::vector<Server::Message_> Server::Message_::EncryptedQueue;