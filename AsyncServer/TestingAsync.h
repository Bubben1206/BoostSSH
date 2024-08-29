#define _CRT_SECURE_NO_WARNINGS
#include "bigint.h"
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
//#include <Windows.h>
#include <thread>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <numeric>
#include <memory>
#include <utility>

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

string aesencrypt(string plaintext, string keytext);
string aesdecrypt(string keytext, unsigned char* cipher);
string randomString(int bytes);
string getPubKey();
string genAES128key();

bigint asciitobigint(string key);
void decodepublickey(bigint& e, bigint& n, string key);


using boost::asio::ip::tcp;

class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket) : socket_(std::move(socket))
    {

    }

    static std::vector<std::pair<boost::asio::basic_stream_socket<boost::asio::ip::tcp, boost::asio::any_io_executor>&, std::string>> socketid;
    
    void start()
    {
        if (!auth(socket_, pubkey))
            return;
        cout << "Temp0";
        sendAESkey();
        cout << "Temp1";
        sendTime();
        cout << "Temp2";
        do_read();
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

private:
    
    char in = '\0';

    bool auth(tcp::socket& socket, string pubkey)
    {
        string temp;
        //boost::asio::streambuf buffer;
        for (int k = 0; k < 3; ++k)
        {
            boost::asio::read(socket, boost::asio::buffer(&in, 1));
            temp.push_back(in);
            buffer.consume(1);
        }

        int len2 = stoi(temp);
        temp = "";
        for (int k = 0; k < len2; ++k)
        {
            boost::asio::read(socket, boost::asio::buffer(&in, 1));
            temp.push_back(in);
            buffer.consume(1);
        }
        clientpubkey = temp;
        size_t found = pubkey.find(clientpubkey);
        if (found == string::npos)
            return false;
        for (size_t k = found + temp.length(); k < pubkey.length(); ++k)
        {
            if (isdigit(pubkey[k]))
                continue;
            else if (isalpha(pubkey[k]))
                username.push_back(pubkey[k]);
            else
                break;
        }
        cout << endl << endl << username << endl << endl << endl; // Don't ask what this is...
        tmp = decodepublickey(e, n, clientpubkey);
        string randstring = randomString(256);
        bigint randauth(randstring);
        bigint encrypts = encryptRSA(randauth, e, n);
        string length = to_string(encrypts.to_string().length());
        int len = encrypts.to_string().length();
        string encrypted = length + encrypts.to_string();
        boost::asio::write(socket_, boost::asio::buffer(encrypted, encrypted.length()));



        string message;
        for (int k = 0; k < randstring.length(); ++k)
        {
            boost::asio::read(socket_, boost::asio::buffer(&in, 1));
            message.push_back(in);   
            buffer.consume(1);
        }
        if (message == randstring)
        {
            
            
            return true;
        }
        return false;

    }

    void sendAESkey()
    {
        
        bigint e = 1;
        bigint n = 1;
        decodepublickey(e, n, clientpubkey);
        string success = "Auth success";
        cout << success << endl;
        boost::asio::write(socket_, boost::asio::buffer(success, success.length()));
        AESkey = genAES128key();
        cout << "Random AES128 key: " << AESkey << endl;
        cout << "Received AESKey" << std::endl;
        bigint asciikey = asciitobigint(AESkey);
        bigint keyencrypt = encryptRSA(asciikey, e, n);
        string encryptedkey = keyencrypt.to_string();
        string length = to_string(encryptedkey.length());
        string formatkey = length + encryptedkey;
        cout << formatkey << endl;
        boost::asio::write(socket_, boost::asio::buffer(formatkey, formatkey.length()));
        combined = AESkey + username;
        socketid.push_back(make_pair(ref(socket_), combined));
    }

    std::string make_daytime_string()
    {
        using namespace std;
        time_t now = time(0);
        return ctime(&now);
    }

    string message = make_daytime_string();
    int t = 0;
    void sendTime()
    {
        message = aesencrypt(message, AESkey);
        boost::system::error_code ignored_error;
        boost::asio::write(socket_, boost::asio::buffer(message), ignored_error);
    }

    boost::asio::streambuf buffer;
    void do_read()
    {
        
        //cout << "Temp 3";
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, 32), [this, self](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    
                    //cout << data_ << endl;
                    string hexdecrypted = aesdecrypt(AESkey, string2hex(data_, 32));
                    string decrypted = hextostring(hexdecrypted);
                    if(decrypted[0] == '/')
                    {
                        command = true;
                        reverse(decrypted.begin(), decrypted.end());
                        decrypted.pop_back();
                        reverse(decrypted.begin(), decrypted.end());
                        decrypted.push_back('\0');
                        const char* temp5 = decrypted.c_str();
                        decrypted = exec(temp5);
                        encrypted = aesencrypt(decrypted, AESkey);
                        do_write(encrypted.length(), socket_);
                    }
                    else if (decrypted[0] == '!')
                    {
                        reverse(decrypted.begin(), decrypted.end());
                        decrypted.pop_back();
                        reverse(decrypted.begin(), decrypted.end());
                        decrypted.push_back('\0');
                        for (int k = 0; k < decrypted.length(); ++k)
                        {
                            if (isalpha(decrypted[k]))
                                enteredUsername.push_back(decrypted[k]);
                            else
                                break;
                        }
                        for (auto t : socketid)
                        {
                            readName = "";
                            int length2 = t.second.length();
                            for (int k = 16; k < t.second.length(); k++)
                                readName.push_back(t.second[k]);
                            if (readName == enteredUsername)
                            {
                                command = true;
                                string customAESkey = t.second.substr(0, 16);
                                string temp = decrypted.substr(enteredUsername.length() + 1, decrypted.length());
                                encrypted = aesencrypt(temp, customAESkey);
                                enteredUsername = "";
                                do_write(encrypted.length(), ref(t.first));
                            }
                        }
                    }
                    else
                    {
                        command = false;
                        do_write(32, socket_);
                    }
                }
            });
        
        
        
    }

    void do_write(std::size_t length, tcp::socket& customsocket)
    {
        char* data = new char[length];
        for (int k = 0; k < length; ++k)
        {
            data[k] = command ? encrypted[k] : data_[k];
        }
        auto self(shared_from_this());
        boost::asio::async_write(customsocket, boost::asio::buffer(data, length), [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                    do_read();
                }
            });
    }

    bool decodepublickey(bigint& e, bigint& n, string key)
    {
        string str = key.substr(0, 5);
        e.read(str);
        str = key.substr(5, key.length());
        n.read(str);
        return true;
    }
    string AESkey;
    string pubkey = getPubKey();
    bigint e = 1;
    bigint n = 1;
    bigint d = 1;
    bigint npriv = 1;

    string readName;
    string combined;
    bool tmp;
    string clientpubkey;
    tcp::socket socket_;
    string username;
    string enteredUsername;
    //string data_;
    string encrypted;
    bool command;
    enum { max_length = 2048 };
    char data_[max_length];

    
};

class server
{
public:
    server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<session>(std::move(socket))->start();
                }

                do_accept();
            });
    }

    tcp::acceptor acceptor_;
};
