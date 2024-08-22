#pragma once
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

typedef long long ll;

class bigint
{
public:
	int size();
	bigint operator ^(const bigint& v);
	string to_string();
	int sumof();
	bigint() : sign(1) {}
	bigint(long long v) { *this = v; }
	bigint(const string& s) { read(s); }

	// Operator declerations
	void operator=(const bigint& v);
	void operator=(long long v);
	bigint operator+(const bigint& v) const;
	bigint operator-(const bigint& v) const;
	void operator*=(int v);
	bigint operator*(int v) const;
	void operator*=(long long v);
	bigint operator*(long long v) const;
	bigint operator/(const bigint& v) const;
	bigint operator%(const bigint& v) const;
	void operator/=(int v);
	bigint operator/(int v) const;
	int operator%(int v) const;
	void operator+=(const bigint& v);
	void operator-=(const bigint& v);
	void operator*=(const bigint& v);
	void operator/=(const bigint& v);
	bool operator<(const bigint& v) const;
	bool operator>(const bigint& v) const;
	bool operator<=(const bigint& v) const;
	bool operator>=(const bigint& v) const;
	bool operator==(const bigint& v);
	bool operator!=(const bigint& v) const;

	void trim();
	bool isZero() const;
	bigint operator-() const;
	bigint abs() const;
	long long longValue() const;
	void read(const string& s);
	friend istream& operator>>(istream& stream, bigint& v);
	friend ostream& operator<<(ostream& stream, const bigint& v);
	typedef vector<long long> vll;
	bigint operator*(const bigint& v) const;

	friend pair<bigint, bigint> divmod(const bigint& a1, const bigint& b1);

	static vector<int> convert_base(const vector<int>& a, int old_digits, int new_digits);

	typedef vector<long long> vll;
	static vll karatsubaMultiply(const vll& a, const vll& b);

private:
	vector<int> a;
	int sign;
};