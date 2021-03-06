//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Luca Geatti
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

#ifndef BLACK_SOLVER_HPP
#define BLACK_SOLVER_HPP

#include <black/support/common.hpp>
#include <black/logic/formula.hpp>
#include <black/logic/alphabet.hpp>
#include <black/support/tribool.hpp>

#include <vector>
#include <utility>
#include <limits>
#include <unordered_set>
#include <string>
#include <numeric>

namespace black::internal {

  // main solver class
  class BLACK_EXPORT solver 
  {
    public:
      friend class model;

      // Constructor and destructor
      solver();
      ~solver();

      // Sets the formula to solve.
      // If `finite` is true, it is interpreted over finite models
      void set_formula(formula f, bool finite = false);

      // Solve the formula with up to `k_max' iterations
      // returns tribool::undef if `k_max` is reached
      tribool solve(size_t k_max = std::numeric_limits<size_t>::max());

      // Returns the model of the formula, if the last call to solve() 
      // returned true
      std::optional<class model> model() const;

      // Returns the last bound tried by the algorithm. The value returned 
      // does not make sense before the first call to solve()
      size_t last_bound() const;

      // Choose the SAT backend. The backend must exist.
      void set_sat_backend(std::string name);

      // Retrieve the current SAT backend
      std::string sat_backend() const;

    private:
      struct _solver_t;
      std::unique_ptr<_solver_t> _data;

  }; // end class Black Solver

  class BLACK_EXPORT model
  {
    public:
      size_t size() const;
      size_t loop() const;
      tribool value(atom a, size_t t) const;
    private:
      friend class solver;
      model(solver const&s) : _solver{s} { }
      
      solver const&_solver;
  };

} // end namespace black::internal

// Names exported to the user
namespace black {
  using internal::solver;
}

#endif // SOLVER_HPP
