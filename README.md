# Lambda: A lambda calculus interpreter

## Features
- A complete lambda calculus grammar with `!` instead of `λ` for programmer convenience
- Beta reduction, alpha renaming
- Variable definitions for organization and readability
- Definition imports from local files
- Library imports (stdlib, bool, math)
- Display functionality for pure functions, booleans, and numerals
- Native numerals support
- Comments

## Examples
```
/* A basic example */
let first = !x.!y.x;
print first a b; /* Prints (a) */
```
```
/* Library imports */
import bool;

print true a b; /* Prints (a) */
printbool true; /* Prints true */
printbool false; /* Prints false */
printbool true false true; /* Prints false */
```
```
/* Numerals */
import math;

print 2; /* Prints (!a2.!a3.a2 (a2 (a3)))*/
printnum 5; /* Prints 5 */
printnum (!a2.!a3.a2 (a2 (a3))); /* Prints 2 */
printnum succ 3; /* Prints 4 */
printnum pred 2; /* Prints 1 */
printnum add 3 5; /* Prints 8 */
```

## Compilation
`make`

## Usage
`./lambda file.lmb`