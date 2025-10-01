# Attribution {#Attribution}

## [Compile Time Regular Expression](https://compile-time.re)

* Authors:
  * Hana Dusíková (https://github.com/hanickadot)
* License: Apache License 2.0 (https://github.com/hanickadot/compile-time-regular-expressions/blob/main/LICENSE)
* [Source code](https://github.com/hanickadot/compile-time-regular-expressions)

Reason of inclusion:  
It can do this:

```Cpp
struct FDate { std::string_view year, month, day; };

auto result = ctre::match<R"((?<year>\d{4})/(?<month>\d{1,2})/(?<day>\d{1,2}))">(s);
return FDate{ result.get<"year">(), result.get<"month">(), result.get<"day">() };
```

In compile time non-the less! Unreal's own included Regex library pales in comparison.


## [magic_enum](https://github.com/Neargye/magic_enum)

* Authors:
  * Daniil Goncharov (https://github.com/Neargye)
* License: MIT (https://github.com/Neargye/magic_enum/blob/master/LICENSE)
* [Source code](https://github.com/Neargye/magic_enum)

Reason of inclusion:  
Serialize/deserialize C++ enums in human readable form without the need to use UENUM macros and
UAT generated headers.

## [range-v3](https://ericniebler.github.io/range-v3/)

* Authors:
  * Eric Niebler (https://ericniebler.com)
* License: Boost Software License 1.0 (https://github.com/ericniebler/range-v3/blob/master/LICENSE.txt)
* [Source code](https://github.com/ericniebler/range-v3)

Reason of inclusion:  
To have a "nice" declarative way to handle views for Unreal containers

## [yaml-cpp](https://github.com/jbeder/yaml-cpp)

* Authors:
  * Jesse Beder (https://github.com/jbeder)
* License: MIT (https://github.com/jbeder/yaml-cpp/blob/master/LICENSE)
* [Source code](https://github.com/jbeder/yaml-cpp)

Reason of inclusion:  
A YAML parser and emitter in C++. Unreal Engine doesn't provide one.

## [constexpr-xxh3](https://github.com/chys87/constexpr-xxh3)

* Authors:
  * Chys (https://chys.info)
* License: BSD 2-Clause License (https://github.com/chys87/constexpr-xxh3/blob/main/LICENSE)
* [Source code](https://github.com/chys87/constexpr-xxh3)

Reason of inclusion:  
This allows us to have almost free runtime exact type checking.

## [xxHash](https://xxhash.com)

* Authors:
  * Yann Collet (http://fastcompression.blogspot.fr/)
  * Easy as PI 3.14 (https://github.com/easyaspi314)
  * Takayuki Matsuoka
  * et. al.
* License: BSD 2-Clause License (https://github.com/Cyan4973/xxHash/blob/dev/LICENSE)
* [Source code](https://github.com/Cyan4973/xxHash)

Reason of inclusion:  
xxHash algorithm is used by another third-party implementation in constexpr time. This allows us to have almost free
runtime exact type checking.


## [Intel ISPC Tasksys](https://ispc.github.io)

* Authors:
  * Intel
* License: BSD-3-Clause (https://github.com/ispc/ispc/blob/main/LICENSE.txt)
* [Source code](https://github.com/ispc/ispc/blob/main/examples/common/tasksys.cpp)

Reason of inclusion:  
To support launch, sync and task keywords of ISPC.
