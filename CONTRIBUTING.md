# Contributing

This file describes setup necessary to contribute to the `master` branch of the library.

Please submit pull requests to gitlab/github.

## Development guide

Your code should read like a book. If it doesn't, do it again.

Generally follow the Zen of Python:

    Beautiful is better than ugly.
    Explicit is better than implicit.
    Simple is better than complex.
    Complex is better than complicated.
    Flat is better than nested.
    Sparse is better than dense.
    Readability counts.
    Special cases aren't special enough to break the rules.
    Although practicality beats purity.
    Errors should never pass silently.
    Unless explicitly silenced.
    In the face of ambiguity, refuse the temptation to guess.
    There should be one-- and preferably only one --obvious way to do it.
    Although that way may not be obvious at first unless you're Dutch.
    Now is better than never.
    Although never is often better than *right* now.
    If the implementation is hard to explain, it's a bad idea.
    If the implementation is easy to explain, it may be a good idea.
    Namespaces are one honking great idea -- let's do more of those!


## Code style

Use Google Code Style:

- Clion: Settings -> Code style -> GoogleStyle 
- other editors: https://github.com/google/styleguide 
  - eclipse: look up `eclipse-cpp-google-style.xml`

Use indentation of 4 spaces.

## Committing code

In CLion, turn on

- Reformat code (only if you have used the ^^ code style!)
- Rearrange code
- Optimize imports
- Perform code analysis

Especially, deal with the problems that turn up with code analysis! 

## Header files
Please put following in your automatic header file:

```
/*
    Copyright $YEAR Faculty of Electrical Engineering at CTU in Prague
    
    This file is part of Game Theoretic Library.
    
    Game Theoretic Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.  If not, see <http://www.gnu.org/licenses/>.
*/


```
  
How to:  

- Clion / Intellij Idea:
  `Settings / Editor / File and Code Templates / Includes / File Header (adjusting the header of the class)`
- Eclipse: https://stackoverflow.com/questions/2604424/how-can-i-add-a-default-header-to-my-source-files-automatically-in-eclipse
  
