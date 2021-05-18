# Roblang

[![Build and test](https://github.com/stickyPiston/roblang/actions/workflows/build.yml/badge.svg)](https://github.com/stickyPiston/roblang/actions/workflows/build.yml)

Another programming language, but this one does less than any other.

## Purpose

Roblang aims to be a low-level language that _could_ function as a replacement for C. It includes features that are missing in C, that would make C a more usable language. Roblang takes a lot of inspiration from [TypeScript](https://www.typescriptlang.org) too.

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

Number literal are present in three forms: hexadecimal notation, decimal notation and binary notation. The number literal type is recognised by the prefix `0x`, `0d` and `0b` respectively. When no prefix is given, the number literal type decimal is assumed. Numbers with a decimal point are considered as floats (or doubles). Prefixes are not allowed when defining floats.

#### 1.4 Identifiers

Identifiers must start with an underscore (`_`) or a letter (both lowercase and uppercase). In the rest of the identifier, numbers, letters and underscores are allowed in the identifier. There is no limit as to how long the identifier may be.

#### 1.5 Functions

Functions follow a fixed pattern: first the parameters surrounded by round parentheses (`(` and `)`) and split by a comma (`,`), then an arrow `->` must be added and after the arrow the function body must follow. The function body consists of the first following expression.

```roblang
# An example of a simple function #
buildGreet = (age) -> { return(if(age >= 18, "Hello", "Wassup")); };

hello = (name, age) -> {
  printf("%s %s", buildGreet(age), name);
};
```

The machine code generated for the functions adheres to the function call conventions of C. Roblang functions are able to be used without special implementation in C (using the `extern` keyword in C).

A function (both annoymous or assigned) creates a new scope. Scopes work the same as in C. Variables defined in a scope cannot be accessed outside of that scope, but variables defined in a higher scope can be used inside the scope.

#### 1.7 Function calls

Function calls are recognised by parentheses after an identifier or function. If parantheses are put after an operator, then it is regarded as an expression. The arguments for the function are placed within the parentheses, and should be split by a command (`,`) if there are more than one. The function arguments are evaluated as expressions, so expressions are allowed to be passed as function arguments. The generated machine code for a function call adheres to the calling conventions of C. A C function is able to be called from within roblang without any special implementation in the roblang file.

```roblang
hello = (10 + 20) * 5; # not a function call #
world = rand(); # this is a function call #
complexFunction((10 + 20) / 90, rand(), hello);
```

#### 1.8 Types

All variables need to be typed in roblang, however whenever the type can be inferred the type defintion can be omitted. So as to associate a type with a variable, put a `:` followed by the type name after the variable name.

```roblang
# Type aliasing, just assing a type to another identifier with = #
String = u8*;

# main does not need a type definition, because it can be inferred from the function prototype.
  The paramter do need a type definition, because the type cannot be inferred.
  The return type of a function is put after the closing ) of the parameter block #
main = (argc: i32, argv: String*): u8 -> {

  # A variable that is of type u16, the type follows the variable name #
  hello: u16 = 1024;
  return(0);
};
```

#### 1.9 Generics

Roblang has built-in support for generics. The syntax for generics are mostly inspired by TypeScript. In order to make a generic type, add a type parameter after the type's name in angled brackets (`<` and `>`).

```roblang
Generic<T> = T
Map<T, U> = { ... };
```

If you want to restrict what types are allowed as parameter you can assign the type parameter to a built-in generic type, which restricts types. Additionally, roblang overloads some operators so that they can be used to restrict types too.

```roblang
Map<T = HasInPrototype<"==">, U> = { ... };

# Number can be an u8 or an i8 or an u16, etc. #
Number<T = u8 | i8 | u16 | i16 | ...> = T;
```

Functions can also take type parameters, and they can also restrict types with the aforementioned principles.

```roblang
doCalculations<T = i8 | i16 | i32 | i64> = (a: T, b: T): T -> {
  # ...
};

# If no type is given, i32 is assumed #
doCalculations(10, 20);
doCalculations<i8>(10, 20);
```

#### 1.10 Structs

Structs in roblang are considered a type variable, that is, they can only be used as a type or as rvalue. For the rest, they work in a similar fashion as C, it is a typed container that combines some (relevant) data. The properties of a struct can be accessed with a period (`.`)

```roblang
Person = { name: u8*, age: u8 };
john: Person = { "John", 37 };

john.age # yields 37 #

a: { brand: u8*, model: u8* } = { "Tesla", "Model 3" };

a.brand # yields "Tesla" #
```

Structs can also be generic, the standard rules for generics are applied to structs too.

```roblang
Vector<T> = { size: u32, items: T[] };
names: Vector<u8*> = { 0, NULL };

Map<T = HasInPrototype<"==">, U> = { size: u32, items: { key: T, value: U }[] };
```

#### 1.11 Struct member functions

Functions can be associated with structs and act as member functions. The type name can even be used as a function, which is useful for creating constructors for structs.

```roblang
String = u8*;

# Person type #
Person = { name: String, age: u8 };
Person.greet = (p: Person) -> {
  printf("Hello, %s!\n", p.name);
};
# Person function (the constructor in this case) #
Person = (name: String, age: u8): Person -> {
  return({ name, age });
}

# The type can be inferred from the Person function's return type #
john = Person("John", 37);

# You don't need to pass an argument to this function, because the first argument to the function is the struct you're calling this function on, which is implicitly passed already. #
john.greet(); # yields "Hello, John!" #
```

The generics rules apply to struct member functions too.

```roblang
Vector<T> = { size: i32, items: T* };
Vector<T>.push = (v: Vector<T>, i: T) -> {
  # Logic for pushing an element here...
};

# A function that does not require a type parameter #
Vector.length = (v: Vector) -> {
  return(v.size); # size exists regardless of the type vector we are handling #
};

vec: Vector<i32> = { 0, NULL };
vec.push(10);
vec.length(); # yields 1 #
```

#### 1.12 Enums

Enums have a similar syntax to structs, but the field names are omitted. An enum creates a type and a variable, which means that enums cannot be function overloaded like structs can.

```roblang
NodeType = { Function, Variable, Number };

processNode = (t: NodeType) -> {
  # ...
};
processNode(NodeType.Function);
```

#### 1.13 Operator overloading

Roblang also allows for operator overloading. The operation is put on the left-hand side of the assignment operator and the function that needs to be run is put on the right-hand side of the assignment operator.

```roblang
Vector<T> = { size: i32, items: T* };

# The types for a and b can be inferred #
Vector<T> + Vector<T> = (a, b): Vector<T> -> {
  # ...
};

Vector<T> - i32 = (a, b) -> {
  # ...
};

vectorOne = Vector(10, 20, 30);
vectorTwo = Vector(40, 50, 60);
vectorThree = Vector("Hello", "World");

tmp = vectorOne + vectorTwo; # yields a Vector<i32> #
tmp = vectorOne + vectorThree; # Error, no overload for Vector<i32> + Vector<i8*> #
tmp = vectorOne - 1; # Also fine, the overload is defined #
```

#### 1.14 Pointers

Pointers are also included in roblang. They have the same semantics as in C, that is, pointer point to a block of memory. If you want to access the contents _at_ that block of memory you need to dereference the pointer (with the `*` operator). If you want to pass around a value as a pointer, you can make a pointer with the `&` operator.

Pointer types are noted with a `*` after a type. You can also use `[]` after a type, which is just syntactic sugar for `*` (but a little more self-documenting).

```roblang
takesReference = (a: i32*) -> {
  # get the value of a #
  printf("Address of a: %p\nValue of a: %d", a, *a);
};

hello: i32 = 10;
helloPointer: i32* = &hello;
takesReference(helloPointer);
takesReference(&hello);
```

#### 1.15 Operators

Roblang provides a basic set of operators:

##### Arity 1

- `~`: Binary not (`not` assembly instruction)
- `!`: Logical not, should adhere to the following truth table: (for explanation about true and false, see section Arity 2 > Logic)
   | Input | Output |
   |-------|--------|
   | true  | false  |
   | false | true   |
- `*`: Value at operator. When this is used in combination with the `=` operator, the value on the right-hand side of the assignment operator is stored at the address that is stored in the variable of right-hand side of the `*` operator (see §1.14).

   ```roblang
   hello: i32* = 10;  # 10 is the address in this situation #
   *hello = 20; # 20 is stored at the address 10 #
   ```

   When the operator is not used in combination with the `=` operator, it yields the value at the address.

   ```roblang
   world: i32* = malloc(4);   # Malloc returns the address of the allocated block #
   *world = 20;         # Therefore you can use the * and = combination to store 20 in the allocated space #
   hello = 10 + *world; # hello would be 10 + 20 #
   ```

- `&`: Reference operator. This operator takes yields the address of the variable, thus it can be used to create a pointer to a variable (see §1.14).
- `++`: Prefix/suffix increment operator. This operator increments the variable by one. The prefix version increments, then yields the value. The suffix version yields and then increments the value.
- `--`: Prefix/suffix decrement operator. This operator decrements the variable by one. The prefix version decrements, then yields the value. The suffix version yields and then decrements the value.
- `[]`: Indexing operator. This operator retrieves the value at the given index from a variable. The variable has to be an array or pointer or have an overload for this variable.

  ```roblang
  array: i32* = malloc(4 * sizeof(i32));
  array[0] = 10;
  array[1] = 20;
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

#### 2.1 Expressions

In roblang, everything is considered an expression. This is done for consistency and for the easy implementation in the parser. Function declarations are simply anonymous function assigned to a "variable". A function assigned to a variable name has different behaviour from a value assigned to a variable name, however roblang aims to minimise the differences. Every expression is translated into machine code, even if it is not used.

```roblang
hello = (name) -> printf("Hello, %s", name);
world = 5;
hello + world; # This does not compile, a function is treated differently (it simply does not make sense to do this) #
hello("John") + world; # This works like in any other language #
```

#### 2.2 Control structures

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

#### 2.3 Types

Types are solely a static construct, which means that types are not like variables (even though the syntax is mostly similar). The compiler will warn whenever you try to assign a type to a value.
