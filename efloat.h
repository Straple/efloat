#pragma once

#include <string>
#include <sstream>
#include <iomanip>

const int EFLOAT_MAX_LEN = 30;

class efloat {
	// true = positive
	// false = negative
	bool sign = true;

	std::string word = "0";

	int exp = 0;

	// x = (sign ? +1 : -1) * word * 10^exp;

	using ld = long double;

	efloat(bool SIGN, const std::string& WORD, int EXP) {
		sign = SIGN;
		word = WORD;
		exp = EXP;
		relax();
	}

	int operator [](int index) const {
		return word[index] - '0';
	}
	char& operator [](int index) {
		return word[index];
	}

	void relax() {
		while (!word.empty() && word.back() == '0') {
			word.pop_back();
		}
		if (word.empty()) {
			word = "0";
		}
		while (size() > EFLOAT_MAX_LEN) {
			word.erase(word.begin());
			exp++;
		}
		if (word == "0") {
			exp = 0;
		}
	}

	int size() const {
		return word.size();
	}

public:

	/*
	* CONSTRUCTORS
	*/

	efloat() {}
	efloat(ld n) {
		std::stringstream ss;
		ss << std::setprecision(25);
		std::cout << std::setprecision(25);
		ss << n;
		word = ss.str();
		if (word[0] == '-') {
			sign = false;
			word.erase(word.begin());
		}
		for (int i = 0; i < word.size(); i++) {
			if (word[i] == 'e') {
				exp += atoi(word.substr(i + 1).c_str());
				word.resize(i);
				break;
			}
		}
		for (int i = 0; i < word.size(); i++) {
			if (word[i] == '.') {
				exp += -int(word.size() - i - 1);
				word.erase(word.begin() + i);
				break;
			}
		}
		reverse(word.begin(), word.end());
		relax();
		//std::cout << "efloat(" << n << ") = " << sign << " " << word << " " << exp << " = " << cast_to_str() << "\n";
	}

	/*
	* CAST
	*/

	std::string cast_to_str() const {
		std::stringstream ss;
		if (!sign) {
			ss << '-';
		}
		std::string s = word;
		reverse(s.begin(), s.end());
		ss << s;
		if (exp != 0) {
			ss << 'e' << exp;
		}
		return ss.str();
	}

	ld cast_to_double() const {
		std::stringstream ss;
		if (!sign) {
			ss << '-';
		}
		std::string s = word;
		reverse(s.begin(), s.end());
		int cur_exp = exp;
		while (s.size() > 25) {
			s.pop_back();
			cur_exp++;
		}
		ss << s;
		if (cur_exp != 0) {
			ss << 'e' << cur_exp;
		}
		ld x;
		ss >> x;
		if (word != "0" && x == 0) { // long double overflowed
			x = INFINITY * (sign ? +1 : -1);
		}
		return x;
	}

	/*
	* ARITHMETIC OPERATIONS
	*/

	efloat operator - () const {
		return efloat(!sign, word, exp);
	}

	efloat& operator *= (const efloat& a) {
		return *this = *this * a;
	}
	efloat& operator += (const efloat& a) {
		return *this = *this + a;
	}
	efloat& operator -= (const efloat& a) {
		return *this = *this - a;
	}
	efloat& operator /= (const efloat& a) {
		return *this = *this / a;
	}

	/*
	* FRIENDS :)
	*/

	friend efloat operator * (const efloat& a, const efloat& b);
	friend efloat operator + (efloat a, efloat b);
	friend efloat operator - (efloat a, efloat b);
	friend efloat operator / (efloat a, efloat b);


	friend void efloats_normalize(efloat& a, efloat& b);

	friend bool efloats_less(const efloat& lhs, const efloat& rhs);

	friend efloat efloats_base_addition(efloat a, const efloat& b);

	friend efloat efloats_base_subtraction(efloat a, const efloat& b);
};

/*
* EFLOATS BASE FUNCTIONS
*/

// a.exp = b.exp
void efloats_normalize(efloat& a, efloat& b) {
	while (a.exp > b.exp) {
		a.word.insert(a.word.begin(), '0');
		a.exp--;
	}
	while (b.exp > a.exp) {
		b.word.insert(b.word.begin(), '0');
		b.exp--;
	}
}

// lhs < rhs
bool efloats_less(const efloat& lhs, const efloat& rhs) {
	if (lhs.size() != rhs.size()) {
		return lhs.size() < rhs.size();
	}
	else {
		for (int i = lhs.size() - 1; i >= 0; i--) {
			if (lhs[i] != rhs[i]) {
				return lhs[i] < rhs[i];
			}
		}
		return false; // lhs == rhs
	}
}

// a += b
efloat efloats_base_addition(efloat a, const efloat& b) {
	if (a.size() < b.size()) {
		a.word.resize(b.size(), '0');
	}
	a.word.push_back('0');

	for (int i = 0; i < a.size(); i++) {
		a[i] -= '0';

		if (i < b.size()) {
			a[i] += b[i];
		}
		if (a[i] > 9) {
			a[i + 1] += a[i] / 10;
			a[i] %= 10;
		}
		a[i] += '0';
	}
	a.relax();
	return a;
}

// a -= b
efloat efloats_base_subtraction(efloat a, const efloat& b) {
	if (a.size() < b.size()) {
		a.word.resize(b.size(), '0');
	}
	a.word.push_back('0');

	for (int i = 0; i < a.size(); i++) {
		a[i] -= '0';

		if (i < b.size()) {
			a[i] -= b[i];
		}
		if (a[i] < 0) {
			a[i + 1]--;
			a[i] += 10;
		}
		a[i] += '0';
	}
	a.relax();
	return a;
}

/*
* EFLOATS OPERATORS
*/

efloat operator * (const efloat& a, const efloat& b) {
	std::string s(a.size() + b.size() + 1, 0);

	for (int i = 0; i < a.size(); i++) {
		for (int j = 0; j < b.size(); j++) {
			s[i + j] += a[i] * b[j];
		}
		for (int i = 0; i < s.size(); i++) {
			if (s[i] > 9) {
				s[i + 1] += s[i] / 10;
				s[i] %= 10;
			}
		}
	}
	for (int i = 0; i < s.size(); i++) {
		s[i] += '0';
	}

	return efloat(a.sign == b.sign, s, a.exp + b.exp);
}

efloat operator + (efloat a, efloat b) {
	if (a.word == "0") {
		return b;
	}
	else if (b.word == "0") {
		return a;
	}

	if (abs(a.exp - b.exp) > 3 * EFLOAT_MAX_LEN) {
		if (a.exp > b.exp) {
			return a;
		}
		else {
			return b;
		}
	}

	efloats_normalize(a, b);

	if (a.sign == b.sign) {
		return efloats_base_addition(a, b);
	}
	else if (efloats_less(a, b)) { // a < b
		return efloats_base_subtraction(b, a);
	}
	else { // a >= b
		return efloats_base_subtraction(a, b);
	}
}

efloat operator - (efloat a, efloat b) {
	if (a.word == "0") {
		return -b;
	}
	else if(b.word == "0") {
		return a;
	}

	if (abs(a.exp - b.exp) > 3 * EFLOAT_MAX_LEN) {
		if (a.exp > b.exp) {
			return a;
		}
		else {
			return -b;
		}
	}

	efloats_normalize(a, b);
	
	if (a.sign != b.sign) {
		return efloats_base_addition(a, b);
	}
	else if (efloats_less(a, b)) { // a < b
		return -efloats_base_subtraction(b, a);
	}
	else { // a >= b
		return efloats_base_subtraction(a, b);
	}
}

efloat operator / (efloat a, efloat b) {
	std::string ans;
	int cnt_exp = a.exp - b.exp;

	// O(N^2)
	{
		auto z = b;
		while (!efloats_less(a, z)) { // z <= a
			z.word.insert(z.word.begin(), '0'); // insert in begin O(N)
			ans.push_back('0');
		}
	}
	// O(N^3)
	while (!efloats_less(a, b)) { // b <= a
		auto z = b;
		int i = 0;

		// O(N^2)
		while (!efloats_less(a, z)) { // z <= a
			z.word.insert(z.word.begin(), '0'); // insert in begin O(N)
			i++;
		}
		z.word.erase(z.word.begin());
		i--;

		// O(n)
		while (!efloats_less(a, z)) { // z <= a
			a = efloats_base_subtraction(a, z);
			ans[i]++;
		}
	}
	// O(N^2)
	while (a.word != "0") {
		if (ans.size() > EFLOAT_MAX_LEN) {
			break;
		}
		cnt_exp--;

		a.word.insert(a.word.begin(), '0');

		if (!efloats_less(a, b) || !ans.empty()) {
			ans.insert(ans.begin(), '0');
			// O(N)
			while (!efloats_less(a, b)) { // z <= a
				a = efloats_base_subtraction(a, b); // O(N)
				ans.front()++;
			}
		}
	}

	return efloat(a.sign == b.sign, ans, cnt_exp);
}
