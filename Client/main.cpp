#include "bigint.h"

#include <array>
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <Windows.h>
#include <thread>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <numeric>
#include <unordered_map>

using namespace std;

using boost::asio::ip::tcp;

void thread_readwrite(tcp::socket& socket, string aeskey, string& message);
void writethread1(tcp::socket& socket, string aeskey, string& message);

void decodepublickey(bigint& e, bigint& n, string key);
void decodeprivatekey(bigint& n, bigint& d, string key);
bigint encryptRSA(bigint message, bigint e, bigint n);
bigint decryptRSA(bigint cipher, bigint d, bigint n);

// Convert functions
unsigned char* string2hex(string text, int n);
string hex2string(unsigned char* hex, int n);
string hexa(int a);
string stoh(string str);
string hextostring(string hex);

// AES Functions
unsigned char* ExpandKey(unsigned char key[16]);
unsigned char* Encrypt(unsigned char* plaintext, unsigned char* expanded_key);
unsigned char* Decrypt(unsigned char* cipher, unsigned char* expanded_key);

unsigned char gmul(unsigned char a, unsigned char b);
unsigned char frcon(unsigned char i);
void AddSubRoundKey(unsigned char* state, unsigned char* round_key);
void EncSubBytes(unsigned char* state);
void LeftShiftRows(unsigned char* state);
void MixColumns(unsigned char* state);
void InverseMixColumns(unsigned char* state);
void RightShiftRows(unsigned char* state);
void DecSubBytes(unsigned char* state);

string decryptAES128key(tcp::socket& socket, string privkey)
{
	bigint d;
	bigint n;
	decodeprivatekey(n, d, privkey);

	char in;
	string temp1;
	boost::asio::streambuf buffer;
	for (int k = 0; k < 3; k++)
	{
		boost::asio::read(socket, boost::asio::buffer(&in, 1));
		temp1.push_back(in);
		buffer.consume(1);
	}
	int len = stoi(temp1);
	temp1 = "";
	for (int k = 0; k < len; ++k)
	{
		boost::asio::read(socket, boost::asio::buffer(&in, 1));
		temp1.push_back(in);
		buffer.consume(1);
	}
	bigint ciphertext(temp1);
	bigint decrypts = decryptRSA(ciphertext, d, n);
	string decrypted = decrypts.to_string();
	string aes;
	string temp2;
	int offset;
	string temp3;
	char temp4;
	for (int k = 0; k < decrypted.length();)
	{
		if (decrypted[k] < '4')
		{
			offset = 3;
			temp2 = decrypted.substr(k, offset);
			temp4 = stoi(temp2);
			aes.push_back(temp4);
			k += offset;
			continue;
		}
		if (decrypted[k] > '3')
		{
			offset = 2;
			temp2 = decrypted.substr(k, offset);
			temp4 = stoi(temp2);
			aes.push_back(temp4);
			k += offset;
			continue;
		}
		//cout << "Received AES128 key: " << aes << endl;
		return aes;
	}
}

string aesencrypt(string plaintext, string keytext)
{
	string res;
	string plainhextext = stoh(plaintext);
	string keyhextext = stoh(keytext);

	keytext = keyhextext;
	plaintext = plainhextext;

	if (keyhextext.length() != 32)
		cout << "Key require a 16 byte length!" << endl;

	unsigned char* key = string2hex(keyhextext, 32);
	unsigned char* expanded_key = ExpandKey(key);

	int n = plaintext.length();
	string total_enc = "";
	for (int part = 0; part < (n + 31) / 32; part++)
	{
		int cutoff = min(n, (part + 1) * 32);
		string part_string = plaintext.substr(part * 32, cutoff);
		for (int i = 0; cutoff % 32 != 0 && i < 32 - cutoff % 32; i++)
		{
			part_string += "0";
		}
		unsigned char* padded_string = string2hex(part_string, 32);
		unsigned char* cipher = Encrypt(padded_string, expanded_key);
		res = hex2string(cipher, 16);
		total_enc += res;
		//cout << " AES128: " << res << endl;
	}
	//cout << total_enc << endl;
	return total_enc;
}

string aesdecrypt(string keytext, unsigned char* cipher)
{
	string keyhextext = stoh(keytext);

	keytext = keyhextext;

	if (keyhextext.length() != 32)
		cout << "Key require a 16 byte length!" << endl;

	unsigned char* key = string2hex(keyhextext, 32);
	unsigned char* expanded_key = ExpandKey(key);

	string total_dec = "";
	unsigned char* reverse_cipher = Decrypt(cipher, expanded_key);
	string dec = hex2string(reverse_cipher, 16);
	total_dec += dec;
	total_dec = hextostring(dec);
	//cout << "Decr: " << dec << endl;
	cout << total_dec;

	return dec;
}

void input(string& msg, tcp::socket& socket, string aeskey, string& message)
{
	while (true)
	{
		std::getline(std::cin, msg);
		writethread1(std::ref(socket), aeskey, ref(message));
	}
}

void auth(tcp::socket& socket, string privkey)
{
	boost::asio::streambuf buffer;

	bigint d;
	bigint n;
	decodeprivatekey(n, d, privkey);

	char in;
	string msg;
	for (int k = 0; k < 3; k++)
	{
		boost::asio::read(socket, boost::asio::buffer(&in, 1));
		msg.push_back(in);
		buffer.consume(1);
	}
	int len = stoi(msg);
	msg = "";
	for (int k = 0; k < len; k++)
	{
		boost::asio::read(socket, boost::asio::buffer(&in, 1));
		msg.push_back(in);
		buffer.consume(1);
	}
	bigint cipher(msg);
	bigint decrypts = decryptRSA(cipher, d, n);
	string decrypted = decrypts.to_string();
	boost::system::error_code ignored_error;
	boost::asio::write(socket, boost::asio::buffer(decrypted, decrypted.length()), ignored_error);
	msg = "";
	for (int k = 0; k < 12; k++)
	{
		boost::asio::read(socket, boost::asio::buffer(&in, 1));
		msg.push_back(in);
		buffer.consume(1);
	}
	cout << msg << endl;
}


int main()
{
	bool shiftPress = false;
	string ip;
	cout << "IP Address: ";
	cin >> ip;
	cout << "Port: ";
	string portnum;
	cin >> portnum;

	std::string filePath = "publickey.txt";
	std::ifstream publickey(filePath);
	if (!publickey)
	{
		std::cerr << "Could net open file: " << filePath << std::endl;
		std::cin.get();
		return 1;
	}
	std::string pubkey;
	std::getline(publickey, pubkey);
	publickey.close();
	filePath = "privatekey.txt";
	std::ifstream privatekey(filePath);
	if (!privatekey)
	{
		std::cerr << "Could net open file: " << filePath << std::endl;
		std::cin.get();
		return 1;
	}
	std::string privkey;
	std::getline(privatekey, privkey);
	privatekey.close();

	try
	{

		boost::asio::io_context io_context;

		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints =
			resolver.resolve(ip, portnum);

		tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints);

		auth(socket, privkey);

		string aeskey = decryptAES128key(socket, privkey);
		cout << "Received AES128 key: " << aeskey << endl;

		string message;

		// Read & Write threads
		std::thread readthread(thread_readwrite, std::ref(socket), aeskey, ref(message));
		std::thread threadwrite(input, ref(message), std::ref(socket), aeskey, ref(message));
		//std::thread writethread(writethread1, std::ref(socket), privkey, pubkey, ref(message));
		readthread.join();

	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

// This only reads data from a streambuffer.
void thread_readwrite(tcp::socket& socket, string aeskey, string& message)
{
	char in;
	string msg;
	for (;;)
	{
		boost::asio::streambuf buffer;
		//std::array<char, 128> buf;
		boost::system::error_code error;
		for (int k = 0; k < 32; k++)
		{
			boost::asio::read(socket, boost::asio::buffer(&in, 1));
			msg.push_back(in);
			buffer.consume(1);
		}

		string hexdecrypted = aesdecrypt(aeskey, string2hex(msg, 32));
		string decrypted = hextostring(hexdecrypted);

		msg = "";

		if (error == boost::asio::error::eof)
			break; // Connection closed safely
		else if (error)
			throw boost::system::system_error(error); // Connection closed due to an error
	}
}

// This only write data to a buffer.
void writethread1(tcp::socket& socket, string aeskey, string& message)
{
	boost::system::error_code error;
	if (error == boost::asio::error::eof)
		; // Connection closed safely
	else if (error)
		throw boost::system::system_error(error); // Connection closed due to an error
	boost::system::error_code ignored_error;

	//message.push_back(10);
	string encrypted = aesencrypt(message, aeskey);

	boost::asio::write(socket, boost::asio::buffer(encrypted, encrypted.length()), ignored_error);
}


