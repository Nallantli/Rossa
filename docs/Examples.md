# Examples

It is easiest to delve into a language not through rote memorization of its API, but through examples. The following are guides and explanations for the most basic scripts.

## "Hello World!"

There is no avoiding the classic K&R schema:

```ra
putln("Hello World!");
```

This one is fairly self-explanatory, and illustrates three major points of the language's syntax.

Foremost, function calls reflect the C style paradigm of `<fname>(<args>)`, where the function name is followed by a list of 0 or more parameters within a parenthetical. Functions must be expressed in this manner even when zero arguments are necessary. A function identifier not followed by a parenthetical returns the value of the function itself, as functions are first-class values rather than addresses to procedures.

In this manner, a hypothetical function `a` with 0 arguments will return the function value itself with

```ra
a;
```

This will not call the function, in fact, it will not do anything notable whatsoever because it does not assign or modify any values.

Secondly, string literal values in Rossa are bordered by double quotes `"<string>"`. While strings are base-values - in a manner akin to JavaScript, Python, and other scripting languages - they cannot similarly be given in optional single quotes `*'<string?>'`. There is no character data-type; when using single quotes, the interpreter reads the numerical equivalent of the character, e.g., `'a'` is equivalent in value and interpretation with the number `97`.

Lastly, lines of instruction are terminated in the C-style with the semi-colon `;`. The compiler will throw an error if no semi-colon is found.

To run the code, save a file the single line to `hello.ra`, and run the interpreter over the file:

```bash
rossa hello.ra
```

The output should appear thus:

```
Hello World!
```

## HiLo

One of my favorite programs to start out with when learning a new language is the classic game of HiLo. The idea is very simple: a random number is generated, and the user inputs their guess until they reach that number. Each time a user inputs a number, the program states whether the number is "too low," or "too high."

To start, we need to initialize a random number generator. To do this, the standard library includes the object `Random`, which is what contains the generator:

```ra
random := new Random();
```

The operator `:=` might be a little unfamiliar to some, it is the declaration-assignment operator. It serves both to declare (set into local memory) the variable and to assign (give the variable a value). All variables need be declared before use.

Next, we will use that generator to give us our _correct_ answer:

```ra
correct := random.nexti(0, 100);
```

Again, we have the operator `:=`, and on the right side we are calling the function `nexti()` within the object we created called `random`. The parameters for `nexti()` are the lower bound and the upper bound integer. Here we ask for a number between 0 and 100 (inclusive).

Now that we have a variable for the correct answer, we need to set a variable for our guess:

```ra
var guess;
```

Using `var` only _declares_ a variable, it does not set it to any value in particular. If you call the variable, it will return `nil`. The `:=` operator should be used when a variable need be declared with a value, while `var` can declare multiple variable names in a single command, e.g. `var a, b, c;`.

Now we must iterate until our `guess` is equivalent to our `correct` number, and to do this we use a `while` loop. The syntax for the loop in Rossa is slightly more explicit than C-style languages:

```ra
while guess != correct do {
```

The syntax asks for the keyword `while`, followed by a statement that evaluates to a Boolean, and then asks for `do`. Following is a new scope declaration with curly-brackets `{}`. Here we've left it open since we are adding code within the scope.

```ra
	puts("Enter guess: ");
	guess = input.line() -> Number;
```

The first line here follows the format of what we saw in the `"Hello World!"` example, the only difference here being that `puts()` does not add a new line after the string, while `putln()` does.

The second line uses `=`, not `:=`, since we want only to assign the variable a value. Using `:=` would declare a new variable `guess` within the internal scope, making all attempts to read the parent scope's `guess` ineffective.

The static object `input` has a member `line()`, which reads a single line from the console. However, the value returns as a string, which cannot be interpreted directly as a number. In a game where it is necessary to have comparison operations, i.e. less than, more than, the value needs to be coerced into a number. Doing this uses the `->` operator, e.g. `"1" -> Number` returns `1`, and `1 -> String` returns `"1"`.

For comparing the value to the correct answer, we may use `if-else` statements:

```ra
	if guess < correct then {
		putln("Too low!");
	} elif guess > correct then {
		putln("Too high!");
	}
```

The syntax of an `if` statement operates similarly to the `while` statement: the token `if` is followed by a Boolean expression, then `then`, and then the scope of the statement. It is worth noting that expressions evaluated within the `if ... then` sequence are evaluated within the inner scope of the statement. This is useful for situations where a variable is to be set and compared in a single line.

Else statements with a following `if` are tokenized with `elif` rather than `else if`.

This completes our while loop, which ends with a simple closing bracket:

```ra
}
```

Now the program will end, but if you wish to offer a message of victory:

```ra
putln("You win! The number was " ++ (correct -> String));
```

Strings may be concatanated with `++`, but doing so requires that `correct` be coerced into a string itself, so again we find use in the `->` operator.

Full code:

```ra
random := new Random();

correct := random.nexti(0, 100);

var guess;

while guess != correct do {
	puts("Enter guess: ");
	guess = input.line() -> Number;
	if guess < correct then {
		putln("Too low!");
	} elif guess > correct then {
		putln("Too high!");
	}
}

putln("You win! The number was " ++ (correct -> String));
```