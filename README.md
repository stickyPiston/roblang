# Roblang

[![Build and test](https://github.com/stickyPiston/roblang/actions/workflows/build.yml/badge.svg)](https://github.com/stickyPiston/roblang/actions/workflows/build.yml)

Another programming language, but this one does less than any other.

## Purpose
Roblang aims to be a low-level programming language, with a level between C and Assembly. Roblang removes the tedia from Assembly while not introducing complex and heavy-weight features like C. Roblang does not include static features like type checking, higher-level data structures, etc. Roblang is designed to be easily implementable and easy to learn due to low number of expression types.

## Implementation

### 1. Syntax

#### 1.1 Whitespace

Whitespace in roblang is ignored in every case (except for strings, see 1.3 Literals).
```roblang
hello         =
10 +        20             /
10;
```

#### 1.2 Comments

Comments are indicated by an hashtag (`#`) and continue until the next hashtag. Whitespace do not interfere with the flow of comments. Comments can also be used inline without interfering with the flow of expressions.
```roblang
# Hello function #
hello = () -> {
  puts(# Comments can be put here # "Hello");
  10 + # Also here # 10;
};
```

#### 1.3 Literals

Roblang knows two literal types: numbers and strings. Strings are special literals, in that they are converted into an array of consecutive number literals according to the [ascii-table](http://asciitable.com) ending with a null-terminating byte (`\0`). Strings are encompassed by double quotes (`"`), everything until the next double quote is part of the string. Comments and roblang code in string literals are parsed as part of the string. Whitespace does not influence the flow of string literal, but does count as characters in the string.
```roblang
puts("Hello,
This is my TED talk.
I would like to talk about roblang.
It is an amazing and revolutionary programming language everyone should use.");
```

Number literal are present in three forms: hexadecimal notation, decimal notation and binary notation. The number literal type is recognised by the prefix `0x`, `0d` and `0b` respectively. When no prefix is given, the number literal type decimal is assumed. Number with a decimal point are disallowed (roblang has no notion of floats).

#### 1.4 Identifiers

Identifiers must start with an underscore (`_`) or a letter (both lowercase and uppercase). In the rest of the identifier, numbers, letters and underscores are allowed in the identifier. There is no limit as to how long the identifier may be.

#### 1.5 Functions

Functions follow a fixed pattern: first the parameters surrounded by round parentheses (`(` and `)`) and split by a comma (`,`), then an arrow `->` must be added and after the arrow the function body must follow. The function body consists of the first following expression. Functions implicitly return the value of the function body.
```roblang
# The first expression after the arrow is the function body, { } are considered one expression (see 1.6 Blocks)
  The return value of if is the return value of buildGreet #
buildGreet = (age) -> if(age >= 18, "Hello", "Wassup");

hello = (name, age) -> {
  printf("%s %s", buildGreet(age), name);
};
```

The machine code generated for the functions adheres to the function call conventions of C. Roblang functions are able to be used without special implementation in C (using the `extern` keyword in C).

#### 1.6 Blocks

Blocks are considered as one expression, but when the machine code for a block is generated, the machine code for expressions in the block are separately generated. The last expression in a block is the "return value" of that block.
```roblang
# hello is 10, because world / 3 is 10 #
hello = {
  world = 10 + 20;
  world / 3;
};
```

TODO: Eventually figure out scope rules

#### 1.7 Function calls

Function calls are recognised by parentheses after a node. If parantheses are put after an operator, then it is regarded as an expression. Te arguments for the function are placed within the parentheses, and should be split by a command (`,`). The function arguments are evaluated as expressions, so expressions are allowed to be passed as function arguments. The generated machine code for a function call adheres to the calling conventions of C. A C function is able to be called from within roblang without any special implementation in the roblang file. The types of a function do not influence how the function is called, and thus, the function does not need to be known when it is called.
```roblang
hello = (10 + 20) * 5; # not a function call #
world = rand(); # this is a function call #
complexFunction((10 + 20) / 90, rand(), hello);
```

#### 1.8 Operators

Roblang provides a basic set of operators:

##### Arity 1

 - `~`: Binary not (`not` assembly instruction)
 - `!`: Logical not, should adhere to the following truth table: (for explanation about true and false, see section Arity 2 > Logic)
   | Input | Output |
   |-------|--------|
   | true  | false  |
   | false | true   |
 - `*`: Value at operator. When this is used in combination with the `=` operator, the value on the right-hand side of the assignment operator is stored at the address that is stored in the variable of right-hand side of the `*` operator.
   ```roblang
   hello = 10;  # 10 is the address in this situation #
   *hello = 20; # 20 is stored at the address 10 #
   ```

   When the operator is not used in combination with the `=` operator, it yields the value at the address.
   ```roblang
   world = malloc(4);   # Malloc returns the address of the allocated block #
   *world = 20;         # Therefore you can use the * and = combination to store 20 in the allocated space #
   hello = 10 + *world; # hello would be 10 + 20 #
   ```

##### Arity 2

**Assignment:**

 - `=`: Assignment operator, assigns the value at the right-hand side of the operator to the left-hand side.

**Arithmetic:**

 - `+`: Arithmetic addition (`add` assembly instruction)
 - `-`: Arithmetic subtraction (`sub` assembly instruction)
 - `*`: Arithmetic multiplication (`mul` or `imul` assembly instruction)
 - `/`: Arithmetic multiplication (`div` assembly instruction)

**Binary:**

 - `<<`: Binary shift left (`shl` assembly instruction). The right-hand side is the number of shifts to be done to the left-hand side.
 - `>>`: Binary shift right (`shl` assembly instruction). The right-hand side is the number of shifts to be done to the left-hand side.
 - `&`: Binary and (`and` assembly instruction)
 - `|`: Binary or (`or` assembly instruction)
 - `^`: Binary xor (`xor` assembly instruction)

**Logic:**

True in this section refers to any thruthy value. Thruthy values are any values but 0. False refers to any not thruthy values, 0 is the only falsey value. If true is yielded from an operator, it is transformed into an 1, and if false is returned from an operator, it is transformed into a 0.

 - `&&`: Logical and, should adhere to the following truth table:
   | Input 1 | Input 2 | Output |
   |---------|---------|--------|
   |  true   |  true   |  true  |
   |  true   |  false  | false  |
   |  false  |  true   | false  |
   |  false  |  false  | false  |
 - `||`: Logical or, should adhere to the following truth table:
   | Input 1 | Input 2 | Output |
   |---------|---------|--------|
   |  true   |  true   |  true  |
   |  true   |  false  |  true  |
   |  false  |  true   |  true  |
   |  false  |  false  | false  |
 - `^^`: Logical xor, should adhere to the following truth table:
   | Input 1 | Input 2 | Output |
   |---------|---------|--------|
   |  true   |  true   | false  |
   |  true   |  false  |  true  |
   |  false  |  true   |  true  |
   |  false  |  false  | false  |
 - `<`: Smaller than, should yield true when the left-hand side is smaller than the right-hand side, otherwise false is yielded.
 - `<=`: Smaller than or equal to, should yield true when the left-hand side is smaller than or equal to the right-hand side, otherwise false is yielded.
 - `>`: Greater than, should yield true when the left-hand side is greater than the right-hand side, otherwise false is yielded.
 - `>=`: Greater than or equal to, should yield true when the left-hand side is greater than or equal to the right-hand side, otherwise false is yielded.
 - `==`: Equals, should yield true when the two operands are equal, otherwise false is yielded.
 - `!=`: Does not equal, yields true when the two operands are not equal, otherwise false is yielded.

### 2 Grammar

#### Expressions

In roblang, everything is considered an expression. This is done for consistency and for the easy implementation in the parser. Function declarations are simply anonymous function assigned to a "variable". A function assigned to a variable name has different behaviour from a value assigned to a variable name, however roblang aims to minimise the differences. Every expression is translated into machine code, even if it is not used.
```roblang
hello = (name) -> printf("Hello, %s", name);
world = 5;
hello + world; # This does not compile, a function is treated differently (it simply does not make sense to do this) #
hello("John") + world; # This works like in any other language #
```

#### Control structures

Roblang has two built-in control structures. To comply with the "everything is an expression" notion, the control structures are implemented as global functions. The signature for `if` is `if( cond, when true, when false )`. When the condition evaluates to true, the `when true` argument is evaluated, this can be a value or a function. This value is then returned.
```roblang
# hello contains 10, because 1 == 1 is true, so 10 returned from the if function #
hello = if(1 == 1, 10, 20); 

# world contains 20 + hello, because 2 != 2 is false, so the return value of () -> { 20 + hello; } is returned from the if function #
world = if(2 != 2, 0, () -> { 20 + hello; });

# You can also pass a function assigned to a varibale to if
  You can also omit the else argument if you do not plan to use it #
func = () -> { puts("Hello"); };
if(1 == 1, func);
```

The second control structure is `while`, the signature for that function is `while( cond, loop )`. The value of loop is evaluated as long as `cond` evaluates to true. `loop` and `cond` should always be a function, otherwise an infinite loop will occur.
```roblang
i = 0;
while(() -> { i < 10; }, () -> {
  printf("%d\n", i);
  i += 1;
});
```
