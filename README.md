# ![Chtholly Lang Logo](doc/logo/logo180.png) Chtholly Lang

[![Join the chat at https://gitter.im/ChthollyLang/Lobby](https://badges.gitter.im/ChthollyLang/Lobby.svg)](https://gitter.im/ChthollyLang/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

![build-status](https://travis-ci.org/PragmaTwice/ChthollyLang.svg?branch=master)
*A **simple implmentation** of Yet another script language Chtholly*

## Requires
* Compiler with [C++17 support](http://en.cppreference.com/w/cpp/compiler_support) *(necessary)*
* [Google Test](https://github.com/google/googletest) *(if you want to run [unit testings](https://github.com/PragmaTwice/ChthollyLang/tree/master/test))*

*You can use MSVC 2017 15.5+ to compile it without any modification*

*Because of terrible support for C++17 in MSVC 15.5, I did some ill-looking work to make it compile successfully*

## Language Feature
* No statement but all expression
* Simple but complete grammar

## Implmentation Feature
* Functional usage of C++ (with little consideration of runtime performance)
* Parser Combinator for LL Parser Generation

## Notebook

### 2017/12/2
Just nearly finish parser

### 2017/12/10
Fuck you, MSVC ICE

### 2018/2/26
Finish parser development

### 2018/6/18
Replace MSVC/CL by LLVM/Clang