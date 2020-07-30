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


#include <black/sat/z3.hpp>
#include <black/logic/alphabet.hpp>

#include <z3.h>
#include <tsl/hopscotch_map.h>

#include <limits>

namespace black::internal::sat::backends 
{
  struct z3::_z3_t {
    Z3_context context;
    Z3_solver solver;

    tsl::hopscotch_map<formula, Z3_ast> terms;

    Z3_ast to_z3(formula);
    Z3_ast to_z3_inner(formula);
  };

  [[noreturn]]
  static void error_handler(Z3_context c, Z3_error_code e) {
    fprintf(stderr, "Z3 error: %s\n", Z3_get_error_msg(c, e));
    std::abort();
  }

  z3::z3() : _data{std::make_unique<_z3_t>()} 
  { 
    Z3_config  cfg;
    
    cfg = Z3_mk_config();
    Z3_set_param_value(cfg, "model", "true");
    
    _data->context = Z3_mk_context(cfg);
    Z3_set_error_handler(_data->context, error_handler);

    Z3_del_config(cfg);

    _data->solver = Z3_mk_solver(_data->context);
    Z3_solver_inc_ref(_data->context, _data->solver);
  }

  z3::~z3() {
    Z3_solver_dec_ref(_data->context, _data->solver);
    Z3_del_context(_data->context);
  }

  void z3::assert_formula(formula f) { 
    Z3_solver_assert(_data->context, _data->solver, _data->to_z3(f));
  }
  
  bool z3::is_sat() const { 
    Z3_lbool result = Z3_solver_check(_data->context, _data->solver);

    return (result == Z3_L_TRUE);
  }
  
  void z3::push() { 
    Z3_solver_push(_data->context, _data->solver);
  }

  void z3::pop() { 
    Z3_solver_pop(_data->context, _data->solver, 1);
  }

  void z3::clear() { 
    Z3_solver_reset(_data->context, _data->solver);
  }

  // TODO: Factor out common logic with z3.cpp
  Z3_ast z3::_z3_t::to_z3(formula f) 
  {
    if(auto it = terms.find(f); it != terms.end()) 
      return it->second;

    Z3_ast term = to_z3_inner(f);
    terms.insert({f, term});

    return term;
  }

  Z3_ast z3::_z3_t::to_z3_inner(formula f) 
  {
    return f.match(
      [this](boolean b) {
        return b.value() ? Z3_mk_true(context) : Z3_mk_false(context);
      },
      [this](atom a) {
        Z3_sort sort = Z3_mk_bool_sort(context);
        Z3_symbol symbol = 
          Z3_mk_string_symbol(context, to_string(a.unique_id()).c_str());
        
        return Z3_mk_const(context, symbol, sort);
      },
      [this](negation, formula n) {
        return Z3_mk_not(context, to_z3(n));
      },
      [this](conjunction c) {
        std::vector<Z3_ast> args;

        formula next = c;
        std::optional<conjunction> cnext{c};
        do {
          formula left = cnext->left();
          next = cnext->right();
          args.push_back(to_z3(left));
        } while((cnext = next.to<conjunction>()));
        args.push_back(to_z3(next));

        black_assert(args.size() <= std::numeric_limits<unsigned int>::max());
        return Z3_mk_and(context, 
          static_cast<unsigned int>(args.size()), args.data());
      },
      [this](disjunction, formula left, formula right) {
        Z3_ast args[] = { to_z3(left), to_z3(right) };
        return Z3_mk_or(context, 2, args);
      },
      [this](then, formula left, formula right) {
        return Z3_mk_implies(context, to_z3(left), to_z3(right));
      },
      [this](iff, formula left, formula right) {
        return Z3_mk_iff(context, to_z3(left), to_z3(right));
      },
      [](otherwise) -> Z3_ast {
        black_unreachable();
      }
    );
  }

}
