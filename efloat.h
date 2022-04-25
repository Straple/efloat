#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <limits>

const int EFLOAT_MAX_LEN = 30;
const int EFLOAT_DOUBLE_LEN = 30;

#define VERIFY(condition, message)\
if(!(condition)){\
	std::cerr << "error:" << "\nmessage: " << (message) << "\nline: " << __LINE__ << "\nfile: " << __FILE__ << std::endl;\
	exit(EXIT_FAILURE);\
}

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

	const int operator [](int index) const {
		return word[index] - '0';
	}
	char& operator [](int index) {
		return word[index];
	}

	void relax() {
		rlz();
		while (size() > EFLOAT_MAX_LEN || word[0] == '0') {
			word.erase(word.begin());
			exp++;
		}
		if (word == "0") {
			exp = 0;
		}
	}

	void rlz() {
		while (!word.empty() && word.back() == '0') {
			word.pop_back();
		}
		if (word.empty()) {
			word = "0";
		}
	}

	const int size() const {
		return word.size();
	}

	// word = "303.13e20"
	void build_with_word() {
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
	}

	// this += rhs
	void base_addition(const efloat& rhs) {
		if (size() < rhs.size()) {
			word.resize(rhs.size(), '0');
		}
		word.push_back('0');

		for (int i = 0; i < size(); i++) {
			word[i] -= '0';
			if (i < rhs.size()) {
				word[i] += rhs[i];
			}
			if (word[i] > 9) {
				word[i + 1] += word[i] / 10;
				word[i] %= 10;
			}
			word[i] += '0';
		}
		rlz();
	}

	// this -= rhs
	void base_subtraction(const efloat& rhs) {
		if (size() < rhs.size()) {
			word.resize(rhs.size(), '0');
		}
		word.push_back('0');

		for (int i = 0; i < size(); i++) {
			word[i] -= '0';
			if (i < rhs.size()) {
				word[i] -= rhs[i];
			}
			if (word[i] < 0) {
				word[i + 1]--;
				word[i] += 10;
			}
			word[i] += '0';
		}
		rlz();
	}

public:

	/*
	* CONSTRUCTORS
	*/

	efloat() {}
	efloat(ld n) {
		std::stringstream ss;
		ss << std::setprecision(EFLOAT_DOUBLE_LEN) << n;
		(*this) = ss.str();
	}
	efloat(const std::string& str) {
		word = str;
		build_with_word();
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
		while (s.size() > EFLOAT_DOUBLE_LEN) {
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
			if (sign) {
				x = std::numeric_limits<ld>::infinity();
			}
			else {
				x = -std::numeric_limits<ld>::infinity();
			}
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
	friend efloat operator / (efloat a, const efloat& b);

	friend bool operator == (const efloat& lhs, const efloat& rhs);
	friend bool operator <  (const efloat& lhs, const efloat& rhs);

	friend std::istream& operator >> (std::istream& input, efloat& val);
	friend std::ostream& operator << (std::ostream& output, const efloat& val);

	friend void efloats_normalize(efloat& a, efloat& b);

	friend int efloats_compare(const efloat& lhs, const efloat& rhs);
};

/*
* >>, << OPERATORS
*/

std::istream& operator >> (std::istream& input, efloat& val) {
	input >> val.word;
	val.exp = 0;
	val.sign = true;
	val.build_with_word();
	return input;
}

std::ostream& operator << (std::ostream& output, const efloat& val) {
	return output << val.cast_to_str();
}

/*
* EFLOATS BASE FUNCTIONS
*/

// a.exp <=> b.exp
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

// lhs < rhs: -1
// lhs > rhs: +1
// lhs == rhs: 0
int efloats_compare(const efloat& lhs, const efloat& rhs) {
	if (lhs.size() != rhs.size()) {
		return lhs.size() < rhs.size() ? -1 : +1;
	}
	else {
		for (int i = lhs.size() - 1; i >= 0; i--) {
			if (lhs[i] != rhs[i]) {
				return lhs[i] < rhs[i] ? -1 : +1;
			}
		}
		return 0;
	}
}

/*
* EFLOATS COMPARE
*/

bool operator == (const efloat& lhs, const efloat& rhs) {
	return lhs.sign == rhs.sign && lhs.exp == rhs.exp && lhs.word == rhs.word;
}
bool operator != (const efloat& lhs, const efloat& rhs) {
	return !(lhs == rhs);
}
bool operator <  (const efloat& lhs, const efloat& rhs) {
	// different signs
	if (lhs.sign != rhs.sign) {
		return !lhs.sign; 
	}
	// more different
	else if(lhs.exp + EFLOAT_MAX_LEN < rhs.exp) {
		return lhs.sign;
	}
	else if (lhs.exp > rhs.exp + EFLOAT_MAX_LEN) {
		return !lhs.sign;
	}
	else {
		efloat a = lhs, b = rhs;
		efloats_normalize(a, b);
		if (lhs.sign) {
			return efloats_compare(a, b) == -1;
		}
		else {
			return efloats_compare(a, b) == +1;
		}
	}
}
bool operator >  (const efloat& lhs, const efloat& rhs) {
	return rhs < lhs;
}
bool operator <= (const efloat& lhs, const efloat& rhs) {
	return !(lhs > rhs);
}
bool operator >= (const efloat& lhs, const efloat& rhs) {
	return !(lhs < rhs);
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

	// more different exp
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
		a.base_addition(b);
	}
	else if (efloats_compare(a, b) == -1) { // a < b
		b.base_subtraction(a);
		std::swap(a, b);
	}
	else { // a >= b
		a.base_subtraction(b);
	}
	a.relax();
	return a;
}

efloat operator - (efloat a, efloat b) {
	if (a.word == "0") {
		return -b;
	}
	else if(b.word == "0") {
		return a;
	}

	// more different exp
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
		a.base_addition(b);
	}
	else if (efloats_compare(a, b) == -1) { // a < b
		b.base_subtraction(a);
		b = -b;
		std::swap(a, b);
	}
	else { // a >= b
		a.base_subtraction(b);
	}
	a.relax();
	return a;
}

efloat operator / (efloat a, const efloat& b) {
	VERIFY(b.word != "0", "efloat division by zero");
	std::string ans;
	int cnt_exp = a.exp - b.exp;

	auto z = b;
	int i = 0;
	// O(N^2)
	{
		while (efloats_compare(a, z) != -1) { // a >= z
			z.word.insert(z.word.begin(), '0'); // insert in begin O(N)
			ans.push_back('0');
			i++;
		}
	}
	// O(N^2)
	while (efloats_compare(a, b) != -1) { // a >= b

		z.word.erase(z.word.begin()); // O(N)
		i--;

		// O(N)
		while (efloats_compare(a, z) != -1) { // a >= z
			a.base_subtraction(z);
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

		if (efloats_compare(a, b) != -1 || !ans.empty()) {
			ans.insert(ans.begin(), '0');
			// O(N)
			while (efloats_compare(a, b) != -1) { // a >= b
				a.base_subtraction(b); // O(N)
				ans.front()++;
			}
		}
	}
	return efloat(a.sign == b.sign, ans, cnt_exp);
}
