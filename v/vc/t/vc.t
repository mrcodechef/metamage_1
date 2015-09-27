#!/usr/bin/env jtest

$ vc 0
1 >= 0

%

$ vc 98765432109876543210
1 >= 98765432109876543210

%

$ vc -x 0
1 >= ""

%

$ vc -x 98765432109876543210
1 >= 055aa54d38e5267eea

%

$ vc 1+2
1 >= 3

%

$ vc 4-3
1 >= 1

%

$ vc 3-4
1 >= -1

%

$ vc '3 * 4'
1 >= 12

%

$ vc '42 * 98765432109876543210'
1 >= 4148148148614814814820

%

$ vc '3 + 4 * 2'
1 >= 11

%

$ vc '(3)'
1 >= 3

%

$ vc '((3))'
1 >= 3

%

$ vc '(3 + 4) * 2'
1 >= 14

%

$ vc '7 / 3'
1 >= 2

%

$ vc -- '-7 / 3'
1 >= -2

%

$ vc '7 / -3'
1 >= -2

%

$ vc -- '-7 / -3'
1 >= 2

%

$ vc '7 % 3'
1 >= 1

%

$ vc -- '-7 % 3'
1 >= -1

%

$ vc '7 % -3'
1 >= 1

%

$ vc -- '-7 % -3'
1 >= -1

%

$ vc '7 div 3'
1 >= 2

%

$ vc -- '-7 div 3'
1 >= -2

%

$ vc '7 div -3'
1 >= -2

%

$ vc -- '-7 div -3'
1 >= 2

%

$ vc '7 mod 3'
1 >= 1

%

$ vc -- '-7 mod 3'
1 >= 2

%

$ vc '7 mod -3'
1 >= -2

%

$ vc -- '-7 mod -3'
1 >= -1

%

$ vc 2^2
1 >= 4

%

$ vc -- -2^2
1 >= -4

%

$ vc 2^3
1 >= 8

%

$ vc 3^2
1 >= 9

%

$ vc 2^2^2
1 >= 16

%

$ vc 2^2^3
1 >= 256

%

$ vc 3^3^3
1 >= 7625597484987

%

$ vc 2^32-1
1 >= 4294967295

%

$ vc 2^64
1 >= 18446744073709551616

%

$ vc '9876543210987654321098765432109876543210 / 7'
1 >= 1410934744426807760156966490301410934744

%

$ vc '9876543210987654321098765432109876543210 % 7'
1 >= 2
