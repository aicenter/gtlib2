/*
    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague

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


#ifndef GTLIB2_FUNCTOOLS_H
#define GTLIB2_FUNCTOOLS_H


#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>


/**
 * Create a zip function, similar to Python.
 *
 * It's useful for iteration over multiple arrays at the same time.
 * Beware: the arrays should be the same size. If not, it can result
 * in undefined behaviour!
 *
 * Example:
 * @code
 * std::vector <int> xs   = { 2, 3, 5, 7, 11, 13 };
 * int               ys[] = { 0, 1, 2, 3,  4,  5 };
 * std::set    <int> zs     { 1, 4, 6, 7,  8,  9 };
 * for (auto tuple : zip( xs, ys, zs ))
 * {
 *   unzip( tuple, x, y, z );
 *   // you can now use x, y, z freely in the code
 * }
 * @endcode
 *
 * @see http://www.cplusplus.com/forum/general/228918/
 */
template <typename... Containers>
auto zip( Containers&&... containers )
-> boost::iterator_range <boost::zip_iterator <decltype( boost::make_tuple( std::begin( containers )... ) )> >
{
  auto zip_begin = boost::make_zip_iterator( boost::make_tuple( std::begin( containers )... ) );
  auto zip_end   = boost::make_zip_iterator( boost::make_tuple( std::end(   containers )... ) );
  return boost::make_iterator_range( zip_begin, zip_end );
}

#ifndef NUM_ARGS
#define NUM_ARGS0(X,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1, N, ...) N
#define NUM_ARGS(...) NUM_ARGS0(0, __VA_ARGS__, 20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#endif

#define UNZIP_1(N,tuple,x)     auto x = boost::get <N-1> (tuple)
#define UNZIP_2(N,tuple,x,...) auto x = boost::get <N-2> (tuple); UNZIP_1(N,tuple,__VA_ARGS__)
#define UNZIP_3(N,tuple,x,...) auto x = boost::get <N-3> (tuple); UNZIP_2(N,tuple,__VA_ARGS__)
#define UNZIP_4(N,tuple,x,...) auto x = boost::get <N-4> (tuple); UNZIP_3(N,tuple,__VA_ARGS__)
#define UNZIP_5(N,tuple,x,...) auto x = boost::get <N-5> (tuple); UNZIP_4(N,tuple,__VA_ARGS__)
#define UNZIP_6(N,tuple,x,...) auto x = boost::get <N-6> (tuple); UNZIP_5(N,tuple,__VA_ARGS__)
#define UNZIP_7(N,tuple,x,...) auto x = boost::get <N-7> (tuple); UNZIP_6(N,tuple,__VA_ARGS__)
#define UNZIP_8(N,tuple,x,...) auto x = boost::get <N-8> (tuple); UNZIP_7(N,tuple,__VA_ARGS__)
#define UNZIP_9(N,tuple,x,...) auto x = boost::get <N-9> (tuple); UNZIP_8(N,tuple,__VA_ARGS__)

#define UNZIP1(N,tuple,...) UNZIP_ ## N(N,tuple,__VA_ARGS__)
#define UNZIP0(N,tuple,...) UNZIP1(N,tuple,__VA_ARGS__)
#define unzip(tuple,...) UNZIP0(NUM_ARGS(__VA_ARGS__),tuple,__VA_ARGS__)


#endif //GTLIB2_FUNCTOOLS_H
