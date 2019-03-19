# Contributing

This file describes setup necessary to contribute to the `master` branch of the library.

Please submit pull requests to gitlab/github.

<!--
todo:
There is a CI pipeline which checks tests, they must first pass before the pull request can be merged to master.

At least one additional thumbs up must be made by another maintainer of the library to approve the pull request and subsequent merge to master.
  
- https://docs.gitlab.com/ee/ci/quick_start/README.html
- http://ghostlyrics.net/building-and-deploying-a-c-library-with-gitlab.html
- https://docs.gitlab.com/ee/user/project/merge_requests/merge_when_pipeline_succeeds.html
-->

<!--
todo:
## Practical advice

Domains - coding of observations etc. with binary flags!
beware: Danger of overflow!
 
 -->
 
## Development guide

Your code should read like a book. If it doesn't, do it again.

Generally follow the Zen of Python:

```
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
```

## Code style

Use Google Code Style:

- Clion: Settings -> Code style -> GoogleStyle 
- other editors: https://github.com/google/styleguide 
  - eclipse: look up `eclipse-cpp-google-style.xml`

Use indentation of 4 spaces.

## Committing code

In CLion, turn on

- [x] Reformat code (only if you have used the ^^ code style!)
- [x] Rearrange code
- [x] Optimize imports
- [x] Perform code analysis

Especially, deal with the problems that turn up with code analysis! 

### Linter

cpplint - cpplint.py is a tool that reads a source file and identifies many (Google c++) style errors. It is not perfect, and has both false positives and false negatives, but it is still a valuable tool. False positives can be ignored by putting // NOLINT at the end of the line or // NOLINTNEXTLINE in the previous line. You can install it via pip: 

    pip install cpplint
    
Then just call

    cpplint source_file.cpp
    
Setup CLion to use the linter automatically:

https://plugins.jetbrains.com/plugin/7871-clion-cpplint

Follow the installation instructions (locate python and cpplint.py)
Mine was located in:

    /home/michal/.python_envs/gt/bin/python
    /home/michal/.python_envs/gt/lib/python3.6/site-packages/cpplint.py

Set linter options to:

    --linelength=100 --verbose=0

## Header files
Please put following in your automatic header file:

```
/*
    Copyright 2017-$YEAR Faculty of Electrical Engineering at CTU in Prague

    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


```
  
How to:  

- Clion / Intellij Idea:
  `Settings / Editor / File and Code Templates / Includes / File Header (adjusting the header of the class)`
- Eclipse: https://stackoverflow.com/questions/2604424/how-can-i-add-a-default-header-to-my-source-files-automatically-in-eclipse
  
