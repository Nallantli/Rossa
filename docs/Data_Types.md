# Data Types

There are 6 base types, not including the two special cases: `Pointer`, for external library usage, and `nil`, the "null" or "undefined" type.

Additionally, user defined classes (referred to as Objects) exist but will be discussed further in a different section.

All data types (even user defined ones) implement the following operators:

Operator|Function|Return
-|-|-
`a = b`|Set Operation - If not overridden, sets the value of `a` to the value of `b`|Value of `a`
`a == b`|Equality Comparison|`Boolean`
`a != b`|Inequality Comparison|`Boolean`
`a === b`|Address Equality Comparison - Functionally equivalent to `==` for all base-types, but for Objects will assess whether the two values point to the same address. Useful generally if the Object contains an override for `==`.|`Boolean`
`a !=== b`|Address Inequality Comparison - See `===`|`Boolean`

## Number

The `Number` type encompasses the traditional designations of integers and floating points. These two datatypes are not differentiated at the language level; the underlying structure of the `Number` type converts automatically between a 64-bit integer and a 64-bit floating point data address.

In this manner, the conversions only take place upon creation, causing very little to negligible overhead during runtime. This also means that during calls to indexing of the `Array` type, if the `Number` is currently holding a floating point value, that floating point value will be floored during casting. For example, an `Array` attempted to be indexed at a value of `1.2` will return the element at index `1`.

### Literals

Numbers can be expressed in three different literal manners:

#### Decimal (Base-10)

This is the most general form; it follows the syntax expressed by a sequence of digits with an optional single radix.

```ruo
1;
1234.56;
0012342.00012; # Note that trailing zeros on both ends are trimmed
```

#### Hexadecimal (Base-16)

Hexadecimal numbers are given by using the digits `0` to `f`, with an initial prefix of `0x`.

```ruo
0xff; # 255
0xabcdef; # 11259375
```

Hexadecimal "digits" are not case-sensitive, nor is the `x` in the prefix. Thus, `0xff` is equivalent to `0XFF`;

#### Binary (Base-2)

Binary numbers are expressed in a similar manner to Hexadecimal, with the prefix `0b` and digits of `0` and `1`.

```ruo
0b01; # 1
0b10; # 2
```

Like with hexadecimal literals, the prefix `b` is not case sensitive.

#### Character

Because of the importance of character values in programming, it is helpful for there to be the possibility of declaring a numerical equivalent for a char-value.

The literal follows the standard representation using single quotes:

```ruo
'a'; # 97
```

Note that while it _looks_ like a character, it is NOT a data-type separate from `Number`. `'a'` and `97` are interpreted equivalently.

### Operators

Operator|Function|Return
-|-|-
`a + b`|Addition|`Number`
`a - b`|Subtraction|`Number`
`a * b`|Multiplication|`Number`
`a / b`|Division|`Number`
`a ** b`|Exponentiation|`Number`
`a % b`|Modulus (Evaluates for floating point numbers too, unlike the more restrictive C/++ modulus)|`Number`
`a < b`|Less Comparison|`Boolean`
`a > b`|More Comparison|`Boolean`
`a <= b`|Less or Equal Comparison|`Boolean`
`a >= b`|More or Equal Comparison|`Boolean`
`a & b`|Bitwise And (Integers)|`Number`
`a \| b`|Bitwise Or (Integers)|`Number`
`a ^ b`|Bitwise XOr (Integers)|`Number`
`a << b`|Left Bitshift (Integers)|`Number`
`a >> b`|Right Bitshift (Integers)|`Number`

## Boolean

`Boolean` values denote a binary construct: `true` or `false`.

### Literals

```ruo
true;
false;
```

### Operators

Operator|Function|Return
-|-|-
`a && b`|Logical And|`Boolean`
`a \|\| b`|Logical Or|`Boolean`

## String

The `String` type is a base value in itself, unlike in other languages where it is given as an array of characters. This consequently means that `String` values are immutable. Likewise, they cannot be iterated over or indexed directly without explicit conversion.

### Literals

Strings are declared with double quotes:

```ruo
"Hello World";
```

### Operators

Operator|Function|Return
-|-|-
`a + b`|String Concatanation|`String`
`a & [b...]`|Templative String Concatanation\*|`String`
`a < b`|Lexical Less Comparison|`Boolean`
`a > b`|Lexical More Comparison|`Boolean`
`a <= b`|Lexical Less or Equal Comparison|`Boolean`
`a >= b`|Lexical More or Equal Comparison|`Boolean`
`size(a)`/`a.size()`|ASCII Char-length of String|`Number`
`length(a)`/`a.length()`|Unicode Char-length of String|`Number`

Note the difference between `size` and `length`: The `String` value `"ルオタ"` has a `size` of `9` (CJK characters take up 3 bytes each), but a `length` of `3`.

\* This operation takes a `String` value and an `Array` of values and inserts the latter into positions defined with `%n%` where `n` is the index in the `Array`. For instance:

```ruo
"{1}, World!" & ["Hello"]; # "Hello, World!"
"{2} is the last name of {1}" & ["John", "Smith"]; # "Smith is the last name of John"
```

This operation is one of the few that does not require explicit conversion, all elements of any type will have their `->String` method called for concatanation.

## Array

An `Array` need not be declared for any particular element type; `Array` values can hold any type or combination of types as a member, including more `Array` values of any size.

### Literals

`Array` literals are declared using square brackets:

```ruo
[1, 2, 3];
[1, "Hello", true, [1, 2, 3]];
```

### Operators

Operator|Function|Return
-|-|-
`a + b`|Array Concatanation|`Array`
`a - b`|Returns an `Array` of elements in `a` that do not exist in `b`|`Array`
`a[b]`|Indexes `a` at (`Number`) `b`|Value at index `b`
`size(a)`/`a.size()`|Element Count|`Number`
`length(a)`/`a.length()`|Equivalent to `size`|`Number`

## Dictionary

`Dictionary` types are key-value maps. When using `=`, a deep copy is made, and new keys may be added at any point after declaration. This is in contrast with `Object` values, which set by reference and cannot be given new members after declaration.

The syntax is designed to be compatible with JSON.

### Literals

`Dictionary` literals are declared using curly brackets, and each value-pair inside is separated by a colon:

```ruo
{
	greeting: "Hello",
	location: "World",
	expressiveness: "!"
};
```

Values, like with an `Array` value, may be of any type, including further `Dictionary` values.

### Operators

Operator|Function|Return
-|-|-
`a[b]`|Indexes `a` at `b`|Value at index `b`
`a.b`|Syntactic sugar for `[]`, where `b` can be expressed as an identifier value|Value at index `b`
`size(a)`/`a.size()`|Element Count|`Number`
`length(a)`/`a.length()`|Equivalent to `size`|`Number`

## Function

Like other high-level languages, `Function` values are first-class data-types, and can be used in the same manner as any other type, passing as arguments or being overwritten during runtime. Overloading is possible by both type of argument and amount of arguments. Of course, any explicit type declaration in a `Function` signature is optional.

To calculate which overload will be used, an integral value is calculated based on the following rules:

Where `A` denotes the list of arguments (passed by a function call) and `P` denotes the list of parameters (as specified in function declaration):

Size of `A` must equal the size of `P` (else overload is not suitable, 0)

For each value in `A`:

If `A[i]` has directly the same type as `P[i]`, +3

If `A[i]` is a type that extends the type of `P[i]`, +2

If `P[i]` is not explicitly declared with a type, +1

Thus, given two overloads:

```ruo
f(a : Number, b : Number) => {}
f(a : Number, b) => {}
```

Calling `f(1, 2)` gives `6` for the first overload, and `4` for the second. The first is chosen.

Calling `f(1, "Hello")` gives `0` for the first (cannot be used), and `4` for the second. The second is chosen.

Calling `f("Hello", "World")` gives `0` for both. Neither is chosen and the interpreter throws an error.

### Literals

`Function` values may be declared explicitly for the scope using an identifier as such:

```ruo
abs(x) => {
	if x < 0 then {
		return -x;
	}
	return x;
}
```

`Function` values may also be declared anonymously without an identifier:

```ruo
(x) => {
	if x < 0 then {
		return -x;
	}
	return x;
};
```

Note that when declared anonymously, the `Function` body declaration ends with a semi-colon.

Anonymous function may also capture variables at the point of their initialization:

```ruo
x := "I'm captured!";

f := ()[x] => {
	putln(x)
};

x = "I'm not captured!";

putln(x);	# I'm not captured!
f();		# I'm captured!
```

Both declaration forms can be simplified for single-line methods as:

```ruo
abs(x) => (x < 0 ? -x : x);
```

Here no explicit `return` statement is required.

From this we have the following syntaxes:

```
<IDENTIFIER>(<ARGS>*) => { <BODY> }
<IDENTIFIER>(<ARGS>*) => <BODY>;
(<ARGS>*) => { <BODY> };
(<ARGS>*) => <BODY>;
```

\* The `Function` signature may also have no arguments, in which the syntax is equivalent - only that within the parantheses be nothing (e.g. `f() => { ... }`).

### Operators

Only the `()` is defined for `Function` values, operating as the call.

## Pointer

The `Pointer` type has no literals or operators - it cannot be declared in Rossa itself. It is used for storing values during interfacing with C++ libraries. Usage is not important unless one wishes to make an external library.

## nil

The type `nil` has no value or operators. It has one literal: `nil`. `nil` may only be equivalent to another `nil` value, and is the default value of any variables declared with `var` (i.e., without a value).