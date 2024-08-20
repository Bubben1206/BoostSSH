#define _CRT_SECURE_NO_WARNINGS
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


/*

TODO:
    -Decryption
    -Encryption

*/

using namespace std;

typedef long long ll;
const int maxn = 1e2 + 14, lg = 15;


const int base = 1000000000;
const int base_digits = 9;

struct bigint
{
	vector<int> a;
	int sign;
	/*<arpa>*/
	int size()
	{
		if (a.empty())return 0;
		int ans = (a.size() - 1) * base_digits;
		int ca = a.back();
		while (ca)
			ans++, ca /= 10;
		return ans;
	}
	bigint operator ^(const bigint& v)
	{
		bigint ans = 1, a = *this, b = v;
		while (!b.isZero())
		{
			if (b % 2)
				ans *= a;
			a *= a, b /= 2;
		}
		return ans;
	}
	string to_string()
	{
		stringstream ss;
		ss << *this;
		string s;
		ss >> s;
		return s;
	}
	int sumof()
	{
		string s = to_string();
		int ans = 0;
		for (auto c : s)  ans += c - '0';
		return ans;
	}
	/*</arpa>*/
	bigint() : sign(1) {}

	bigint(long long v)
	{
		*this = v;
	}

	bigint(const string& s)
	{
		read(s);
	}

	void operator=(const bigint& v)
	{
		sign = v.sign;
		a = v.a;
	}

	void operator=(long long v)
	{
		sign = 1;
		a.clear();
		if (v < 0)
			sign = -1, v = -v;
		for (; v > 0; v = v / base)
			a.push_back(v % base);
	}

	bigint operator+(const bigint& v) const
	{
		if (sign == v.sign)
		{
			bigint res = v;

			for (int i = 0, carry = 0; i < (int)max(a.size(), v.a.size()) || carry; ++i)
			{
				if (i == (int)res.a.size())
					res.a.push_back(0);
				res.a[i] += carry + (i < (int)a.size() ? a[i] : 0);
				carry = res.a[i] >= base;
				if (carry)
					res.a[i] -= base;
			}
			return res;
		}
		return *this - (-v);
	}

	bigint operator-(const bigint& v) const
	{
		if (sign == v.sign)
		{
			if (abs() >= v.abs())
			{
				bigint res = *this;
				for (int i = 0, carry = 0; i < (int)v.a.size() || carry; ++i)
				{
					res.a[i] -= carry + (i < (int)v.a.size() ? v.a[i] : 0);
					carry = res.a[i] < 0;
					if (carry)
						res.a[i] += base;
				}
				res.trim();
				return res;
			}
			return -(v - *this);
		}
		return *this + (-v);
	}

	void operator*=(int v)
	{
		if (v < 0)
			sign = -sign, v = -v;
		for (int i = 0, carry = 0; i < (int)a.size() || carry; ++i)
		{
			if (i == (int)a.size())
				a.push_back(0);
			long long cur = a[i] * (long long)v + carry;
			carry = (int)(cur / base);
			a[i] = (int)(cur % base);
			//asm("divl %%ecx" : "=a"(carry), "=d"(a[i]) : "A"(cur), "c"(base));
		}
		trim();
	}

	bigint operator*(int v) const
	{
		bigint res = *this;
		res *= v;
		return res;
	}

	void operator*=(long long v)
	{
		if (v < 0)
			sign = -sign, v = -v;
		if (v > base)
		{
			*this = *this * (v / base) * base + *this * (v % base);
			return;
		}
		for (int i = 0, carry = 0; i < (int)a.size() || carry; ++i)
		{
			if (i == (int)a.size())
				a.push_back(0);
			long long cur = a[i] * (long long)v + carry;
			carry = (int)(cur / base);
			a[i] = (int)(cur % base);
			//asm("divl %%ecx" : "=a"(carry), "=d"(a[i]) : "A"(cur), "c"(base));
		}
		trim();
	}

	bigint operator*(long long v) const
	{
		bigint res = *this;
		res *= v;
		return res;
	}

	friend pair<bigint, bigint> divmod(const bigint& a1, const bigint& b1)
	{
		int norm = base / (b1.a.back() + 1);
		bigint a = a1.abs() * norm;
		bigint b = b1.abs() * norm;
		bigint q, r;
		q.a.resize(a.a.size());

		for (int i = a.a.size() - 1; i >= 0; i--)
		{
			r *= base;
			r += a.a[i];
			int s1 = r.a.size() <= b.a.size() ? 0 : r.a[b.a.size()];
			int s2 = r.a.size() <= b.a.size() - 1 ? 0 : r.a[b.a.size() - 1];
			int d = ((long long)base * s1 + s2) / b.a.back();
			r -= b * d;
			while (r < 0)
				r += b, --d;
			q.a[i] = d;
		}

		q.sign = a1.sign * b1.sign;
		r.sign = a1.sign;
		q.trim();
		r.trim();
		return make_pair(q, r / norm);
	}

	bigint operator/(const bigint& v) const
	{
		return divmod(*this, v).first;
	}

	bigint operator%(const bigint& v) const
	{
		return divmod(*this, v).second;
	}

	void operator/=(int v) {
		if (v < 0)
			sign = -sign, v = -v;
		for (int i = (int)a.size() - 1, rem = 0; i >= 0; --i)
		{
			long long cur = a[i] + rem * (long long)base;
			a[i] = (int)(cur / v);
			rem = (int)(cur % v);
		}
		trim();
	}

	bigint operator/(int v) const
	{
		bigint res = *this;
		res /= v;
		return res;
	}

	int operator%(int v) const
	{
		if (v < 0)
			v = -v;
		int m = 0;
		for (int i = a.size() - 1; i >= 0; --i)
			m = (a[i] + m * (long long)base) % v;
		return m * sign;
	}

	void operator+=(const bigint& v)
	{
		*this = *this + v;
	}
	void operator-=(const bigint& v)
	{
		*this = *this - v;
	}
	void operator*=(const bigint& v)
	{
		*this = *this * v;
	}
	void operator/=(const bigint& v)
	{
		*this = *this / v;
	}

	bool operator<(const bigint& v) const
	{
		if (sign != v.sign)
			return sign < v.sign;
		if (a.size() != v.a.size())
			return a.size() * sign < v.a.size() * v.sign;
		for (int i = a.size() - 1; i >= 0; i--)
			if (a[i] != v.a[i])
				return a[i] * sign < v.a[i] * sign;
		return false;
	}

	bool operator>(const bigint& v) const
	{
		return v < *this;
	}
	bool operator<=(const bigint& v) const
	{
		return !(v < *this);
	}
	bool operator>=(const bigint& v) const
	{
		return !(*this < v);
	}
	bool operator==(const bigint& v) const
	{
		return !(*this < v) && !(v < *this);
	}
	bool operator!=(const bigint& v) const
	{
		return *this < v || v < *this;
	}

	void trim()
	{
		while (!a.empty() && !a.back())
			a.pop_back();
		if (a.empty())
			sign = 1;
	}

	bool isZero() const
	{
		return a.empty() || (a.size() == 1 && !a[0]);
	}

	bigint operator-() const
	{
		bigint res = *this;
		res.sign = -sign;
		return res;
	}

	bigint abs() const {
		bigint res = *this;
		res.sign *= res.sign;
		return res;
	}

	long long longValue() const
	{
		long long res = 0;
		for (int i = a.size() - 1; i >= 0; i--)
			res = res * base + a[i];
		return res * sign;
	}

	void read(const string& s)
	{
		sign = 1;
		a.clear();
		int pos = 0;
		while (pos < (int)s.size() && (s[pos] == '-' || s[pos] == '+'))
		{
			if (s[pos] == '-')
				sign = -sign;
			++pos;
		}
		for (int i = s.size() - 1; i >= pos; i -= base_digits)
		{
			int x = 0;
			for (int j = max(pos, i - base_digits + 1); j <= i; j++)
				x = x * 10 + s[j] - '0';
			a.push_back(x);
		}
		trim();
	}

	friend istream& operator>>(istream& stream, bigint& v)
	{
		string s;
		stream >> s;
		v.read(s);
		return stream;
	}

	friend ostream& operator<<(ostream& stream, const bigint& v)
	{
		if (v.sign == -1)
			stream << '-';
		stream << (v.a.empty() ? 0 : v.a.back());
		for (int i = (int)v.a.size() - 2; i >= 0; --i)
			stream << setw(base_digits) << setfill('0') << v.a[i];
		return stream;
	}

	static vector<int> convert_base(const vector<int>& a, int old_digits, int new_digits)
	{
		vector<long long> p(max(old_digits, new_digits) + 1);
		p[0] = 1;
		for (int i = 1; i < (int)p.size(); i++)
			p[i] = p[i - 1] * 10;
		vector<int> res;
		long long cur = 0;
		int cur_digits = 0;
		for (int i = 0; i < (int)a.size(); i++)
		{
			cur += a[i] * p[cur_digits];
			cur_digits += old_digits;
			while (cur_digits >= new_digits)
			{
				res.push_back(int(cur % p[new_digits]));
				cur /= p[new_digits];
				cur_digits -= new_digits;
			}
		}
		res.push_back((int)cur);
		while (!res.empty() && !res.back())
			res.pop_back();
		return res;
	}

	typedef vector<long long> vll;

	static vll karatsubaMultiply(const vll& a, const vll& b)
	{
		int n = a.size();
		vll res(n + n);
		if (n <= 32) {
			for (int i = 0; i < n; i++)
				for (int j = 0; j < n; j++)
					res[i + j] += a[i] * b[j];
			return res;
		}

		int k = n >> 1;
		vll a1(a.begin(), a.begin() + k);
		vll a2(a.begin() + k, a.end());
		vll b1(b.begin(), b.begin() + k);
		vll b2(b.begin() + k, b.end());

		vll a1b1 = karatsubaMultiply(a1, b1);
		vll a2b2 = karatsubaMultiply(a2, b2);

		for (int i = 0; i < k; i++)
			a2[i] += a1[i];
		for (int i = 0; i < k; i++)
			b2[i] += b1[i];

		vll r = karatsubaMultiply(a2, b2);
		for (int i = 0; i < (int)a1b1.size(); i++)
			r[i] -= a1b1[i];
		for (int i = 0; i < (int)a2b2.size(); i++)
			r[i] -= a2b2[i];

		for (int i = 0; i < (int)r.size(); i++)
			res[i + k] += r[i];
		for (int i = 0; i < (int)a1b1.size(); i++)
			res[i] += a1b1[i];
		for (int i = 0; i < (int)a2b2.size(); i++)
			res[i + n] += a2b2[i];
		return res;
	}

	bigint operator*(const bigint& v) const
	{
		vector<int> a6 = convert_base(this->a, base_digits, 6);
		vector<int> b6 = convert_base(v.a, base_digits, 6);
		vll a(a6.begin(), a6.end());
		vll b(b6.begin(), b6.end());
		while (a.size() < b.size())
			a.push_back(0);
		while (b.size() < a.size())
			b.push_back(0);
		while (a.size() & (a.size() - 1))
			a.push_back(0), b.push_back(0);
		vll c = karatsubaMultiply(a, b);
		bigint res;
		res.sign = sign * v.sign;
		for (int i = 0, carry = 0; i < (int)c.size(); i++)
		{
			long long cur = c[i] + carry;
			res.a.push_back((int)(cur % 1000000));
			carry = (int)(cur / 1000000);
		}
		res.a = convert_base(res.a, 6, base_digits);
		res.trim();
		return res;
	}


};

bigint decryptRSA(bigint cipher, bigint d, bigint n);
bigint encryptRSA(bigint message, bigint e, bigint n);
void decodeprivatekey(bigint& n, bigint& d, string key);
void decodepublickey(bigint& e, bigint& n, string key);


unsigned char s[256] =
{
	0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
	0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
	0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
	0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
	0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
	0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
	0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
	0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
	0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
	0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
	0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
	0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
	0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
	0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
	0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
	0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

unsigned char inv_s[256] =
{
	0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
	0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
	0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
	0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
	0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
	0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
	0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
	0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
	0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
	0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
	0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
	0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
	0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
	0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
	0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
	0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

unsigned char gmul(unsigned char a, unsigned char b)
{
	unsigned char p = 0;
	while (a && b)
	{
		if (b & 1)
			p ^= a;

		if (a & 0x80)
			a = (a << 1) ^ 0x11b; /* x^8 + x^4 + x^3 + x + 1 */
		else
			a <<= 1;
		b >>= 1;
	}
	return p;
}

unsigned char frcon(unsigned char i)
{
	if (i == 0)
		return 0x8d;
	unsigned char res = 1;
	for (unsigned char x = 1; x < i; x++)
	{
		res = gmul(res, 2);
	}
	return res;
}

void SubWordRotWordXOR(unsigned char* temp_word, unsigned char i)
{
	unsigned char temp = temp_word[0];
	temp_word[0] = temp_word[1];
	temp_word[1] = temp_word[2];
	temp_word[2] = temp_word[3];
	temp_word[3] = temp;

	temp_word[0] = s[temp_word[0]];
	temp_word[1] = s[temp_word[1]];
	temp_word[2] = s[temp_word[2]];
	temp_word[3] = s[temp_word[3]];

	temp_word[0] ^= frcon(i);
	// other 3 bytes are XORed with 0
}

unsigned char* ExpandKey(unsigned char key[16])
{
	unsigned char* expanded_key = new unsigned char[176];

	for (int i = 0; i < 16; i++)
	{
		expanded_key[i] = key[i];
	}

	int bytes_count = 16;
	int rcon_i = 1;
	unsigned char temp[4];

	while (bytes_count < 176)
	{
		for (int i = 0; i < 4; i++)
		{
			temp[i] = expanded_key[i + bytes_count - 4];
		}

		if (bytes_count % 16 == 0) {
			SubWordRotWordXOR(temp, rcon_i++);
		}

		for (unsigned char a = 0; a < 4; a++) {
			expanded_key[bytes_count] = expanded_key[bytes_count - 16] ^ temp[a];
			bytes_count++;
		}
	}

	return expanded_key;
}

void AddSubRoundKey(unsigned char* state, unsigned char* round_key)
{
	for (int i = 0; i < 16; i++)
		state[i] ^= round_key[i];
}

void EncSubBytes(unsigned char* state)
{
	for (int i = 0; i < 16; i++)
		state[i] = s[state[i]];
}

void LeftShiftRows(unsigned char* state)
{
	unsigned char temp_state[16];

	/*
	0 4 8  12	-> 0  4  8  12
	1 5 9  13	-> 5  9  13 1
	2 6 10 14	-> 10 14 2  6
	3 7 11 15	-> 15 3  7  11
	*/

	temp_state[0] = state[0];
	temp_state[1] = state[5];
	temp_state[2] = state[10];
	temp_state[3] = state[15];

	temp_state[4] = state[4];
	temp_state[5] = state[9];
	temp_state[6] = state[14];
	temp_state[7] = state[3];

	temp_state[8] = state[8];
	temp_state[9] = state[13];
	temp_state[10] = state[2];
	temp_state[11] = state[7];

	temp_state[12] = state[12];
	temp_state[13] = state[1];
	temp_state[14] = state[6];
	temp_state[15] = state[11];

	for (int i = 0; i < 16; i++)
	{
		state[i] = temp_state[i];
	}
}

void MixColumns(unsigned char* state)
{
	unsigned char temp_state[16];

	temp_state[0] = (unsigned char)(gmul(state[0], 2) ^ gmul(state[1], 3) ^ state[2] ^ state[3]);
	temp_state[1] = (unsigned char)(state[0] ^ gmul(state[1], 2) ^ gmul(state[2], 3) ^ state[3]);
	temp_state[2] = (unsigned char)(state[0] ^ state[1] ^ gmul(state[2], 2) ^ gmul(state[3], 3));
	temp_state[3] = (unsigned char)(gmul(state[0], 3) ^ state[1] ^ state[2] ^ gmul(state[3], 2));

	temp_state[4] = (unsigned char)(gmul(state[4], 2) ^ gmul(state[5], 3) ^ state[6] ^ state[7]);
	temp_state[5] = (unsigned char)(state[4] ^ gmul(state[5], 2) ^ gmul(state[6], 3) ^ state[7]);
	temp_state[6] = (unsigned char)(state[4] ^ state[5] ^ gmul(state[6], 2) ^ gmul(state[7], 3));
	temp_state[7] = (unsigned char)(gmul(state[4], 3) ^ state[5] ^ state[6] ^ gmul(state[7], 2));

	temp_state[8] = (unsigned char)(gmul(state[8], 2) ^ gmul(state[9], 3) ^ state[10] ^ state[11]);
	temp_state[9] = (unsigned char)(state[8] ^ gmul(state[9], 2) ^ gmul(state[10], 3) ^ state[11]);
	temp_state[10] = (unsigned char)(state[8] ^ state[9] ^ gmul(state[10], 2) ^ gmul(state[11], 3));
	temp_state[11] = (unsigned char)(gmul(state[8], 3) ^ state[9] ^ state[10] ^ gmul(state[11], 2));

	temp_state[12] = (unsigned char)(gmul(state[12], 2) ^ gmul(state[13], 3) ^ state[14] ^ state[15]);
	temp_state[13] = (unsigned char)(state[12] ^ gmul(state[13], 2) ^ gmul(state[14], 3) ^ state[15]);
	temp_state[14] = (unsigned char)(state[12] ^ state[13] ^ gmul(state[14], 2) ^ gmul(state[15], 3));
	temp_state[15] = (unsigned char)(gmul(state[12], 3) ^ state[13] ^ state[14] ^ gmul(state[15], 2));

	for (int i = 0; i < 16; i++)
	{
		state[i] = temp_state[i];
	}
}

unsigned char* Encrypt(unsigned char* plaintext, unsigned char* expanded_key)
{
	unsigned char state[16];
	unsigned char* cipher = new unsigned char[16];

	for (int i = 0; i < 16; i++)
		state[i] = plaintext[i];

	AddSubRoundKey(state, expanded_key);

	for (int i = 1; i <= 9; i++)
	{
		EncSubBytes(state);
		LeftShiftRows(state);
		MixColumns(state);
		AddSubRoundKey(state, expanded_key + (16 * i));
	}

	EncSubBytes(state);
	LeftShiftRows(state);
	AddSubRoundKey(state, expanded_key + 160);

	for (int i = 0; i < 16; i++)
		cipher[i] = state[i];

	return cipher;
}

void InverseMixColumns(unsigned char* state)
{
	unsigned char temp_state[16];

	temp_state[0] = (unsigned char)(gmul(state[0], 14) ^ gmul(state[1], 11) ^ gmul(state[2], 13) ^ gmul(state[3], 9));
	temp_state[1] = (unsigned char)(gmul(state[0], 9) ^ gmul(state[1], 14) ^ gmul(state[2], 11) ^ gmul(state[3], 13));
	temp_state[2] = (unsigned char)(gmul(state[0], 13) ^ gmul(state[1], 9) ^ gmul(state[2], 14) ^ gmul(state[3], 11));
	temp_state[3] = (unsigned char)(gmul(state[0], 11) ^ gmul(state[1], 13) ^ gmul(state[2], 9) ^ gmul(state[3], 14));

	temp_state[4] = (unsigned char)(gmul(state[4], 14) ^ gmul(state[5], 11) ^ gmul(state[6], 13) ^ gmul(state[7], 9));
	temp_state[5] = (unsigned char)(gmul(state[4], 9) ^ gmul(state[5], 14) ^ gmul(state[6], 11) ^ gmul(state[7], 13));
	temp_state[6] = (unsigned char)(gmul(state[4], 13) ^ gmul(state[5], 9) ^ gmul(state[6], 14) ^ gmul(state[7], 11));
	temp_state[7] = (unsigned char)(gmul(state[4], 11) ^ gmul(state[5], 13) ^ gmul(state[6], 9) ^ gmul(state[7], 14));

	temp_state[8] = (unsigned char)(gmul(state[8], 14) ^ gmul(state[9], 11) ^ gmul(state[10], 13) ^ gmul(state[11], 9));
	temp_state[9] = (unsigned char)(gmul(state[8], 9) ^ gmul(state[9], 14) ^ gmul(state[10], 11) ^ gmul(state[11], 13));
	temp_state[10] = (unsigned char)(gmul(state[8], 13) ^ gmul(state[9], 9) ^ gmul(state[10], 14) ^ gmul(state[11], 11));
	temp_state[11] = (unsigned char)(gmul(state[8], 11) ^ gmul(state[9], 13) ^ gmul(state[10], 9) ^ gmul(state[11], 14));

	temp_state[12] = (unsigned char)(gmul(state[12], 14) ^ gmul(state[13], 11) ^ gmul(state[14], 13) ^ gmul(state[15], 9));
	temp_state[13] = (unsigned char)(gmul(state[12], 9) ^ gmul(state[13], 14) ^ gmul(state[14], 11) ^ gmul(state[15], 13));
	temp_state[14] = (unsigned char)(gmul(state[12], 13) ^ gmul(state[13], 9) ^ gmul(state[14], 14) ^ gmul(state[15], 11));
	temp_state[15] = (unsigned char)(gmul(state[12], 11) ^ gmul(state[13], 13) ^ gmul(state[14], 9) ^ gmul(state[15], 14));

	for (int i = 0; i < 16; i++)
		state[i] = temp_state[i];
}

void RightShiftRows(unsigned char* state)
{
	unsigned char temp_state[16];

	/*
	0 4 8  12	-> 0  4  8  12
	1 5 9  13	-> 13 1  5  9
	2 6 10 14	-> 10 14 2  6
	3 7 11 15	-> 7  11 15 3
	*/

	temp_state[0] = state[0];
	temp_state[1] = state[13];
	temp_state[2] = state[10];
	temp_state[3] = state[7];

	temp_state[4] = state[4];
	temp_state[5] = state[1];
	temp_state[6] = state[14];
	temp_state[7] = state[11];

	temp_state[8] = state[8];
	temp_state[9] = state[5];
	temp_state[10] = state[2];
	temp_state[11] = state[15];

	temp_state[12] = state[12];
	temp_state[13] = state[9];
	temp_state[14] = state[6];
	temp_state[15] = state[3];

	for (int i = 0; i < 16; i++)
		state[i] = temp_state[i];
}

void DecSubBytes(unsigned char* state)
{
	for (int i = 0; i < 16; i++)
		state[i] = inv_s[state[i]];
}

unsigned char* Decrypt(unsigned char* cipher, unsigned char* expanded_key)
{
	unsigned char state[16];
	unsigned char* plaintext = new unsigned char[16];

	for (int i = 0; i < 16; i++)
		state[i] = cipher[i];

	AddSubRoundKey(state, expanded_key + 160);
	RightShiftRows(state);
	DecSubBytes(state);

	int numberOfRounds = 9;

	for (int i = 9; i >= 1; i--)
	{
		AddSubRoundKey(state, expanded_key + (16 * i));
		InverseMixColumns(state);
		RightShiftRows(state);
		DecSubBytes(state);
	}

	AddSubRoundKey(state, expanded_key);

	for (int i = 0; i < 16; i++)
		plaintext[i] = state[i];
	return plaintext;
}

unsigned char* string2hex(string text, int n)
{
	unordered_map<char, int> mp;
	for (int i = 0; i < 10; i++)
		mp[i + '0'] = i;
	for (int i = 0; i < 6; i++)
		mp[i + 'a'] = i + 10;
	unsigned char* res = new unsigned char[n / 2];
	for (int i = 0; i < n / 2; i++)
	{
		char c1 = text.at(i * 2);
		char c2 = text.at(i * 2 + 1);
		int b1 = mp[c1];
		int b2 = mp[c2];
		res[i] = 16 * b1 + b2;
	}
	return res;
}

string hex2string(unsigned char* hex, int n)
{
	unordered_map<char, int> mp;
	for (int i = 0; i < 10; i++)
		mp[i] = i + '0';
	for (int i = 0; i < 6; i++)
		mp[i + 10] = i + 'a';
	string res;
	for (int i = 0; i < n; i++)
	{
		int x = hex[i];
		int b1 = mp[x / 16];
		int b2 = mp[x % 16];
		res += string(1, b1) + string(1, b2);
	}
	return res;
}

string hexa(int a)
{
	int q;
	int r;
	int numlength = to_string(a).length();
	const int base = 16;
	string ans;

	while (a)
	{
		r = a % base;
		a -= r;
		a /= base;
		if (r < 10)
			ans.push_back('0' + r);
		else
			ans.push_back(r - 10 + 'a');
	}
	if (ans.length() == 1)
		ans.push_back('0');
	reverse(ans.begin(), ans.end());
	return ans;
}

string stoh(string str)
{
	string ans;
	for (int k : str)
		ans += hexa(k);
	return ans;
}

string hextostring(string hex)
{
	string temp;
	string ans;
	int val;
	for (int k = 0; k < hex.length(); k += 2)
	{
		temp = hex.substr(k, 2);
		//if (temp[0] == 'a')
			//temp = "0a";
		val = stoi(temp, nullptr, 16);
		ans += (char)val;
	}
	return ans;
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

using boost::asio::ip::tcp;

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
	for(int k = 0; k < 16; ++k)
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

	string AESkey;

	while(true)
	{
		try
		{
			boost::asio::io_context io_context;
			tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));
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
        while(true)
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
			//cout << "Decrypted: " << decrypted << endl;
			decrypted = exec(temp5);
			//cout << "Decrypted: " << decrypted << endl;
			//decrypted = "ghsdifughsodifughsodifguhsdoifguhsdouifgsoiudfghusdfgouihdfhguiogiohufdfdigsuhogfdsiuoh";

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