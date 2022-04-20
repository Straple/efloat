#pragma once

#include <string>
#include <sstream>
#include <iomanip>

const int EFLOAT_MAX_LEN = 300;

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
		while (size() > EFLOAT_MAX_LEN + 10) {
			word.erase(word.begin());
			exp++;
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
		ss << std::setprecision(20);
		std::cout << std::setprecision(20);
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

void efloats_normalize(efloat& a, efloat& b) {
	while (a.exp > b.exp) {
		a.word.insert(a.word.begin(), '0');
		a.exp--;
	}
	while (b.exp > a.exp) {
		b.word.insert(b.word.begin(), '0');
		b.exp--;
	}

	// ���� a == 0 ��� b == 0 => ����� ���� (����� = "000000"), ������� �������� ���:
	a.relax();
	b.relax();

	if (a.exp != b.exp) {
		std::cout << "efloats_normalize failed\n";
	}
}

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

	auto ans = efloat(a.sign == b.sign, s, a.exp + b.exp);
	ans.relax();
	return ans;
}

efloat operator + (efloat a, efloat b) {
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
	efloats_normalize(a, b);
	
	if (a.sign == b.sign) {
		if (efloats_less(a, b)) { // a < b
			return -efloats_base_subtraction(b, a);
		}
		else { // a >= b
			return efloats_base_subtraction(a, b);
		}
	}
	else {
		return efloats_base_addition(a, b);
	}
}

efloat operator / (efloat a, efloat b) {
	efloats_normalize(a, b);
	std::string ans;
	int cnt_exp = 0;

	{
		auto z = b;
		while (!efloats_less(a, z)) { // z <= a
			z.word.insert(z.word.begin(), '0');
			ans.push_back('0');
		}
	}

	while (!efloats_less(a, b)) { // b <= a
		if (ans.size() > EFLOAT_MAX_LEN) {
			break;
		}

		auto z = b;
		int i = 0;
		while (!efloats_less(a, z)) { // z <= a
			z.word.insert(z.word.begin(), '0');
			i++;
		}
		z.word.erase(z.word.begin());
		i--;

		while (!efloats_less(a, z)) { // z <= a
			a = efloats_base_subtraction(a, z);
			ans[i]++;
		}
	}
	while (a.word != "0") {
		if (ans.size() > EFLOAT_MAX_LEN) {
			break;
		}
		cnt_exp--;

		a.word.insert(a.word.begin(), '0');
		ans.insert(ans.begin(), '0');
		while (!efloats_less(a, b)) { // z <= a
			a = efloats_base_subtraction(a, b);
			ans.front()++;
		}
	}

	efloat super_ans(a.sign == b.sign, ans, cnt_exp);
	super_ans.relax();
	return super_ans;
}
