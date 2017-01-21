#Wreck Engine Style Guide

##Formatting

###Do:
  - Use 4 spaces for tabs (this convention may be broken in older code, fix it!)
  - Visually align assignments where it makes sense
  - Use line breaks in long expressions, like conditionals or parameters:
    - Only do this if the expression is in danger of exceeding reasonable horizontal screen space
```cpp
if(a_really_long_function() 
&& anotherReallyLongFunction()
&& yetAnotherLongFunction()) {
	betterUseBraces();
}

// notice the alignment of the start of the first params on each line
foo(abacus, weathering, beets
  , determinator, antidisestablishmentarianism);
```
  - Try to keep separators (e.g. logical operators, commas) at the beginning of the NEW line, 
  not the end of the old one
  - Try to keep braces on the same line as their statement
    - This is a loose requirement
  - At least one space before an inline comment, one space after the `//` before the content:
    - `a = b + c; // yup, it's a comment`
	
###Don't:
  - Use linebreaks unnecessarily:
```cpp
// 2 short initializers do not need line breaks!
// I know cppreference does this and I hate it
Thing() :
wow(false),
im("edgy") 
{}
	
// better, also using line breaks appropriately
Thing(int) : wow(true), im("a good kid")
, extraLongThing("mega salami") {}
	
// this is debatable for members, but for temporaries NO
void foo() {
	int a;
	int b;
	int c;
	//...
}
```
  - Overalign:
```cpp
int                thingama; // this is clearly too many spaces
TheLongestTypename hoober;
	
int   bob; // still a bit silly, but acceptable
float ralph;
```

##Naming

###Do:
  - Use underscores between words in utility code, and camelcase for everything else:
    - `FILE load_file(const char* name);` vs. `bool collidesWith(Collider* other);`
	- This extends to local and global variables
  - Follow library conventions when naming types
    - e.g. OpenGL uses GLlowercase for types
  - Prefer namespaces or nested types to type/library prefixes
    - e.g. `Transform::mat_cache` GOOD, `TransformMats` BAD, 
      `EventEndpointData` BAD, `Event::EndpointData` GOOD
  - Capitalize class names, unless they're utilities that aren't library related
  - Make macros explicit with all-caps
  - Prefer descriptive names to comments
  - Generally use verbs for methods
	- Prefer `menu.display()` over `displayMenu()`
  - Use `get` and `set` in accessor names if they aren't single responsibility
    - This also applies to pseudo-accessors that get/set things that aren't fields
	- The exception is utility code; just use regular accessor names for that
```cpp
int _apples;
int _oranges;

int apples() { return _apples; }
// vs
int getOranges() { _apples--; return oranges; }
```
	
###Don't:
  - Use numbers, e.g. `foo2()` 
    - Unless there's a valid reason, e.g. `MD5`
  - Use unnecessary variable prefixes; only use underscores when needed
    - The main case would be a member variable with an accessor function: 
      the member variable gets an underscore, the accessor gets the unprefixed version
    - e.g. if there's no accessor, `numBalloons`; if there is, `_numBalloons`
    - Examples of unnecessary prefixes: `m_member`, `s_static`, `g_global`
    - The exception is unavoidable name conflicts; 
      function parameters like `t_param` are acceptable in these cases.
  - Capitalize function/variable names, C# style
  - Name variables **entirely** after their type, e.g. `Program program;`
  
  
##General Rules

###Do:
  - Use `auto` wherever possible
  - Use `const` and `constexpr` wherever possible
    - Try to make class methods `const` wherever possible, 
      and try to figure out which members are logically `mutable`
    - High priority if the class is a value type and used in `const` context a lot
  - Write `for(... : ...) // foreach` when it's cleaner to than a regular `for` loop
  - Prefer `size_t` for step counters when it makes sense
  - When using a bound that requires a function call or dereference in a `for` loop, 
	cache its value in a place that makes sense (generally the loop initialization):
    - `for(size_t i = 0; i < vec.size(); ++i); // bad`
    - `for(size_t i = 0, size = vec.size(); i < size; ++i); // good`
  - Prefer the `ACCS_` macros found in **property.h** over writing accessors manually
    - Unless there is significant complexity where inlining them is undesirable
  - Use single line comments for up to a few line comments on functions or classes, 
	and formatted block comments for more than that.
    - XML is also acceptable, and probably should be preferred
  - Make liberal use of comments if the functionality is something you're likely to forget 
	or others are not likely to understand from names alone
    - This also applies if the sum of the whole is not immediately obvious from the name
  - Prefer composition with RAII over explicit initialization/destruction (rule of zero)
	
###Don't
  - Use dedicated type names for variables of in-engine types if you can use `auto`
    - This makes refactoring that much harder
  - Use nested ternaries
  - Use maps without thinking about alternatives; they take up a lot of space
    - If you need map functionality, consider whether 
	`map` (slower access, less space) or `unordered_map` (faster access, more space) 
	is the appropriate choice.
  - Manually manage memory; just use smart pointers (smart_ptr.h)