# Contribution Guidelines {#Contributing}

[TOC]

Any contributions to MCRO are welcome and appreciated. If you want to contribute back to its source code we will do our best that your work will consistently fit in to the code base. In order to make this process easier please consider the following guide for contribution.

## Git flow

MCRO currently is only big enough for one `main` branch, which is locked from direct commits. Before contributing your change make a `feature` or a `bugfix` branch about what you'd like to add / fix. For example `feature/shiny-new-stuff` or `bugfix/annoying-stuff`. Please keep your commits focused on the subject of what the branches name. If you find dependency fixes or add dependency features which your subject relies on, it can still be part of the same branch and the same PR, but please explicitly list that in the PR description and title. PR's should always target `main` branch.

When you create a PR please describe your train-of-thought behind it and describe the root cause why you made it. The length of this description is usually proportional to the amount of changes or additions introduced (unless they're repetitive). In most cases it will be required to write unit tests for your changes. If you feel like that's too much work, submit your PR anyway and we may discuss how we could help with that.

MCRO is still a small project so there are not much rules for contribution, but please consider common-sense and cooperation with others. Also most importantly be kind, understanding, professional and assertive when phrasing the text of any PR, its contents, or any discussion point around MCRO and its broader vicinity. Offensive tones, discriminatory language, any form of bigotry, slender, racism, sexism, xenophobia, anti-LGBTQ, anti-semitism will not be tolerated, even in the unlikely event that the "contribution" behind such foul language is "valuable".

## Code Style C++

MCRO is slightly deviating in code-style from the usual Unreal Coding Style-guides. This is chosen for personal preference and couple of pragmatic things. In my opinion the Unreal Guidelines are too strictly followed in the industry, to the degree that it is actively working against code readability, and promotes verbosity and bloat. For example the total ban on `auto` keyword or the ban on implicit conversion constructors.

This is matter of personal opinions and tastes and I'm sure many of the readers will have perfectly valid reasoning against all the style changes MCRO brings with itself. I'm trying to justify my changes and reason why they're practical.

The following sections are either changes from the Unreal coding style or specifications where it is not specified.

### Whitespaces

Indentation is done with tab characters (as in Unreal).

### Placement of header files

Header files should be placed in explicit subfolders of their topics/modules. This may result in deeper folder structure but it allows to have simpler header file names without the risk of clashing and a better understanding of where they come from in the source file. For example don't place them directly in public module folders:

```
Don't:

MyModule
└───Public
    └───MyFeature.h

#include "MyFeature.h"
// where does this come from? what happens if other module has `MyFeature`?
```

Do place them in a subfolder:

```
Do:

MyModule
└───Public
    └───MyModule
        └───MyFeature.h

#include "MyModule/MyFeature.h"
```

This seems redundant but provides immediate feedback about the origin of a header file when included.

### Usage of namespaces

Epic games unfortunately doesn't believe in namespaces as language features ([or do they now? idk anymore](https://dev.epicgames.com/documentation/en-us/unreal-engine/blueprint-namespaces-in-unreal-engine)), and so they force unreal developers to use [smurf-naming](https://blog.codinghorror.com/new-programming-jargon/#21-smurf-naming-convention) almost everywhere.

I on the other hand believe in namespaces and that they've been invented for a good reason. That's why almost everything in MCRO should be under topical namespaces where that is allowed. Obviously for UClasses and other things processed by UHT namespaces are not usable.

This means MCRO features should be imported this way:

```Cpp
#include "Mcro/Error.h"

namespace Mcro::MyFeature
{
    using namespace Mcro::Error;
}
```

This can be even done in a header file as the namespace usage has a clear scope boundary.

When using the MCRO library, a cumulated header file is used `Mcro/CommonCore.h` and `Mcro/Common.h` the latter containing more elaborate extra features. Both declare the `Mcro::Common` namespace which collects all MCRO features into a single using directive. When adding new headers and namespaces to MCRO, you should also add those to `Mcro/CommonCore.h` or `Mcro/Common.h` depending on how heavy said feature is.

Using MCRO features in headers which are processed by UHT, so in UClasses, UStructs and so on, is a bit trickier. You may use fully qualified names like `Mcro::Common::IError` or shorten the namespace with an alias `namespace mcro = Mcro::Common` both are meh but "it is what it is".

Once Unreal will support C++ modules for all platforms and all major compilers (not only on MSVC) then namespaces from MCRO may be omitted. That however will be a breaking major version change.

### Naming convention

In addition to the pascal-case Hungarian notation for types, templates and enums the following rules are used in MCRO:

#### Templates

Template declaration should have a space between `template` and `<`.

```Cpp
// Do
template <typename T>

// Don't
template<typename T>
```

If the template parameter lists or template arguments get long, they should be organized into multiple lines with ["egyptian"](https://blog.codinghorror.com/new-programming-jargon/#3-egyptian-brackets) arrow-brackets, with the closing bracket un-indented.

```Cpp
template <
    typename Foo,
    typename Bar,
    typename Stuff,
>
```

##### Concepts

C++ 20 concepts are a new metaprogramming feature, but Unreal actually had a [naming convention for them](https://github.com/EpicGames/UnrealEngine/blob/803688920e030c9a86c3659ac986030fba963833/Engine/Source/Runtime/Core/Public/Templates/Models.h#L15) even before that became part of C++. So MCRO takes this onwards and uses the `C` prefix for concepts:

```Cpp
// Do
template <typename T>
concept CMyConcept = /*...*/ ;

// Don't
template <typename T>
concept TMyConcept = /*...*/ ;
```

##### Template Parameters

Template parameters should follow PascalCase casing and shouldn't have a prefix

```Cpp
// Do
template <typename SomeElaborateStuff, typename FirstThing>

// Don't
template <typename TSomeElaborateStuff, typename TFirstThing>
```

##### Templated functional structs

In templating empty structs are often used as functions to transform one type to another, or to test something on a type using specialization. However the result of this is either an inner type alias or an inner constexpr value. For example in STL type traits `remove_pointer` is a struct and you would use its result through a type alias `typename remove_pointer<T>::type`. Except you would not do this and use the alias `remove_pointer_t<T>`. However this eventually makes `remove_pointer` an implementation detail, as it's not meant to be used outside of STL headers (unless for type traits where specialization makes sense). `remove_pointer_t<T>` is used much more frequently than `remove_pointer` in that regard.

For that reason similar functional templating structs are postfixed with `_Struct` in MCRO, and its alias is used without any appendages:

```Cpp
template <size_t, typename>
struct TTypeAt_Struct
{
    using Type = void;
};

template <size_t I, CStdOrRangeV3Tuple T>
struct TTypeAt_Struct<I, T>
{
    using Type = std::tuple_element_t<I, T>;
};

template <size_t I, CTuple T>
using TTypeAt = typename TTypeAt_Struct<I, T>::Type;
```

#### Variable/Function parameter names

All names in Unreal should start with a capital letter, however that can reduce visual clarity about what is a local variable and what is something outside of a function body scope, like a class member or a global variable. For this reason MCRO advocates for camelCasing local variables or function parameters:

```Cpp
struct FMyClass
{
    int32 Foo = 0;
    int32 Bar = 0;
    bool bStuff = false;
}

void FMyClass::MyFunction(int32 foo, int32 bar, bool stuff)
{
    FString asd(TEXT("yo"));
    int32 wazz = 5;
    Foo = foo;
    Bar = bar;
    bStuff = stuff;
    // ...
}
```

Notice how the `b` prefix also disappeared from the local bool, and notice how there's no need to have `In` appendages for the parameters. This is even more useful with constructors:

```Cpp
FMyClass::FMyClass(int32 foo, int32 bar, bool stuff)
    : Foo(foo), Bar(bar), bStuff(stuff)
{}
```

#### Multiline function parameters/arguments

If the function parameter lists or function invocation arguments get long, they should be organized into multiple lines with ["egyptian"](https://blog.codinghorror.com/new-programming-jargon/#3-egyptian-brackets) parenthesis, with the closing bracket un-indented.

```Cpp
void MyFunc(
    int32 foo,
    int32 bar,
    int32 stuff,
) {
    // ...
}
```

If function definition follows directly the opening curly-bracket can be on the same line as the parameter list closing bracket.

Related to this, if a function exceeds three parameters, declare a struct instead for passing arguments. A POCO struct is also really useful for argument passing because C++20's designated initializer syntax, that can serve as named arguments. That notion also makes passing data to lower-level API also much easier.

### Optional curly brackets

In MCRO curly brackets around single expression scoped statements (like `if`, `for`, `while`, etc...) are not mandatory. This is not enforced though. The original intention of the Unreal Coding convention with mandatory brackets were to not break macros like `UE_LOG` which may have multiple expressions.

### Implicitly converting constructors

It is super annoying to type the names of compatible variables when they're convertible between each-other so MCRO doesn't enforce `explicit` constructors on its features. The original intention of enforcing `explicit` converting constructors is visualizing a conversion step from one type to another type which may be expensive. However in many cases it makes the code unnecessarily bloated.

It also makes templating less intuitive to write in some particular cases.

### Reference and const qualifier order

MCRO uses

* `FMyType&` for mutable reference (no space between)
* `FMyType const&` for const reference
  * This is unlike Unreal when the same is expressed with `const FMyType&`
  * The reasoning behind placing the `const` after the type because we're qualifying the mode of the reference, not the type itself. This is especially important distinction with const pointers, as the type `const FMyType*` is not const actually. The only way to have a const reference to a const pointer is with the postfixed const-ref `const FMyType* const&`.
* `FMyType const*` for const pointer to keep it consistent with `const&`
  * in fact this order is easier to combine explicitly with a const-ref to a const pointer `int const* const&`.
* 

Note that preceeding `const` should be only used in simple scenarios like an array of const items `TArray<const FMyType>`.

### Lot less restrictive usage of auto

`auto` keyword in Unreal is discouraged, and in general C++ developers are not so unanimous about its benefits. Indeed it has its foot-guns for example how it doesn't preserve reference qualifiers on itself. However some templating libraries and even MCRO itself can get really elaborate and long on templated type names, in which case `auto` is not only a blessing, it is necessary. For this reason `auto` is encouraged in MCRO.

If you take `auto` as short for `template <typename T> T ...` then you may also use its reference qualifiers in the same way. For example `auto const&` will avoid copying. `auto&&` or equivalent `decltype(auto)` is the same as "universal" or "forwarding" reference on template arguments (meaning that qualifiers are preserved).

In templated, constexpr or inline functions `auto` can be used as return type as well, especially when a lambda functor or some elaborate templated type is returned.

### Trailing return type

MCRO encourages to use trailing return types when that would be longer than 2 or 3 words, or longer than the function name. Trailing return types also can use scoped aliases as well in function definitions whereas preceeding return types cannot.

## Documentation

MCRO uses Doxygen to generate documentation. All public facing API needs to have a JavaDoc style comment associated with it (in 99% of cases on their top).

If the functionality is trivial a one liner is enough:

```Cpp
/** @brief This is some simple stuff */
```

Note that `@brief` is mandatory, as I didn't like personally the automatic understanding of what brief should be in Doxygen (the first sentence of the first paragraph without an operator). In MCRO 'brief' can be multiple sentences or lines even, but maximum 2-3 lines and 3 sentences. Paragraphs without operators are part of detailed descriptions.

The following is an example elaborate documentation comment. Note that each line has a space aligned `*` character and a TAB afterwards. The tab character is chosen so the tab indentation of code examples are not broken.

```Cpp
/**
 *	@brief
 *	In ullamco dolore anim tempor velit proident quis cupidatat aute non esse ea velit. Minim proident qui consectetur
 *	mollit eu proident veniam id. Minim voluptate pariatur elit ipsum.
 *	
 *	Consectetur ut do quis qui irure eiusmod mollit adipisicing occaecat ipsum amet aliqua sit non magna. Enim minim et
 *	exercitation cupidatat ullamco consequat ad magna occaecat. Laboris laboris fugiat aliqua laborum Lorem duis. Mollit
 *	ex est mollit. Nisi sunt minim velit non deserunt. Consectetur minim nulla enim deserunt qui elit ex ut labore.
 *	
 *	Excepteur qui magna minim. Magna laboris sit eu eu Lorem elit magna. Eu culpa eu veniam proident ipsum. Laborum id
 *	sit enim qui est minim enim. Incididunt fugiat consectetur dolore laboris dolore laborum laborum labore Lorem
 *	consequat magna. Consectetur quis veniam consequat minim officia Lorem voluptate qui ea sunt nisi esse amet qui.
 *	Laborum proident occaecat esse irure incididunt et velit ea incididunt quis quis quis ad adipisicing. Commodo elit
 *	proident reprehenderit ea labore Lorem irure fugiat reprehenderit cupidatat ullamco.
 *	@code
 *	class FMyComponent
 *		: public TInherit<IFoo, IBar, IEtc>
 *		, public IComponent
 *	{
 *		// ...
 *	}
 *	@endcode
 *	
 *	@tparam      Self  Deducing this
 *	@param       self  Deduced this (not present in calling arguments)
 *	@param  condition  Only add message when this condition is satisfied
 *	
 *	@param veryComplicated
 *	In ullamco dolore anim tempor velit proident quis cupidatat aute non esse ea velit. Minim proident qui consectetur
 *	mollit eu proident veniam id. Minim voluptate pariatur elit ipsum.
 *	
 *	@param   input  the message format
 *	@param fmtArgs  format arguments
 *	@return  Self for further fluent API setup
 */
```

Single line operators should be separated by 2 spaces from their text. Parameters can be done in blocks of single line documentations or if they have more than one line worth of information, as entire paragraphs annotated with the `@(t)param` operator. When blocks of single line documentation is used their parameter names should be right-aligned to the 2 spaces separating the documentation text. Many text-editors and IDE's have features to help with that (I use Rider or VS Code, both have such features (or extensions)).

Multiple instances of the same operators will be combined into the same section on the generated webpage for the item, even when they're separated by other operators, or are in different blocks of comment.

```Cpp
/**
 *	@remarks
 *	Consectetur ut do quis qui irure eiusmod mollit adipisicing occaecat ipsum amet aliqua sit non magna. Enim minim et
 *	exercitation cupidatat ullamco consequat ad magna occaecat. Laboris laboris fugiat aliqua laborum Lorem duis. Mollit
 *	ex est mollit. Nisi sunt minim velit non deserunt. Consectetur minim nulla enim deserunt qui elit ex ut labore.
 *	@remarks
 *	Excepteur qui magna minim. Magna laboris sit eu eu Lorem elit magna. Eu culpa eu veniam proident ipsum. Laborum id
 *	sit enim qui est minim enim. Incididunt fugiat consectetur dolore laboris dolore laborum laborum labore Lorem
 *	consequat magna. Consectetur quis veniam consequat minim officia Lorem voluptate qui ea sunt nisi esse amet qui.
 *	Laborum proident occaecat esse irure incididunt et velit ea incididunt quis quis quis ad adipisicing. Commodo elit
 *	proident reprehenderit ea labore Lorem irure fugiat reprehenderit cupidatat ullamco.
 */
```

This is especially useful with the `@file` operator.

### Documenting headers

When the usage of the header is not trivial or the header represents a more elaborate system of API components, use the `@file` operator in a block comment after includes but before the start of any other code.

### Documenting namespaces

Unless the header mainly declares one namespace. Documenting namespaces is done the same way as anything else, including the mandatory `@brief`.

### Documenting module build rules

Unlike in Unreal, MCRO encourages to describe what a particular module is and how it is used in its module-rule file, documenting its module rule class. Doxygen supports the XML Docs convention popular in .NET, so in C# that is used in compatible places. Unless for `@file` or the "license comment" which keeps using the JavaDoc comment blocks.

```CSharp
/// <summary>
/// A module containing Windows platform specific utilities like interop between Unreal and WinRT/COM
/// </summary>
public class McroWindows : ModuleRules
{
    //...
}
```

### License comments

MCRO is licensed under MPL 2.0. MPL recognises individual files as separate entities instead of GPL wich treats an entire project as a single big entity. For this reason it is very important that each source file in MCRO starts with a license comment on its top.

```Cpp
/** @noop License Comment
 *  @file
 *  @copyright
 *  This Source Code is subject to the terms of the Mozilla Public License, v2.0.
 *  If a copy of the MPL was not distributed with this file You can obtain one at
 *  https://mozilla.org/MPL/2.0/
 *  
 *  @author Jon Doe, Mary Sue
 *  @date 2025
 */
```

`@noop License Comment` part is important so other tooling can recognize the special usage of this comment block. This is also recognized and processed accordingly by Doxygen. C# files use the same style of this comment block as C++ files. If you've edited/created the file please add yourself to the comma separated list of authors.