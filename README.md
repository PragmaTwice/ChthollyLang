# ![Chtholly Lang Logo](doc/logo/logo180.png) Chtholly Lang

[![Build Status](https://www.travis-ci.org/PragmaTwice/ChthollyLang.svg?branch=master)](https://www.travis-ci.org/PragmaTwice/ChthollyLang)
[![codecov](https://codecov.io/gh/PragmaTwice/ChthollyLang/branch/master/graph/badge.svg)](https://codecov.io/gh/PragmaTwice/ChthollyLang)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/b63e484d99d24bb0965494587910ec4e)](https://www.codacy.com/app/PragmaTwice/ChthollyLang?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PragmaTwice/ChthollyLang&amp;utm_campaign=Badge_Grade)
[![Join the chat at https://gitter.im/ChthollyLang/Lobby](https://badges.gitter.im/ChthollyLang/Lobby.svg)](https://gitter.im/ChthollyLang/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


*A **simple implmentation** of Yet another script language Chtholly*

## Requires
* Compiler and std Library with [C++17 support](http://en.cppreference.com/w/cpp/compiler_support) *(necessary)*
* [Google Test](https://github.com/google/googletest) *(if you want to run [unit testings](https://github.com/PragmaTwice/ChthollyLang/tree/master/test))*

Successfully compiled and passed all tests in:
* LLVM/clang-cl 7 with MSVC 15.8/stdlib in Windows
* LLVM/clang++ 6 with libstdc++ in Linux
* GNU/g++ 7 in Linux

*Maybe other environment, but you should test and verify by yourself.*

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