#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got=$actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 -  5;"
assert 47 "5 + 6 * 7;"
assert 15 "5 * (9 - 6);"
assert 4 "(3 + 5) / 2;"
assert 6 "-2 * -3;"
assert 1 "-2 + 3;"
assert 2 "1 + -2 + 3;"
assert 1 "2 ==2;"
assert 1 "1 == 1;"
assert 0 "2 + 2 == 5;"
assert 0 "1 != 1;"
assert 1 "2 + 2 != 5;"
assert 1 "1 < 2;"
assert 0 "2 < 1;"
assert 0 "1 > 2;"
assert 1 "1 <= 2;"
assert 0 "2 <= 1;"
assert 0 "1 >= 2;"
assert 3 "1; 2; 3;"
assert 1 "int a = 1; a;"
assert 5 "int a = 2; int b = 3; a+b;";
assert 2 "int a, b; a = b = 1; a + b;"
assert 2 "int a, b = 1; a = 1; a + b;"
assert 1 "int foo = 1; foo;"
assert 6 "int a_b = 2; int c12 = 3; a_b*c12;"
assert 1 "return 1; 2; 3;"
assert 2 "1; return 2; 3;"
assert 3 "1; 2; return 3;"
assert 1 "int retur = 1; retur;"
assert 4 "{ 1; {2; 3;} return 4;}"
assert 3 "{ 1; {2; return 3;} 4;}"
assert 5 "{ ;;; return 5;}"
assert 2 "if (0) return 1; return 2;"
assert 2 "if (1-1) return 1; return 2;"
assert 2 "if (1) { 1; return 2;} return 3;"
assert 1 "if (1) return 1; return 2;"
assert 1 "if (2-1) return 1; return 2;"
assert 5 "if (0) { 1; 2; return 3; } else { 4; return 5; }"
assert 3 "if (1) { 1; 2; return 3; } else { 4; return 5; }"
assert 6 "if (1) { 1; 2; if (1) return 6; } else { 4; return 5; }"
assert 55 "int i, j; i = j = 0; for (i = 0; i <= 10; i = i + 1) j = i + j; return j;"
assert 3 "for (;;) return 3; return 5;"
assert 10 "int i = 0; while (i < 10) { i = i+1; } return i;"
assert 3 "int x = 3; return *&x;"
assert 3 "int x = 3; int* y = &x; return *y;"
assert 3 "int x = 3; int* y = &x; int** z = &y; return **z;"
assert 2 "int x = 1; int y = 2; return *(&x + 1);"
assert 1 "int x = 1; int y = 2; return *(&y - 1);"
assert 2 "int x = 1; int y = 2; int z = 3; return &z - &x;"
assert 2 "int x = 1; int* y = &x; *y = 2; return x;"
echo OK
