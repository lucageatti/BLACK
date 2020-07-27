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

#ifndef BLACK_ALPHABET_IMPL_HPP
#define BLACK_ALPHABET_IMPL_HPP

#include <black/support/common.hpp>
#include <black/logic/formula.hpp>
#include <black/sat/mathsat.hpp>

#include <deque>
#include <unordered_map>

namespace black::internal {

  struct alphabet_impl
  {
    alphabet_impl(alphabet *sigma) : _sigma{sigma} {}

    template<typename F, typename ...Args>
    F *allocate_formula(Args&& ...args) {
      return allocate(_tag<F>{}, FWD(args)...);
    }

    alphabet            *_sigma;

    boolean_t            _top{true};
    boolean_t            _bottom{false};

    std::deque<atom_t>   _atoms;
    std::deque<unary_t>  _unaries;
    std::deque<binary_t> _binaries;

    using unary_key = std::tuple<unary::type, formula_base*>;
    using binary_key = std::tuple<binary::type,
                                  formula_base*,
                                  formula_base*>;

    std::unordered_map<any_hashable, atom_t*> _atoms_map;
    std::unordered_map<unary_key,   unary_t*> _unaries_map;
    std::unordered_map<binary_key, binary_t*> _binaries_map;

    // MathSAT environment object
    // TODO: decouple formula construction code from the SAT solver
    msat_env _msat_env = mathsat_init();

  private:
    template<typename>
    struct _tag {};

    template<typename T, REQUIRES(is_hashable<T>)>
    atom_t *allocate(_tag<atom_t>, T&& _label)
    {
      any_hashable label{FWD(_label)};

      if(auto it = _atoms_map.find(label); it != _atoms_map.end())
        return it->second;

      atom_t *a = &_atoms.emplace_back(label);
      _atoms_map.insert({label, a});

      return a;
    }

    unary_t *
    allocate(_tag<unary_t>, unary::type type, formula_base* arg)
    {
      if(auto it = _unaries_map.find({type, arg}); it != _unaries_map.end())
        return it->second;

      unary_t *f =
        &_unaries.emplace_back(static_cast<formula_type>(type), arg);
      _unaries_map.insert({{type, arg}, f});

      return f;
    }

    binary_t *
    allocate(_tag<binary_t>, binary::type type,
             formula_base* arg1, formula_base* arg2)
    {
      auto it = _binaries_map.find({type, arg1, arg2});
      if(it != _binaries_map.end())
        return it->second;

      binary_t *f =
        &_binaries.emplace_back(static_cast<formula_type>(type), arg1, arg2);
      _binaries_map.insert({{type, arg1, arg2}, f});

      return f;
    }
  };

  // Out-of-line implementation from the handle class in formula.hpp,
  // to have a complete alphabet_impl type
  template<typename H, typename F>
  template<typename FType, typename Arg>
  std::pair<black::alphabet *, unary_t *>
  handle_base<H, F>::allocate_unary(FType type, Arg const&arg)
  {
    // The type is templated only because of circularity problems
    static_assert(std::is_same_v<FType, unary::type>);

    // Get the alphabet from the argument
    black::alphabet *sigma = arg._alphabet;

    // Ask the alphabet to actually allocate the formula
    unary_t *object =
      sigma->_impl->template allocate_formula<unary_t>(type, arg._formula);

    return {sigma, object};
  }

  template<typename H, typename F>
  template<typename FType, typename Arg1, typename Arg2>
  std::pair<black::alphabet *, binary_t *>
  handle_base<H, F>::allocate_binary(FType type,
                                     Arg1 const&arg1, Arg2 const&arg2)
  {
    // The type is templated only because of circularity problems
    static_assert(std::is_same_v<FType, binary::type>);

    // Check that both arguments come from the same alphabet
    black_assert(arg1._alphabet == arg2._alphabet);

    // Get the alphabet from the first argument (same as the second, by now)
    black::alphabet *sigma = arg1._alphabet;

    // Ask the alphabet to actually allocate the formula
    binary_t *object = sigma->_impl->template allocate_formula<binary_t>(
      type, arg1._formula, arg2._formula
    );

    return {sigma, object};
  }

  //
  // Out-of-line implementation of the operators taking boolean args
  inline auto operator &&(bool f1, formula f2) {
    return conjunction(f2.alphabet()->boolean(f1), f2);
  }
  inline auto operator &&(formula f1, bool f2) {
    return conjunction(f1, f1.alphabet()->boolean(f2));
  }
  inline auto operator ||(bool f1, formula f2) {
    return disjunction(f2.alphabet()->boolean(f1), f2);
  }
  inline auto operator ||(formula f1, bool f2) {
    return disjunction(f1, f1.alphabet()->boolean(f2));
  }
  inline auto U(formula f1, bool f2) {
    return U(f1, f1.alphabet()->boolean(f2));
  }
  inline auto U(bool f1, formula f2) {
    return U(f2.alphabet()->boolean(f1), f2);
  }
  inline auto R(formula f1, bool f2) {
    return R(f1, f1.alphabet()->boolean(f2));
  }
  inline auto R(bool f1, formula f2) {
    return R(f2.alphabet()->boolean(f1), f2);
  }
  inline auto S(formula f1, bool f2) {
    return S(f1, f1.alphabet()->boolean(f2));
  }
  inline auto S(bool f1, formula f2) {
    return S(f2.alphabet()->boolean(f1), f2);
  }
  inline auto T(formula f1, bool f2) {
    return T(f1, f1.alphabet()->boolean(f2));
  }
  inline auto T(bool f1, formula f2) {
    return T(f2.alphabet()->boolean(f1), f2);
  }
  
} // namespace black::internal

namespace black {
  //
  // Out-of-line definitions of methods of class `alphabet`
  //
  inline alphabet::alphabet()
    : _impl{std::make_unique<internal::alphabet_impl>(this)} {}

  inline boolean alphabet::boolean(bool value) {
    return value ? top() : bottom();
  }

  inline boolean alphabet::top() {
    return black::internal::boolean{this, &_impl->_top};
  }

  inline boolean alphabet::bottom() {
    return black::internal::boolean{this, &_impl->_bottom};
  }

  template<typename T, REQUIRES_OUT_OF_LINE(internal::is_hashable<T>)>
  inline atom alphabet::var(T&& label) {
    using namespace internal;
    if constexpr(std::is_constructible_v<std::string,T>) {
      return
        atom{this, _impl->allocate_formula<atom_t>(std::string{FWD(label)})};
    } else {
      return atom{this, _impl->allocate_formula<atom_t>(FWD(label))};
    }
  }

  inline formula alphabet::from_id(formula_id id) {
    using namespace internal;
    return
    formula{this, reinterpret_cast<formula_base *>(static_cast<uintptr_t>(id))};
  }

  inline msat_env alphabet::mathsat_env() const {
    return _impl->_msat_env;
  }

  namespace internal {
    inline msat_term formula::to_sat() const {
      // TODO: check if the formula is propositional-only
      // TODO: check that the environment of the alphabet is the same
      if(MSAT_ERROR_TERM(_formula->encoding))
        _formula->encoding = to_mathsat(*this);

      black_assert(!MSAT_ERROR_TERM(_formula->encoding));
      return _formula->encoding;
    }
  }

} // namespace black

#endif // BLACK_ALPHABET_IMPL_HPP
