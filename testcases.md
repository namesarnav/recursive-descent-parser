# Test Cases (with purpose)

## Base operators
1. `5` — single literal (smoke test)
2. `(5)` — parentheses only
3. `2+3` — addition
4. `7-4` — subtraction
5. `6*9` — multiplication
6. `8/2` — division

## Combinations and precedence
7. `2*3+5` — `*` binds before `+` (expect 11)
8. `2+3*5` — `*` binds before `+` (expect 17)
9. `57*2-(3+4)` — nested grouping and subtraction (expect 50)
10. `(2+3)*5` — parentheses override precedence (expect 25)

## Exponentiation (right-associative)
11. `2**3` — power (expect 8)
12. `2**3**2` — right-assoc: `2 ** (3 ** 2)` → 512
13. `(2**3)**2` — explicit left-grouping → 64

## Unary ++/-- (prefix & postfix)
14. `++5` — prefix increment on literal → 6
15. `5++` — postfix increment on literal → 6
16. `--10` — prefix decrement → 9
17. `10--` — postfix decrement → 9
18. `++(3+4) * 5--` — both sides use unary → 32
19. `2**++3` — unary inside power → 16

## Chained unary
20. `++--5` — chain prefix, net effect 5
21. `(5++)--` — chain postfix around grouping
22. `++(5++)` — mixed positions around grouping

## Error handling (syntax & runtime)
23. `2+*3` — syntax error: missing operand
24. `((2+3)` — syntax error: missing `)`
25. `2**-1` — runtime error: negative exponent
26. `8/(3-3)` — runtime error: division by zero
