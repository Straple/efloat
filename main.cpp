#include <iostream>
#include <fstream>

#include "efloat.h"

#include "random.h"

void validate_efloat() {
    cout << "validate efloat...\n";

    cout << "arithmetic operators checking... ";
    {
        auto start = clock();
        ld error = 0;

        for (int test = 0; test < 1e5; test++) {
            ld a = get_rnd_range(-1, 1), b = get_rnd_range(-1, 1);
            efloat efloat_a = a, efloat_b = b;
            
#define get_error(a, b) std::abs(1 - (a).cast_to_double() / (b))

#define simulate_operator(oop) error = max(error, get_error(efloat_a oop efloat_b, a oop b));

            simulate_operator(+);
            simulate_operator(-);
            simulate_operator(*);
            simulate_operator(/);

#undef get_error
#undef simulate_operator
        }

        cout << "error: " << error << "\n";
        if (error > 1e-15) {
            cout << "FATAL\n";
            exit(-1);
        }
        cout << "time: " << ld(clock() - start) / CLOCKS_PER_SEC << "s\n";
    }

    cout << "compare operators checking... ";
    {
        auto start = clock();
        int error = 0;
        for (int test = 0; test < 1e6; test++) {
            ld a = get_rnd_range(-1, 1), b = get_rnd_range(-1, 1);
            efloat efloat_a = a, efloat_b = b;

#define simulate_operator(oop) error += (a oop b) != (efloat_a oop efloat_b);

            simulate_operator(==);
            simulate_operator(!=);
            simulate_operator(>);
            simulate_operator(<);
            simulate_operator(<=);
            simulate_operator(>=);
        }
        cout << "error: " << error << "\n";
        if (error != 0) {
            cout << "FATAL\n";
            exit(-1);
        }
        cout << "time: " << ld(clock() - start) / CLOCKS_PER_SEC << "s\n";
    }
}

int main() {
    //ofstream out("output.txt");
    cout << setprecision(10);

    validate_efloat();

    //return 0;

    // 2.2946s
    // efloat a = get_rnd();
    
    // 3.785s
    // efloat a = get_rnd(), b = get_rnd();

    // 5.5716s->4.2s
    // efloat a = get_rnd(), b = get_rnd();
    // a += b;

    // 4.1386s
    // efloat a = get_rnd(), b = get_rnd();
    // a -= b;

    // 13.4162s->10.0442s->8s
    // efloat a = get_rnd(), b = get_rnd();
    // a *= b;

    // 40s
    // efloat a = get_rnd(), b = get_rnd();
    // a /= b;

    // time test
    ld total = 0;
    for (int t = 0; t < 5; t++) {
        auto start = clock();
        for (int i = 0; i < 1e6; i++) {
            efloat a = get_rnd(), b = get_rnd();
            a *= b;
            /*efloat a = get_rnd(), b = get_rnd();
            a /= b;*/
        }
        total += ld(clock() - start) / CLOCKS_PER_SEC;
        cout << ld(clock() - start) / CLOCKS_PER_SEC << "s\n";
    }
    cout << "total: " << total / 5 << "s\n";



    return 0;
}