//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>

#include <fmt/format.h>

#include <black/support/common.hpp>
#include <black/logic/alphabet.hpp>
#include <black/logic/formula.hpp>

int main()
{
  using namespace black::details;

  static_assert(is_nullary_formula<truth_t>);
  static_assert(is_unary_formula<tomorrow_t>);
  static_assert(is_binary_formula<conjunction_t>);

  alphabet sigma;

  atom p = sigma.var(), q = sigma.var();

  formula f = conjunction(p, q);

  f.match(
    [](conjunction) { fmt::print("A conjunction\n"); },
    [](atom)        { fmt::print("An atom\n");       },
    [](truth)       { fmt::print("True\n");          },
    [](auto) {
      fmt::print("Something else\n");
    }
  );

  fmt::print("Changing the world, one solver at the time...\n");

  return 0;
}