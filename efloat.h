#pragma once

/*
 * Beta патч
 * Состояние патча:
 * 1) Исправлены баги со сравнением 0 и чисел типа 10e-55, так как тогда
 * программа считала, что 0 > 10e-55, так как у нуля экспонента 0, а у этого
 * числа -55, что намного меньше
 * 2) заменены гнусные C-style cast на C++ static_cast
 * 3) 0 теперь имеет только один вид: -0 нет
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

const int EFLOAT_MAX_LEN = 30;
const int EFLOAT_DOUBLE_LEN = 30;

#define VERIFY(condition, message)                                        \
    if (!(condition)) {                                                   \
        std::cerr << "error:"                                             \
                  << "\nmessage: " << (message) << "\nline: " << __LINE__ \
                  << "\nfile: " << __FILE__ << std::endl;                 \
        exit(EXIT_FAILURE);                                               \
    }

class efloat {
    // true = positive
    // false = negative
    bool sign = true;

    std::string word = "0";

    int exp = 0;

    // x = (sign ? +1 : -1) * word * 10^exp;

    using ld = long double;

    efloat(bool SIGN, const std::string &WORD, int EXP) {
        sign = SIGN;
        word = WORD;
        exp = EXP;
        relax();
    }

    [[nodiscard]] char operator[](size_t index) const {
        return word[index];
    }

    [[nodiscard]] char &operator[](size_t index) {
        return word[index];
    }

    [[nodiscard]] bool is_null() const {
        return word == "0" || word.empty();
    }

    // sets null to standard value
    void update_null() {
        if (is_null()) {
            sign = true;
            word = "0";
            exp = 0;
        }
    }

    void relax() {
        // remove leading zeros
        rlz();
        // remove extra characters
        while (size() > EFLOAT_MAX_LEN) {
            word.erase(word.begin());
            exp++;
        }
        // remove zeros
        while (!word.empty() && word[0] == '0') {
            word.erase(word.begin());
            exp++;
        }
        update_null();
    }

    // remove leading zeros
    void rlz() {
        while (!word.empty() && word.back() == '0') {
            word.pop_back();
        }
        update_null();
    }

    [[nodiscard]] size_t size() const {
        return word.size();
    }

    // word = "303.13e-20"
    // init efloat
    void build_with_word() {
        // -
        if (word[0] == '-') {
            sign = false;
            word.erase(word.begin());
        }
        // e
        for (size_t i = 0; i < word.size(); i++) {
            if (word[i] == 'e') {
                exp += atoi(word.substr(i + 1).c_str());
                word.resize(i);
                break;
            }
        }
        // .
        for (size_t i = 0; i < word.size(); i++) {
            if (word[i] == '.') {
                exp += -static_cast<int>(word.size() - i - 1);
                word.erase(word.begin() + static_cast<long long>(i));
                break;
            }
        }
        reverse(word.begin(), word.end());
        relax();
    }

    // this += rhs
    void base_addition(const efloat &rhs) {
        if (size() < rhs.size()) {
            word.resize(rhs.size(), '0');
        }
        word.push_back('0');

        for (size_t i = 0; i < size(); i++) {
            word[i] -= '0';
            if (i < rhs.size()) {
                word[i] += rhs[i] - '0';
            }
            if (word[i] > 9) {
                word[i + 1]++;
                word[i] -= 10;
            }
            word[i] += '0';
        }
        rlz();
    }

    // this -= rhs
    void base_subtraction(const efloat &rhs) {
        if (size() < rhs.size()) {
            word.resize(rhs.size(), '0');
        }
        word.push_back('0');

        for (size_t i = 0; i < size(); i++) {
            word[i] -= '0';
            if (i < rhs.size()) {
                word[i] -= rhs[i] - '0';
            }
            if (word[i] < 0) {
                word[i + 1]--;
                word[i] += 10;
            }
            word[i] += '0';
        }
        rlz();
    }

    [[nodiscard]] efloat mult_by(int x) const {
        efloat res;
        res.word.pop_back();
        int carry = 0;
        for (size_t i = 0; i < size() || carry; i++) {
            if (i < size()) {
                carry += (word[i] - '0') * x;
            }
            res.word.push_back(static_cast<char>(carry % 10 + '0'));
            carry /= 10;
        }
        return res;
    }

public:
    /*
     * CONSTRUCTORS
     */

    efloat() = default;

    efloat(ld n) {
        std::stringstream ss;
        ss << std::setprecision(EFLOAT_DOUBLE_LEN) << n;
        (*this) = ss.str();
    }

    efloat(const std::string &str) {
        word = str;
        if (word == "-0") {
            word.erase(word.begin());
        }
        build_with_word();
    }

    /*
     * CAST
     */

    [[nodiscard]] std::string cast_to_str() const {
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

    [[nodiscard]] ld cast_to_double() const {
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
        if (!is_null() && x == 0) {  // long double overflowed
            x = std::numeric_limits<ld>::infinity();
            if (!sign) {
                x = -x;
            }
        }
        return x;
    }

    /*
     * ARITHMETIC OPERATIONS
     */

    efloat operator-() const {
        efloat result(!sign, word, exp);
        result.update_null();
        return result;
    }

    efloat &operator*=(const efloat &a) {
        return *this = *this * a;
    }

    efloat &operator+=(const efloat &a) {
        return *this = *this + a;
    }

    efloat &operator-=(const efloat &a) {
        return *this = *this - a;
    }

    efloat &operator/=(const efloat &a) {
        return *this = *this / a;
    }

    /*
     * FRIENDS :)
     */

    friend efloat operator*(const efloat &a, const efloat &b);

    friend efloat operator+(efloat a, efloat b);

    friend efloat operator-(efloat a, efloat b);

    friend efloat operator/(efloat a, const efloat &b);

    friend bool operator==(const efloat &lhs, const efloat &rhs);

    friend bool operator<(const efloat &lhs, const efloat &rhs);

    friend std::istream &operator>>(std::istream &input, efloat &val);

    friend std::ostream &operator<<(std::ostream &output, const efloat &val);

    friend void efloats_normalize(efloat &a, efloat &b);

    friend int efloats_compare(const efloat &lhs, const efloat &rhs);

    friend bool equal_with_high_precision(efloat a, efloat b);
};

/*
 * >>, << OPERATORS
 */

std::istream &operator>>(std::istream &input, efloat &val) {
    input >> val.word;
    val.exp = 0;
    val.sign = true;
    val.build_with_word();
    return input;
}

std::ostream &operator<<(std::ostream &output, const efloat &val) {
    return output << val.cast_to_str();
}

/*
 * EFLOATS BASE FUNCTIONS
 */

// a.exp <=> b.exp
void efloats_normalize(efloat &a, efloat &b) {
    // boosted version
    if (a.exp > b.exp) {
        a.word.insert(a.word.begin(), a.exp - b.exp, '0');
        a.exp = b.exp;
    }
    if (b.exp > a.exp) {
        b.word.insert(b.word.begin(), b.exp - a.exp, '0');
        b.exp = a.exp;
    }
    a.rlz();
    b.rlz();
    // old version
    /*while (a.exp > b.exp) {
            a.word.insert(a.word.begin(), '0');
            a.exp--;
    }
    while (b.exp > a.exp) {
            b.word.insert(b.word.begin(), '0');
            b.exp--;
    }*/
}

// lhs < rhs: <0
// lhs > rhs: >0
// lhs == rhs: 0
[[nodiscard]] int efloats_compare(const efloat &lhs, const efloat &rhs) {
    if (lhs.size() != rhs.size()) {
        return static_cast<int>(lhs.size()) - rhs.size();
    } else {
        for (int i = static_cast<int>(lhs.size()) - 1; i >= 0; i--) {
            if (lhs[i] != rhs[i]) {
                return lhs[i] - rhs[i];
            }
        }
        return 0;
    }
}

/*
 * EFLOATS COMPARE
 */

bool operator==(const efloat &lhs, const efloat &rhs) {
    return lhs.sign == rhs.sign && lhs.exp == rhs.exp && lhs.word == rhs.word;
}

bool operator!=(const efloat &lhs, const efloat &rhs) {
    return !(lhs == rhs);
}

bool operator<(const efloat &lhs, const efloat &rhs) {
    // different signs
    if (lhs.sign != rhs.sign) {
        return !lhs.sign;
    }
    // updating null
    else if (lhs.is_null()) {
        if (rhs.is_null()) {
            return false;
        } else {
            return lhs.sign;
        }
    } else if (rhs.is_null()) {
        return !lhs.sign;
    }
    // more different
    else if (lhs.exp + EFLOAT_MAX_LEN < rhs.exp) {
        return lhs.sign;
    } else if (lhs.exp > rhs.exp + EFLOAT_MAX_LEN) {
        return !lhs.sign;
    }
    // compare words
    else {
        efloat a = lhs, b = rhs;
        efloats_normalize(a, b);
        if (lhs.sign) {
            return efloats_compare(a, b) < 0;
        } else {
            return efloats_compare(a, b) > 0;
        }
    }
}

bool operator>(const efloat &lhs, const efloat &rhs) {
    return rhs < lhs;
}

bool operator<=(const efloat &lhs, const efloat &rhs) {
    return !(lhs > rhs);
}

bool operator>=(const efloat &lhs, const efloat &rhs) {
    return !(lhs < rhs);
}

/*
 * EFLOATS OPERATORS
 */

[[nodiscard]] efloat operator*(const efloat &a, const efloat &b) {
    std::string s(a.size() + b.size() + 1, 0);

    for (size_t i = 0; i < a.size(); i++) {
        int carry = 0;
        for (size_t j = 0; i + j < s.size(); j++) {
            carry += s[i + j];
            if (j < b.size()) {
                carry += static_cast<int>(a[i] - '0') * (b[j] - '0');
            }
            s[i + j] = carry % 10;
            carry /= 10;
        }
    }
    for (size_t i = 0; i < s.size(); i++) {
        s[i] += '0';
    }

    return efloat(a.sign == b.sign, s, a.exp + b.exp);
}

[[nodiscard]] efloat operator+(efloat a, efloat b) {
    if (a.is_null()) {
        return b;
    } else if (b.is_null()) {
        return a;
    }

    // more different exp
    if (abs(a.exp - b.exp) > 3 * EFLOAT_MAX_LEN) {
        if (a.exp > b.exp) {
            return a;
        } else {
            return b;
        }
    }

    efloats_normalize(a, b);

    if (a.sign == b.sign) {
        a.base_addition(b);
    } else if (efloats_compare(a, b) < 0) {  // a < b
        b.base_subtraction(a);
        std::swap(a, b);
    } else {  // a >= b
        a.base_subtraction(b);
    }
    a.relax();
    return a;
}

[[nodiscard]] efloat operator-(efloat a, efloat b) {
    if (a.is_null()) {
        return -b;
    } else if (b.is_null()) {
        return a;
    }

    // more different exp
    if (abs(a.exp - b.exp) > 3 * EFLOAT_MAX_LEN) {
        if (a.exp > b.exp) {
            return a;
        } else {
            return -b;
        }
    }

    efloats_normalize(a, b);

    if (a.sign != b.sign) {
        a.base_addition(b);
    } else if (efloats_compare(a, b) < 0) {  // a < b
        b.base_subtraction(a);
        b.sign = !b.sign;
        std::swap(a, b);
    } else {  // a >= b
        a.base_subtraction(b);
    }
    a.relax();
    return a;
}

// binary search modification. 63s
/*efloat operator / (efloat a, const efloat& b) {
        VERIFY(b.word != "0", "efloat division by zero");
        std::string ans;
        int cnt_exp = a.exp - b.exp;

        auto z = b;
        int i = 0;
        // O(N^2)
        {
                while (efloats_compare(a, z) >= 0) { // a >= z
                        z.word.insert(z.word.begin(), '0'); // insert in begin
O(N) ans.push_back('0'); i++;
                }
        }
        // O(N^2)
        while (efloats_compare(a, b) >= 0) { // a >= b

                z.word.erase(z.word.begin()); // O(N)
                i--;

                int tl = 0, tr = 10;
                while (tl < tr - 1) {
                        int tm = (tl + tr) / 2;
                        efloat x = z.mult_by(tm);
                        if (efloats_compare(a, z.mult_by(tm)) >= 0) {
                                tl = tm;
                        }
                        else {
                                tr = tm;
                        }
                }
                ans[i] += tl;
                a.base_subtraction(z.mult_by(tl));
        }
        // O(N^2)
        while (a.word != "0") {
                if (ans.size() > EFLOAT_MAX_LEN) {
                        break;
                }
                cnt_exp--;

                a.word.insert(a.word.begin(), '0');

                if (efloats_compare(a, b) >= 0 || !ans.empty()) {
                        ans.insert(ans.begin(), '0');

                        int tl = 0, tr = 10;
                        while (tl < tr - 1) {
                                int tm = (tl + tr) / 2;
                                efloat x = b.mult_by(tm);
                                if (efloats_compare(a, b.mult_by(tm)) >= 0) {
                                        tl = tm;
                                }
                                else {
                                        tr = tm;
                                }
                        }
                        ans.front() += tl;
                        a.base_subtraction(b.mult_by(tl));
                }
        }
        return efloat(a.sign == b.sign, ans, cnt_exp);
}*/

// vanila version. 40s
[[nodiscard]] efloat operator/(efloat a, const efloat &b) {
    VERIFY(b.word != "0", "efloat division by zero");
    std::string ans;
    bool ans_sign = a.sign == b.sign;
    int cnt_exp = a.exp - b.exp;

    auto z = b;
    int i = 0;
    // O(N^2)
    {
        while (efloats_compare(a, z) >= 0) {     // a >= z
            z.word.insert(z.word.begin(), '0');  // insert in begin O(N)
            ans.push_back('0');
            i++;
        }
    }
    // O(N^2)
    while (efloats_compare(a, b) >= 0) {  // a >= b

        z.word.erase(z.word.begin());  // O(N)
        i--;

        // O(N)
        while (efloats_compare(a, z) >= 0) {  // a >= z
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

        if (efloats_compare(a, b) >= 0 || !ans.empty()) {
            ans.insert(ans.begin(), '0');
            // O(N)
            while (efloats_compare(a, b) >= 0) {  // a >= b
                a.base_subtraction(b);            // O(N)
                ans.front()++;
            }
        }
    }
    return efloat(ans_sign, ans, cnt_exp);
}

[[nodiscard]] efloat abs(const efloat &x) {
    if (x < 0) {
        return -x;
    } else {
        return x;
    }
}

bool equal_with_high_precision(efloat a, efloat b) {
    efloat diff = abs(a - b);

    if (diff.is_null()) {
        return true;
    }

    if (diff.word.size() == 1 &&
        (diff.exp <= a.exp + static_cast<int>(a.size()) - EFLOAT_MAX_LEN + 3 &&
         diff.exp <= b.exp + static_cast<int>(b.size()) - EFLOAT_MAX_LEN + 3)) {
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] efloat esqrt(const efloat &n) {
    efloat x = 1;
    int step = 0;
    // std::cerr << step << ": " << x << std::endl;
    while (true) {
        efloat nx = (x + n / x) / 2;
        step++;
        // std::cerr << step << ": " << nx << std::endl;
        if (equal_with_high_precision(x, nx)) {
            break;
        }
        x = nx;
    }
    return x;
}

[[nodiscard]] efloat ecbrt(const efloat &n) {
    efloat x = 1;
    while (true) {
        efloat nx = (x + n / (x * x)) / 2;
        if (equal_with_high_precision(x, nx)) {
            break;
        }
        x = nx;
    }
    return x;
}

// TODO: need check
[[nodiscard]] efloat eexp(const efloat &n) {
    efloat result = 0;
    efloat x = 1;
    for (int dep = 0; true; dep++, x /= dep, x *= n) {
        result += x;
        if (x < 1e-9) {
            break;
        }
    }
    return result;
}

// TODO: don't worked
[[nodiscard]] efloat eln(efloat n) {
    n += 1;
    n = 1 / n;

    efloat result = 0;
    efloat x = n;

    for (int step = 1; step < 10; step++, x *= n) {
        result += x / step * (step % 2 == 0 ? -1 : +1);
    }
    return result;
}
