//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

#include <black/logic/formula.hpp>
#include <black/logic/alphabet.hpp>

#include <vector>
#include <initializer_list>

namespace black::internal 
{
  struct literal {
    bool sign;
    struct atom atom;
  };

  struct clause {
    std::vector<literal> literals;

    clause() { }
    clause(std::vector<literal> lits) : literals(std::move(lits)) { }
    clause(std::initializer_list<literal> elems) : literals{elems} { }
  };
  
  struct cnf {
    std::vector<clause> clauses;

    cnf() { }
    cnf(std::vector<clause> cls) : clauses(std::move(cls)) { }
    cnf(std::initializer_list<clause> elems) : clauses{elems} { }
  };

  cnf to_cnf(formula f);
  formula to_formula(literal lit);
  formula to_formula(alphabet &sigma, clause c);
  formula to_formula(alphabet &sigma, cnf c);
}

namespace black {
  using internal::literal;
  using internal::clause;
  using internal::cnf;
  using internal::to_cnf;
  using internal::to_formula;
}