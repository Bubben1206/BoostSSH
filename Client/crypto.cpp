#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ctime>
#include <numeric>
#include <thread>

using namespace std;



// RSA
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


	// Karatsube Algorithm!!!
	static vll karatsubaMultiply(const vll& a, const vll& b)
	{
		int n = a.size();
		vll res(n + n);
		if (n <= 32) 
		{
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

bigint gcd(const bigint& a, const bigint& b)
{
	return b.isZero() ? a : gcd(b, a % b);
}

bigint modpow(bigint base, bigint exp, bigint mod)
{
	bigint res;
	for (res = 1; exp > 0; exp /= 2)
	{
		if (exp % 2)
			res = (res * base) % mod;
		base = (base * base) % mod;
	}
	return res;
}

string randomString(int bytes)
{
	string res;
	for (int k = 0; k < bytes; ++k)
	{
		if (!k)
		{
			res.push_back(rand() % 9 + 1 + '0');
			continue;
		}
		res.push_back(rand() % 10 + '0');
	}
	return res;
}

string randomStringBIGINT(bigint bytes)
{
	string res;
	for (bigint k = 0; k < bytes; k += 1)
	{
		if (k == 0)
		{
			res.push_back(rand() % 9 + 1 + '0');
			continue;
		}
		res.push_back(rand() % 10 + '0');
	}
	return res;
}

bool isPrime(bigint testnum, int reps)
{
	if (!(testnum % 2))
		return false;
	if (testnum == 2)
		return true;

	bigint exp = 1;
	bigint x = testnum - 1;
	bigint base = 2;

	bigint temp = (x / (base ^ exp));
	exp += 1;
	while (!(temp % 2))
	{
		temp = (x / (base ^ exp));
		exp += 1;
	}
	exp -= 1;
	for (int k = 2; k < 3; ++k)
	{
		bigint a(randomString(testnum.size()));
		while (testnum - 2 < a)
			a.read(randomString(testnum.size()));
		//cout << a << endl;
		bigint m = (x / (base ^ exp));
		bigint res = modpow(a, m, testnum);
		if (res == x || res == 1)
			return true;
		for (int i = 0; i < reps; ++i)
		{
			if (res == x)
				return true;
			else if (res == 1)
				return false;
			else
				res = modpow(res, 2, testnum);
		}
	}
	return false;
}

bigint getcoprime(bigint n, bigint phi)
{
	cout << "Generating random string.\n";
	bigint random(randomString(128));
	cout << "String generated\n";
	bigint temp(randomStringBIGINT(phi.size()));
	while (temp > phi)
	{
		cout << "Random string too big! Generating new string...\n";
		temp.read(randomStringBIGINT(phi.size()));
	}
	if (temp % 2 == 0)
		temp += 1;

	while ((n % temp == 0) || (phi % temp == 0))
		temp += 2;
	cout << "Calculated e.\n";
	return temp;
}


// The first five characters of the public key contains the "e" value, the rest is part of the "n" value
string rsapublicformat(bigint n, bigint e)
{
	string key = e.to_string() + n.to_string();

	return key;
}

string rsaprivateformat(bigint n, bigint d, bool bign)
{
	string privkey;
	if (bign)
		privkey = "1" + n.to_string() + d.to_string();
	else
		privkey = "0" + n.to_string() + d.to_string();
	return privkey;
}

void displayexponent(bigint* exponent)
{
	while (true)
	{
		if (*exponent == 1)
			continue;
		cout << *exponent << endl;
	}
}

bigint extendedEuclid(bigint phi, bigint e, bigint& x, bigint& y)
{
	if (e == 0) {
		x = 1;
		y = 0;
		return phi;
	}

	bigint x1, y1;

	bigint gcd = extendedEuclid(e, phi % e, x1, y1);

	x = y1;
	y = x1 - (phi / e) * y1;

	return gcd;
}

bigint negmod(bigint a, bigint b)
{
	a = a * -1;
	bigint ans = (a % b) - b;
	ans *= -1;
	return ans;
}


string keygenRSA(int bytes, string& privkey)
{
	bigint p;
	bigint q;
	bigint n;
	bigint phi;
	bigint e;
	bigint d;

	string pubkey;

	bool is513bytes;


	string randnum = randomString(bytes);
	while (!isPrime(randnum, 4))
	{
		cout << "Generating p...\n";
		randnum = randomString(bytes);
	}

	cout << "p Found!\n";
	p.read(randnum);

	randnum = randomString(bytes);
	while (!isPrime(randnum, 4))
	{
		cout << "Generating q...\n";
		randnum = randomString(bytes);
	}

	cout << "q Found!\n";
	q.read(randnum);

	n = p * q;
	if (n.size() == 513)
	{
		is513bytes = true;
	}
	else
		is513bytes = false;

	phi = (p - 1) * (q - 1);

	cout << "Calculating e...\n";
	e = 65537;



	cout << "Calculating d...\n";
	bigint x = 1, y = 1;
	extendedEuclid(phi, e, x, y);
	d = negmod(y, phi);

	cout << "d: " << d << endl << endl;
	cout << "n: " << n << endl << endl;

	cout << "Formatting public key... ";
	pubkey = rsapublicformat(n, e);
	cout << "Formatting success, Public key: " << pubkey << endl;

	cout << "Formatting private key... ";
	privkey = rsaprivateformat(n, d, is513bytes);
	cout << "Formatting success, Private key: " << privkey << endl;

	return pubkey;
}

void decodepublickey(bigint& e, bigint& n, string key)
{
	string str = key.substr(0, 5);
	e.read(str);
	str = key.substr(5, key.length());
	n.read(str);
}

void decodeprivatekey(bigint& n, bigint& d, string key)
{
	string str;
	if (key[0] - '0' != 0)
	{
		str = key.substr(1, 513);
		n.read(str);
		str = key.substr(514, key.length());
		d.read(str);
	}
	else if (key[0] - '0' == 0)
	{
		str = key.substr(1, 512);
		n.read(str);
		str = key.substr(513, key.length());
		d.read(str);
	}
}

bigint e(string key)
{
	string str = key.substr(0, 5);
	bigint e(str);
	return e;
}

bigint n(string key)
{
	string str = key.substr(5, key.length());
	bigint n(str);
	return str;
}

bigint encryptRSA(bigint message, bigint e, bigint n)
{
	//e = 65537;
	//n.read("29325750978374407623889381236188432769629747361902917988658536325443167449884560306826391034304575631809249357630107134471788824298920683156683217297408732843634564252674195225479895204744167363935173892008385067223984363598337711439474466378681187770179192974376297329030539278867298789969023042165155725783508830516989597950719499281506629089616640323971780340579255298529006125661228858177178502644576041424693688787517299377999414298499600251473264983309739746274420434991759944521984066657123168666566875679");
	bigint cipher = modpow(message, e, n);
	return cipher;
}

bigint decryptRSA(bigint cipher, bigint d, bigint n)
{
	//d.read("34059521514105717544287412334615846753637454418093634216163894455764409930503581065877253764547157770279274670878637024084939471601334310681814849135748708502465607102194928723967036992909456384535296030701897214959805816861483944091536666049402586146985039743659713400595646783322656097191244037720707239293881929780907565931970127100918968514754100580897250227890644387292010013247378637671659759920784401152321835271410690630276579226220742449032501400165274460979677591745313400819383753320257417770300076385");
	//n.read("29325750978374407623889381236188432769629747361902917988658536325443167449884560306826391034304575631809249357630107134471788824298920683156683217297408732843634564252674195225479895204744167363935173892008385067223984363598337711439474466378681187770179192974376297329030539278867298789969023042165155725783508830516989597950719499281506629089616640323971780340579255298529006125661228858177178502644576041424693688787517299377999414298499600251473264983309739746274420434991759944521984066657123168666566875679");
	bigint message = modpow(cipher, d, n);
	return message;
}
