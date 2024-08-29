#include "bigint.h"

#include <array>
#include <string>
#include <iostream>
//#include <Windows.h>
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