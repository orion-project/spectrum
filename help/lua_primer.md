# Lua Script Primer

Spectrum uses [Lua](http://www.lua.org) internally to calculate custom formulas. Here are some citations from the [original documentation](https://www.lua.org/manual/5.3/) to get a brief intro into the language. Provided information should be sufficient to write formulas for [adding new graphs](./add_formula.md).

Lua is a free-form language. It ignores spaces (including newlines) and comments between lexical elements (tokens), except as delimiters between names and keywords. 


## Variables

Variables are places that store values.

Lua is a case-sensitive language: `and` is a reserved word, but `And` and `AND` are two different valid names.

The following keywords are reserved and cannot be used as names:
`and`, `break`, `do`, `else`, `elseif`, `end`,
`false`, `for`, `function`, `goto`, `if`, `in`,
`local`, `nil`, `not`, `or`, `repeat`, `return`,
`then`, `true`, `until`, `while`.

A numeric value can be written with an optional fractional part and an optional decimal exponent, marked by a letter `e` or `E`.  Examples of valid values are `3`, `3.1416`, `314.16e-2`, `0.31416E1`, `34e1`

Lua allows multiple assignments, e.g.: `x, y = 10, 20` or `x, y, z = y, z, x`.

## Operators

Lua supports the following arithmetic operators:

- `+` — addition
- `-` — subtraction
- `*` — multiplication
- `/` — float division
- `//` — floor division
- `%` — modulo (remainder of a division that rounds the quotient towards minus infinity)
- `^` — exponentiation (it works for non-integer exponents too)
- `-` — unary minus

Lua supports the following relational operators:

- `==` — equality
- `~=` — inequality
- `<` — less than
-  `>` — greater than
- `<=` — less or equal
- `>=` — greater or equal

## Comments

A comment starts with a double hyphen (`--`) and runs until the end of the line. 

## Arrays <a id=lua_array>&nbsp;</a>

Arrays are indexed starting from 1.

Array literals:

```lua
local colors = {"red", "green", "blue"}
```

How to use a loop for populating an array:

```lua
local size = 100
local X = {}
for i = 1, size do
  X[i] = i^2
end
```

Use built-in function for adding elements to the and of array:

```lua
local list = {}
table.insert(list, "first")
table.insert(list, "second")
```

## Mathematical Functions

Lua provides a set of [mathematical functions](https://www.lua.org/manual/5.3/manual.html#6.7) in the `math` library. One has to call them using the library name, e.g., `math.sin(math.pi / 4)`.

For convenience, Spectrum supports its own set of standard mathematical functions that can be used without any additional prefix, e.g., `sin(pi()/4)`. 

- `sin` — returns the sine of the angle in radians.
- `sinh` — returns the hyperbolic sine of the argument.
- `asin` — returns the arcsine of the argument as an angle in radians.
- `cos` — returns the cosine of the angle in radians.
- `cosh` — returns the hyperbolic cosine of the argument.
- `acos` — returns the arccosine of the argument as an angle in radians.
- `tan` — returns the tangent of the angle in radians.
- `tanh` — returns the hyperbolic tangent of the argument.
- `atan` — returns the arctangent of the argument as an angle in radians.
- `cot` — returns the cotangent of the angle in radians.
- `coth` — returns the hyperbolic cotangent of the argument.
- `acot` — returns the arccotangent of the argument as an angle in radians.
- `sec` — returns the secant of the angle in radians.
- `sech` — returns the hyperbolic secant of the argument.
- `csc` — returns the cosecant of the angle in radians.
- `csch` — returns the hyperbolic cosecant of the argument.
- `abs` — returns the absolute value of the argument.
- `floor` — returns the largest integer that is not greater than the argument. For example, `floor(41.2) = 41`.
- `ceil` — returns the smallest integer that is not less than the argument. For example, `ceil(41.2) = 42`.
- `exp` — returns the value of `e` to the power of the argument, where `e` is the base of natural logarithms.
- `ln` — returns the natural logarithm of the argument. Natural logarithm uses base `e`.
- `lg` — returns the logarithm of the argument in base 10.
- `sqrt` — returns the square root of the argument.
- `deg2rad` — converts the angle from degrees to radians.
- `rad2deg` — converts the angle from radians to degrees.
- `pi` — returns value `π`.

## See Also

- [Add Graph From Formula](./add_formula.md)