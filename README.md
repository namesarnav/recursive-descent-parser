# Project 1 — Milestone 2 (Recursive Descent Parser in C)
### Arnav Verma 

This program parses and evaluates arithmetic expressions with:
- integers
- parentheses `(` `)`
- binary operators: `+ - * / **` (power is **right-associative**)
- unary operators: prefix and postfix `++` and `--`

For valid inputs, the program prints:
1) the token stream (`lexeme,kind` per line),
2) a textual **Abstract Syntax Tree**, and
3) the evaluated **Value**.

For syntax errors, it prints tokens until the error and then an informative message (with the position).


## Build

```bash
gcc parser.c -o parser
```

## Run

```bash
# via stdin
echo "2*3+5" | ./parser

# via argv
./parser "57*2-(3+4)"
./parser "2**3**2"    # right-associative => 512
./parser "++(3+4) * 5--"
```

#### Additionally, I made the `run.sh` script which you can also use to run. 

## Grammar Implemented

```
expression := term (('+'|'-') term)*
term       := power (('*'|'/') power)*
power      := prefix ('**' power)?          # right-assoc
prefix     := ('++' | '--') prefix | postfix
postfix    := primary ( '++' | '--' )*
primary    := INTEGER | '(' expression ')'
```

### Notes / Assumptions
- Since this milestone uses integers (no variables), `++x` / `x++` evaluates as `x + 1`, and `--x` / `x--` as `x - 1`.
- Division is integer division.
- Division by zero and negative exponents are reported as runtime errors.
