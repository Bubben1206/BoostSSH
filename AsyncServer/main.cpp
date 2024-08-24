#define _CRT_SECURE_NO_WARNINGS
#include "bigint.h"
#include "TestingAsync.h"
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <Windows.h>
#include <thread>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <numeric>

using namespace std;

using boost::asio::ip::tcp;

bigint decryptRSA(bigint cipher, bigint d, bigint n);
bigint encryptRSA(bigint message, bigint e, bigint n);
void decodeprivatekey(bigint& n, bigint& d, string key);
void decodepublickey(bigint& e, bigint& n, string key);

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
		cout << "Part: " << part_string << " AES128: " << res << endl;
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



std::string make_daytime_string()
{
	using namespace std;
	time_t now = time(0);
	return ctime(&now);
}
void handleClientInput(tcp::socket& socket, string pubkey, string AESkey);


void disconnect(tcp::socket& socket)
{
	boost::system::error_code error;
	socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
	socket.close();
}

string randomString(int bytes);

bool auth(tcp::socket& socket, string pubkey)
{
	boost::asio::streambuf buffer;

	bigint e = 1;
	bigint n = 1;
	decodepublickey(e, n, pubkey);
	string randstring = randomString(256);
	bigint randauth(randstring);
	bigint encrypts = encryptRSA(randauth, e, n);
	string length = to_string(encrypts.to_string().length());
	int len = encrypts.to_string().length();
	string encrypted = length + encrypts.to_string();
	boost::asio::write(socket, boost::asio::buffer(encrypted, encrypted.length()));


	char in;
	string message;
	for (int k = 0; k < randstring.length(); ++k)
	{
		boost::asio::read(socket, boost::asio::buffer(&in, 1));
		message.push_back(in);
		buffer.consume(1);
	}
	if (message == randstring)
		return true;
	return false;

}

string genAES128key()
{
	string ans;
	for (int k = 0; k < 16; ++k)
	{
		int numorchar = rand() % 2;
		int capital = rand() % 2;
		ans.push_back(numorchar ? (capital ? rand() % 27 + 'a' : rand() % 27 + 'A') : rand() % 9 + '1');
	}
	return ans;
}

bigint asciitobigint(string key)
{
	bigint ans;
	string temp;
	for (char c : key)
	{
		temp += to_string(c);
	}
	ans.read(temp);
	return ans;
}

std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;



	auto pipe = _popen(cmd, "r");

	if (!pipe) throw std::runtime_error("popen() failed!");

	while (!feof(pipe))
	{
		if (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
			result += buffer.data();
	}

	auto rc = _pclose(pipe);

	if (rc == EXIT_SUCCESS)
	{ // == 0
		cout << rc << endl;
	}
	else if (rc == EXIT_FAILURE)
	{
		string tmp(cmd);
		return "'" + tmp + "' is not recognized as an internal or external command, operable program or batch file.";
	}
	return result;
}

string getPubKey()
{
	string filePath = "publickey.txt";
	ifstream publickey(filePath);
	if (!publickey)
	{
		cerr << "Could not open file: " << filePath << endl;
		cin.get();
		return "error";
	}
	string pubkey;
	getline(publickey, pubkey);
	publickey.close();
	return pubkey;
}



int main()
{
	string filePath = "publickey.txt";
	ifstream publickey(filePath);
	if (!publickey)
	{
		cerr << "Could not open file: " << filePath << endl;
		cin.get();
		return 1;
	}
	string pubkey;
	getline(publickey, pubkey);
	publickey.close();

	srand(time(nullptr));

	string AESkey;

	try
	{
		boost::asio::io_context io_context;

		server s(io_context, std::atoi("13"));

		io_context.run();

		return 0;

		tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 22));
		tcp::socket socket(io_context);
		acceptor.accept(socket);
		std::string message = make_daytime_string();

		if (auth(socket, pubkey))
		{
			bigint e;
			bigint n;
			decodepublickey(e, n, pubkey);
			string success = "Auth success";
			cout << success << endl;
			boost::asio::write(socket, boost::asio::buffer(success, success.length()));
			AESkey = genAES128key();
			cout << "Random AES128 key: " << AESkey << endl;
			bigint asciikey = asciitobigint(AESkey);
			bigint keyencrypt = encryptRSA(asciikey, e, n);
			string encryptedkey = keyencrypt.to_string();
			string length = to_string(encryptedkey.length());
			string formatkey = length + encryptedkey;
			cout << formatkey << endl;
			boost::asio::write(socket, boost::asio::buffer(formatkey, formatkey.length()));


		}
		else
		{
			string failed = "Auth failed!";
			cout << failed << endl;
			boost::asio::write(socket, boost::asio::buffer(failed, failed.length()));
			disconnect(socket);
		}

		message = aesencrypt(message, AESkey);

		boost::system::error_code ignored_error;
		boost::asio::write(socket, boost::asio::buffer(message), ignored_error); // Writes the time of connection to the client

		std::thread inputHandler(handleClientInput, std::ref(socket), pubkey, AESkey);
		inputHandler.join();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}

void handleClientInput(tcp::socket& socket, string pubkey, string AESkey)
{

	bigint e = 1;
	bigint n = 1;
	bigint d = 1;
	bigint npriv = 1;

	decodepublickey(e, n, pubkey);

	try
	{
		boost::asio::streambuf buffer;
		char in = '\0';
		bool shiftPressed;
		std::string message;
		int length;
		while (true)
		{
			for (int k = 0; k < 32; ++k)
			{
				boost::asio::read(socket, boost::asio::buffer(&in, 1));
				message.push_back(in);
				buffer.consume(1);
			}

			string hexdecrypted = aesdecrypt(AESkey, string2hex(message, 32));
			string decrypted = hextostring(hexdecrypted);
			boost::system::error_code error;
			if (decrypted.substr(0, 10) == "disconnect")
			{
				string disconnectmessage = "Disconnecting!";
				boost::asio::write(socket, boost::asio::buffer(disconnectmessage, disconnectmessage.length()));
				disconnect(socket);
				return;
			}
			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly
			else if (error)
			{
				throw boost::system::system_error(error);
			}
			const char* temp5 = decrypted.c_str();
			decrypted = exec(temp5);


			string encrypted = aesencrypt(decrypted, AESkey);

			boost::asio::write(socket, boost::asio::buffer(encrypted, encrypted.length()));
			message = "";


		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception in client handler: " << e.what() << std::endl;
	}
}