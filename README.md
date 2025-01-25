<div align="center">

<img src="Docs/Images/proto-logo-0.webp" width=400 />

# MCRO
A C++23 utility proto-plugin for Unreal Engine, for a more civilised age.

</div>

> [!CAUTION]
> This library is far from being production ready and is not recommended to be used yet at all

- [MCRO](#mcro)
  - [What's a proto-plugin?](#whats-a-proto-plugin)
  - [What's up with `_Origin` suffix everywhere?](#whats-up-with-_origin-suffix-everywhere)
  - [What it can do?](#what-it-can-do)
    - [Error handling](#error-handling)
    - [Delegate type inferance](#delegate-type-inferance)
    - [Advanced `TEventDelegate`](#advanced-teventdelegate)
    - [`TTypeName`](#ttypename)
    - [Auto modular features](#auto-modular-features)
    - [Observable `TState`](#observable-tstate)
    - [Function Traits](#function-traits)
    - [Concepts](#concepts)
    - [Extending the Slate declarative syntax](#extending-the-slate-declarative-syntax)
    - [Text interop](#text-interop)
    - [ISPC parallel tasks support](#ispc-parallel-tasks-support)
    - [Last but not least](#last-but-not-least)
  - [Plans](#plans)
  - [Legal](#legal)

[Read the full documentation at](https://mcro.de/mcro)  
*(Note that currently it's also very WIP)*

## What's a proto-plugin?

This repository is not meant to be used as its own full featured Unreal Plugin, but rather other plugins can compose this one into themselves using the [Folder Composition](https://github.com/microdee/md.Nuke.Cola?tab=readme-ov-file#folder-composition) feature provided by [Nuke.Unreal](https://github.com/microdee/Nuke.Unreal) (via [Nuke.Cola](https://github.com/microdee/md.Nuke.Cola)). The recommended way to use this is via using a simple Nuke.Cola build-plugin which inherits `IUseMcro` for example:

```CSharp
using Nuke.Common;
using Nuke.Cola;
using Nuke.Cola.BuildPlugins;
using Nuke.Unreal;

[ImplicitBuildInterface]
public interface IUseMcroInMyProject : IUseMcro
{
    Target UseMcro => _ => _
        .McroGraph()
        .DependentFor<UnrealBuild>(b => b.Prepare)
        .Executes(() =>
        {
            UseMcroAt(this.ScriptFolder(), "MyProjectSuffix");
        });
}
```

If this seems painful please blame Epic Games who decided to not allow ~~marketplace~~/Fab plugins to depend on each other, or at least depend on free and open source plugins from other sources. So I had to come up with all this "Folder Composition" nonsense so end-users might be able to use multiple of my plugins sharing common dependencies without module name conflicts.

To use MCRO as its own plugin without the need for Nuke.Unreal, see this repository: **(DOESN'T EXIST YET)**

## What's up with `_Origin` suffix everywhere?

When this proto plugin is imported into other plugins, this suffix is used for disambiguation, in case the end-user uses multiple pre-compiled plugins depending on MCRO. If this seems annoying please refer to the paragraph earlier.

## What it can do?

Here are some code appetizers without going too deep into their details. The demonstrated features usually can do a lot more than what's shown here.

### Error handling

An elegant workflow for dealing with code which can and probably will fail. It allows the developer to record the circumstances of an error, record its propagation accross components which are affected, add suggestions to the user or fellow developers for fixing the error, and display that with a modal Slate window, or print it into logs following a YAML structure.

```Cpp
#include "Mcro/Common.h"

FCanFail FK4ADevice::Tick_Sync()
{
    using namespace Mcro::Common;

    ASSERT_RETURN(Device.is_valid())
        ->AsCrashing()
        ->BreakDebugger()
        ->WithCppStackTrace()
        ->Notify(LastError);

    try
    {
        k4a::capture capture;
        ASSERT_RETURN(Device.get_capture(&capture))->Notify(LastError);

        if (auto color = capture.get_color_image(); ColorStream->Active)
        {
            PROPAGATE_FAIL(ColorStream->Push(color))->Notify(LastError);
        }
        if (auto depth = capture.get_depth_image(); DepthStream->Active)
        {
            PROPAGATE_FAIL(DepthStream->Push(depth))->Notify(LastError);
        }
        if (auto ir = capture.get_ir_image(); IRStream->Active)
        {
            PROPAGATE_FAIL(IRStream->Push(ir))->Notify(LastError);
        }
    }
    catch (k4a::error const& exception)
    {
        return IError::Make(new TCppException(exception))
            ->AsFatal()->WithCppStackTrace()->WithLocation()
            ->Notify(LastError);
    }
    return Success();
}
```

### Text macros without parenthesis

```Cpp
FString myVar(TEXT_"Hello world!");
UE_LOG(LogTemp, Display, TEXT_"%s", *myVar);

          FText myText   = INVTEXT_"Even FText macros are there";        // -> FText
          auto  myString = STRING_"Typed literals";                      // -> FString
          auto  myName   = NAME_"FName string literals";                 // -> FName
constexpr auto  myView   = TEXTVIEW_"String view created in constexpr";  // -> FStringView
constexpr auto  stdView  = STDVIEW_"TCHAR dependent STL string literal"; // -> std::[w]string_view
```

<details><summary>Vanilla:</summary>

```Cpp
FString myVar(TEXT("Hello world!"));
UE_LOG(LogTemp, Display, TEXT("%s"), *myVar);

          FText   myText = INVTEXT("Even FText macros are there");
          FString myString(TEXT("Typed literals"));
          FName   myName  (TEXT("FName string literals"));
constexpr auto    myView = TEXTVIEW("String view created in constexpr"); // -> FStringView
// there are no vanilla utilities for cross-TCHAR STL string handling
```

</details>

One of the oldest eye-sores I have with Unreal Engine source code is the `TEXT()` macro, especially the extra pair of parenthesis it requires around string literals. Unfortunately it is impossible to achieve the same goal without any preprocessor, that would be ideal, but one can exploit C++ string literal concatenation rules for simple `TCHAR` strings, or operator overloads for `FText`.

The earlier was even a suggestion of Microsoft [when they announced their standard compliant new preprocessor](https://learn.microsoft.com/en-us/cpp/preprocessor/preprocessor-experimental-overview?view=msvc-170#lval), and asked the general public to stop exploiting the behaviors of the old one.

### Delegate type inference

In `Mcro::Delegates` namespace there's a bunch of overloads of `From` function which can infer a delegate type and its intended binding only based on its input arguments. It means a couple of things:

1. The potentially lengthy delegate types are no longer needed to be repeated.
2. Creating an alias for each (multicast) delegate is no longer a must have.

For example given an imaginary type with a terrible API for some reason

```Cpp
struct FObservable
{
    using FOnStateResetTheFirstTimeEvent = TMulticastDelegate<void(FObservable const&)>;
    FOnStateResetTheFirstTimeEvent OnStateResetTheFirstTimeEvent;

    using FDefaultInitializerDelegate = TDelegate<FString(FObservable const&)>;
    void SetDefaultInitializer(FDefaultInitializerDelegate const& defaultInit)
    {
        // etc ...
    }

    // etc...
}
```

we can do

```Cpp
#include "Mcro/Common.h"

struct FListener : TSharedFromThis<FListener>
{
    TMulticastDelegate<void(FObservable const&)> PropagateEvent;
    void OnFirstStateReset(FObservable const& observable, FString const& capturedData)
    void BindTo(FObservable& observable)
    {
        using namespace Mcro::Delegates::InferDelegate;

        // Repeating the delegate type on call site is not necessary
        observable.SetDefaultInitializer(From(this, [this](FObservable const&) -> FString
        {
            return TEXT_"Some default value";
        }));

        // Chaining events like this is a single line
        observable.OnStateResetTheFirstTimeEvent.Add(From(this, PropagateEvent));

        // No need to decide on the method of binding, the most suitable one is chosen via templating
        observable.OnStateResetTheFirstTimeEvent.Add(From(this, &FListener::OnFirstStateReset, TEXT_"Capture this"));
    }
}
```

<details><summary>Equivalent vanilla Unreal delegates:</summary>

```Cpp
struct FListener : TSharedFromThis<FListener>
{
    TMulticastDelegate<void(FObservable const&)> PropagateEvent;
    void OnFirstStateReset(FObservable const& observable, FString const& capturedData)
    void BindTo(FObservable& observable)
    {
        // Delegate API in these situations is pretty verbose
        observable.SetDefaultInitializer(FDefaultInitializerDelegate::CreateSPLambda(this, [this](FObservable const&) -> FString
        {
            return TEXT_"Some default value";
        }));

        // Chaining events is a new Feature introduced with From
        observable.OnStateResetTheFirstTimeEvent.AddSPLambda(this, [this](FObservable const& observableArg)
        {
            PropagateEvent.Broadcast(observableArg);
        });

        // Ok here the difference is almost nothing
        observable.OnStateResetTheFirstTimeEvent.AddSP(this, &FListener::OnFirstStateReset, TEXT_"Capture this");
    }
}
```

</details>

There's also a dynamic / native (multicast) delegate interop including similar chaining demonstrated here.

### Advanced `TEventDelegate`

Did your thing ever load after an event which your thing depends on, but now you have to somehow detect that the event has already happened and you have to execute your thing manually? With `Mcro::Delegates::TEventDelegate` this situation has a lot less friction:

```Cpp
#include "Mcro/Common.h"
using namespace Mcro::Common::With::InferDelegate;

// TBelatedEventDelegate is an alias for TEventDelegate<Signature, BelatedInvoke>
TBelatedEventDelegate<void(int)> SomeEvent;

// Broadcast first
SomeEvent.Broadcast(42);

// Late subscribers will not be left behind
SomeEvent.Add(From([](int value)
{
    UE_LOG(LogTemp, Display, TEXT_"The last argument this event broadcasted with: %d", value);
}));
// -> The last argument this event broadcasted with: 42

// This event doesn't have this behavior set by default
TEventDelegate<void(int)> SomeOtherEvent;

// Broadcast first
SomeOtherEvent.Broadcast(1337);

// Late subscribers still can explicitly ask to be notified
SomeOtherEvent.Add(
    From([](int value)
    {
        UE_LOG(LogTemp, Display, TEXT_"The last argument this event broadcasted with: %d", value);
    }),
    BelatedInvoke
);
// -> The last argument this event broadcasted with: 1337
```

Or your thing only needs to do its tasks on the first time a frequently invoked event is triggered?

```Cpp
#include "Mcro/Common.h"
using namespace Mcro::Common::With::InferDelegate;

TEventDelegate<void(int)> SomeFrequentEvent;

// This function should only be called the first time after its binding
SomeFrequentEvent.Add(
    From([](int value)
    {
        UE_LOG(LogTemp, Display, TEXT_"This value is printed only once: %d", value);
    }),
    InvokeOnce
);

SomeFrequentEvent.Broadcast(1);
// -> This value is printed only once: 1
SomeFrequentEvent.Broadcast(2);
// (nothing is executed here)
```

Chaining events?

```Cpp
#include "Mcro/Common.h"
using namespace Mcro::Common::With::InferDelegate;

TMulticastDelegate<void(int)> LowerLevelEvent;
TEventDelegate<void(int)> HigherLevelEvent;

HigherLevelEvent.Add(From([](int value)
{
    UE_LOG(LogTemp, Display, TEXT_"Value: %d", value);
}));
LowerLevelEvent.Add(HigherLevelEvent.Delegation(this /* optionally bind an object */));

LoverLevelEvent.Broadcast(42);
// -> Value: 42
```

Of course the above chaining can be combined with belated~ or one-time invocations. 

### `TTypeName`

C++ 20 can do string manipulation in compile time, [including regex](https://github.com/hanickadot/compile-time-regular-expressions). With that, compiler specific "pretty function" macros become a key tool for simple static reflection. Based on that MCRO has `TTypeName`

```Cpp
#include "Mcro/CommonCore.h"
using namespace Mcro::Common;

struct FMyType
{
    FStringView GetTypeString() { return TTypeName<FMyType>; } // -> "FMyType" as view
}
```

even better with C++ 23 deducing this

```Cpp
#include "Mcro/CommonCore.h"
using namespace Mcro::Common;

struct FMyBaseType
{
    // returns const-ref to a templated global variable
    template <typename Self>
    FString const& GetTypeString(this Self&&) const { return TTypeString<Self>; }
}

struct FMyDerivedType : FMyBaseType {}

FMyDerivedType myVar;
UE_LOG(LogTemp, Display, TEXT_"This is `%s`", *myVar.GetTypeString());
// -> This is `FMyDerivedType`

// BUT!

FMyBaseType const& myBaseVar = myVar;
UE_LOG(LogTemp, Display, TEXT_"This is `%s`", *myBaseVar.GetTypeString());
// -> This is `FMyBaseType`
```

MCRO provides a base type `IHaveType` for storing the final type as an `FName` to avoid situations like above

```Cpp
#include "Mcro/CommonCore.h"
using namespace Mcro::Common;

struct FMyBaseType : IHaveType {}

struct FMyDerivedType : FMyBaseType
{
    FMyDerivedType() { SetType(); }
}

FMyDerivedType myVar;
UE_LOG(LogTemp, Display, TEXT_"This is as expected a `%s`", *myVar.GetType().ToString());
// -> This is as expected `FMyDerivedType`

FMyBaseType const& myBaseVar = myVar;
UE_LOG(
    LogTemp, Display,
    TEXT_"But asking the base type will still preserve `%s`",
    *myBaseVar.GetType().ToString()
);
// -> But asking the base type will still preserve `FMyDerivedType`
```

### Auto modular features

One use of `TTypeName` is making modular features easier to use:

```Cpp
#include "Mcro/Common.h"
using namespace Mcro::Common;

// In central API module
class IProblemSolvingFeature : public TAutoModularFeature<IProblemSolvingFeature>
{
public:
    virtual void SolveAllProblems() = 0;
};

// In another implementation specific module
// Define implementation first
class FProblemSolver : public IProblemSolvingFeature, public IFeatureImplementation
{
public:
    FProblemSolver() { Register(); }
    
    virtual void SolveAllProblems() override;
};

// Create one global instance
FProblemSolver GProblemSolver;

// In some other place, which doesn't know about the above implementation, the feature is used:
IProblemSolvingFeature::Get().SolveAllProblems();
```

Notice how the feature name has never needed to be explicitly specified as a string, because the type name is used under the hood.

### Observable `TState`

You have data members of your class, but you also want to notify others about how that's changing?

```Cpp
#include "Mcro/Common.h"
using namespace Mcro::Common::With::InferDelegate;

struct FMyStuff
{
    TState<int, StorePrevious> State {-1};

    FMyStuff()
    {
        // Get previous values as well when `StorePrevious` flag is active
        State.OnChange([](int next, TOptional<int> previous)
        {
            // If `StorePrevious` flag is active we should always have a value in `previous`
            // so we should never see -2
            UE_LOG(LogTemp, Display, TEXT_"Changed from %d to %d", previous.Get(-2), next);
        });

        // Listen to change only the first time
        State.OnChange(
            [](int next) { UE_LOG(LogTemp, Display, TEXT_"The first changed value is %d", next); },
            InvokeOnce
        );
    }

    void Update(int input)
    {
        if (State.HasChangedFrom(input))
        {
            // Do things only when input value has changed from previous update
            // Combine it with short-circuiting maybe ;)
        }
    }
};

struct FFoobar : TSharedFromThis<FFoobar>
{
    FMyStuff MyStuff;

    void Thing(TChangeData<T> const& change, FString const& capture)
    {
        UE_LOG(LogTemp, Display, TEXT_"React to %d with %s", change.Next, *capture);
    }

    void Do()
    {
        MyStuff.Update(1);
        // -> Changed from -1 to 1
        // -> The first changed value is 1
        MyStuff.Update(2);
        // -> Changed from 1 to 2
        MyStuff.Update(2);
        // (nothing is logged as previous update was also 2)
        MyStuff.OnChange(
            [](int next) { UE_LOG(LogTemp, Display, TEXT_"Arriving late %d", next); },
            BelatedInvoke
        );
        // -> Arriving late 2
        MyStuff.Update(3);
        // -> Changed from 2 to 3
        // -> Arriving late 3
        MyStuff.OnChange(From(this, &FFoobar::Thing, TEXT_"a unicorn"));
        MyStuff.Update(4)
        // -> Changed from 3 to 4
        // -> Arriving late 4
        // -> React to 4 with a unicorn
    }
}

```

### Function Traits

Make templates dealing with function types more readable and yet more versatile via the `Mcro::FunctionTraits` namespace. This is the foundation of many intricate teemplates Mcro can offer.

Constraint/infer template parameters from the signature of an input function. (This is an annotated exempt from `Mcro::UObjects::Init`)

```Cpp
#include "Mcro/FunctionTraits.h" // it also brings in Concepts

namespace Mcro::UObjects::Init
{
	using namespace Mcro::FunctionTraits;

    template <
        // CFunctorObject constraints to a lambda function
        // use CFunctionLike to allow anything callable including function pointers
        CFunctorObject Initializer,

        // Get the first argument of `Initializer`
        typename Argument = TFunction_Arg<Initializer, 0>,

        // That argument must be a UObject, and also cache its type
        CUObject Result = std::decay_t<Argument>
    >
    // That argument  must also be an l-value ref
    requires std::is_lvalue_reference_v<Argument>
    Result* Construct(FConstructObjectParameters&& params, Initializer&& init)
    { ... }
}
```

### Concepts

There's a copy of the C++ 20 STL Concepts library in `Mcro::Concepts` namespace but with more Unreal friendly nameing. Also it adds some conveniance concepts, like `*Decayed` versions of type constraints, or some Unreal specific constraints like `CSharedRefOrPtr` or `CUObject`

### Extending the Slate declarative syntax

`Mcro::Slate::AttributeAppend` adds the `/` operator to be used in Slate UI declarations, which can work with functions describing a common block of attributes for given widget.

```Cpp
#include "Mcro/Common";
using namespace Mcro::Common::With::AttributeAppend;

// Define a reusable block of attributes
auto Text(FString const& text) -> TAttributeBlock<STextBlock>
{
    return [&](STextBlock::FArguments& args) -> auto&
    {
        return args
        . Text(FText::FromString(text))
        . Font(FCoreStyle::GetDefaultFontStyle("Mono", 12));
    };
}

// Nest these blocks
auto OptionalText(FString const& text) -> TAttributeBlock<STextBlock>
{
    return [&](STextBlock::FArguments& args) -> auto&
    {
        return args
            . Visibility(IsVisible(!text.IsEmpty()))
            / Text(text);
    };
}

// Accept attribute blocks as arguments
auto ExpandableText(
    FText const& title,
    FString const& text,
    TAttributeBlock<STextBlock> const& textAttributes
) -> TAttributeBlock<SExpandableArea>
{
    return [&](SExpandableArea::FArguments& args) -> auto&
    {
        return args
            . Visibility(IsVisible(!text.IsEmpty()))
            . AreaTitle(title)
            . InitiallyCollapsed(true)
            . BodyContent()
            [
                SNew(STextBlock) / textAttributes
            ];
    };
}
```

Or add slots right in the Slate declarative syntax derived from an input array:

```Cpp
// This is just a convenience function so we don't repeat ourselves
auto Row() -> SVerticalBox::FSlot::FSlotArguments
{
    return MoveTemp(SVerticalBox::Slot()
        . HAlign(HAlign_Fill)
        . AutoHeight()
    );
}

void Construct(FArguments const& inArgs)
{
    ChildSlot
    [
        SNew(SVerticalBox)
        + Row()[ SNew(STextBlock) / Text(TEXT_"Items:") ]
        + TSlots(inArgs._Items.Get(), [&](FString const& item)
        {
            return MoveTemp(
                Row()[ SNew(STextBlock) / Text(item) ]
            );
        })
        + Row()[ SNew(STextBlock) / Text(TEXT_"Fin.") ]
    ];
}
```

### Text interop

The `Mcro::Text` namespace provides some handy text templating and conversion utilities and interop between Unreal string and std::strings for third-party libraries.

```Cpp
#include "Mcro/CommonCore";
using namespace Mcro::Common;

// Accept many string types at once
template <CStringOrViewOrName String>
bool GetSomethingCommon(String&& input) { ... }

// Work with std::string types
template <CStdStringOrViewInvariant String>
size_t GetLength(String&& input) { return input.size(); }
```

```Cpp
// Type alias for choosing a std::string which is best matching the current TCHAR.
FStdString foo(TEXT_"bar"); // -> std::wstring (on Windows at least)
```

```Cpp
FString foo(TEXT_"bar");
std::string fooNarrow = StdConvertUtf8(foo);
```

### ISPC parallel tasks support

`McroISPC` module gives support for `task` and `launch` keywords of the ISPC language

```C
task void MakeLookupUVLine(
    uniform uint32 buffer[],
    uniform uint32 width
) {
    uniform uint32 lineStart = taskIndex0 * width;
    foreach (i = 0 ... width)
    {
        varying uint32 address = lineStart + i;
        buffer[address] = (i << 16) | taskIndex0;
    }
}

export void MakeLookupUV(
    uniform uint32 buffer[],
    uniform uint32 width,
    uniform uint32 height
) {
    launch [height] MakeLookupUVLine(buffer, width);
}
```

### Last but not least

* Dynamic ↔ Native (multicast) delegate interop
* Event multiplexing to virtual function, native event, dynamic event
* Object binding and promises for `AsyncTask`
* Bullet-proof third-party library include guards.
* Rudimentary rendering utilities
* In-place lambda initializers for both C++ objects and UObjects
* RAII DLL loaders
* `IObservableModule`
* Universal API to get subsystems
* Shared object utilities
* `FTimespan` literals
* YAML utilities (via [yaml-cpp](https://github.com/jbeder/yaml-cpp))
* Windows:
  * `IError` wrapper for `HRESULT` and `GetLastError` extracting human readable error messages from them.

## Plans

* [ ] Doxygen generated "nice" documentation 
* [ ] Supporting common platform specific chores
  * [ ] Windows
    * [ ] Common COM / WinRT utilities
  * [ ] Linux
  * [ ] Mac
  * [ ] Android
  * [ ] iOS / TVOS / VisionOS
* [ ] Test coverage which can be taken seriously
* [ ] Graphics utilities
  * [ ] Shared textures but without the Unreal TextureShare library

## Legal
[Traits.h](../../Plugins/Yanui/Source/Mcro/Mcro_Yn/Public/Mcro/Delegates/Traits.h)
[Third-party components used by MCRO](ATTRIBUTION.md)

When using MCRO in a plugin distributed via Fab, the components above must be listed upon submission.

In addition the following tools and .NET libraries are used for build tooling:

* [NUKE](https://nuke.build)
* [Scriban](https://github.com/scriban/scriban)
* [Humanizer](https://github.com/Humanizr/Humanizer)

And documentation:

* [Doxygen](https://www.doxygen.nl/index.html)
* [Doxygen Awesome](https://jothepro.github.io/doxygen-awesome-css/)

This library is distributed under the **Mozilla Public License Version 2.0** open-source software license. Each source code file where this license is applicable contains a comment about this.

Modifying those files or the entire library for commercial purposes is allowed but those modifications should also be published free and open-source under the same license. However files added by third-party for commercial purposes interfacing with the original files under MPL v2.0 are not affected by MPL v2.0 and therefore can have any form of copyright the third-party may choose.

Using this library in any product without modifications doesn't limit how that product may be licensed.

[Full text of the license](LICENSE)

<div align="center">

<img src="Docs/Images/proto-logo-1.webp" width=400 />

</div>