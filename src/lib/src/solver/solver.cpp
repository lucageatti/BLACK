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

#include <black/logic/parser.hpp>
#include <black/solver/solver.hpp>
#include <black/solver/mathsat.hpp>

#include <fmt/format.h>

namespace black::details {



  /*
   * Incremental version of 'solve' 
   */
  bool solver::inc_solve() 
  {
    int k=0;
    while(true){
      // Generating the k-unraveling
      add_to_msat(k_unraveling(k));
      // if 'encoding' is unsat, then stop with UNSAT.
      if(!is_sat()) 
        return false;  

      // else, continue to check EMPTY and LOOP.
      // Generating EMPTY and LOOP
      msat_push_backtrack_point(env);
      add_to_msat(empty_and_loop(k));
      // if 'encoding' is sat, then stop with SAT.
      if(is_sat()) 
        return true;
      
      // else, generate the PRUNE
      // Computing allSAT of 'encoding & PRUNE^k'
      msat_pop_backtrack_point(env);
      formula all_models = all_sat(encoding && prune(k), alpha.bottom());
      // Fixing the negation of 'all_models'
      add_to_msat( !all_models );
      // Incrementing 'k' for the next iteration
      k++;
    } // end while(true)
  }


  /*
   * Main algorithm (allSAT-based) for the solver.
   */
  bool solver::solve() 
  {
    int k=0;
    formula encoding = alpha.top();
    while(true){
      // Generating the k-unraveling
      if(k)
        encoding = encoding && k_unraveling(k);
      else // first iteration
        encoding = k_unraveling(k);
      // if 'encoding' is unsat, then stop with UNSAT.
      if(!is_sat(encoding)) 
        return false;  

      // else, continue to check EMPTY and LOOP.
      // Generating EMPTY and LOOP
      formula looped = encoding && empty_and_loop(k);
      // if 'encoding' is sat, then stop with SAT.
      if(is_sat(looped)) 
        return true;
      
      // else, generate the PRUNE
      // Computing allSAT of 'encoding & PRUNE^k'
      formula all_models = all_sat(encoding && prune(k), alpha.bottom());
      // Fixing the negation of 'all_models'
      encoding = encoding && !all_models;
      // Incrementing 'k' for the next iteration
      k++;
    } // end while(true)
  }



  /*
   * Incremental version of 'bsc_prune'
   */
  bool solver::inc_bsc_prune()
  {
    int k=0;
    while(true){
      // Generating the k-unraveling
      add_to_msat(k_unraveling(k));
      // if 'encoding' is unsat, then stop with UNSAT.
      if(!is_sat()) 
        return false;
      
      // else, continue to check EMPTY and LOOP.
      // Generating EMPTY and LOOP
      msat_push_backtrack_point(env);
      add_to_msat(empty_and_loop(k));
      // if 'encoding' is sat, then stop with SAT.
      if(is_sat()) 
        return true;
      
      // else, generate the PRUNE
      // Computing allSAT of 'encoding & not PRUNE^k'
      msat_pop_backtrack_point(env);
      add_to_msat( !prune(k) );
      if(!is_sat()) 
        return false;
      
      // else, increment k
      k++;
    } // end while(true)
  }



  /*
   * BSC augmented with the PRUNE rule.
   */
  bool solver::bsc_prune()
  {
    int k=0;
    formula encoding = alpha.top();
    while(true){
      // Generating the k-unraveling
      if(k)
        encoding = encoding && k_unraveling(k);
      else // first iteration
        encoding = k_unraveling(k);
      // if 'encoding' is unsat, then stop with UNSAT.
      if(!is_sat(encoding)) 
        return false;
      
      
      // else, continue to check EMPTY and LOOP.
      // Generating EMPTY and LOOP
      formula looped = encoding && empty_and_loop(k);
      // if 'encoding' is sat, then stop with SAT.
      if(is_sat(looped)) 
        return true;
      
      // else, generate the PRUNE
      // Computing allSAT of 'encoding & not PRUNE^k'
      encoding = encoding && ( !prune(k) );
      if(!is_sat(encoding)) 
        return false;
      
      // else, increment k
      k++;
    } // end while(true)
  }



  
  /*
   * Incremental version of BSC.
   */
  bool solver::inc_bsc()
  {
    int k=0;
    
    while(true){
      // Generating the k-unraveling
      add_to_msat(k_unraveling(k))

      if(!is_sat()) 
        return false;

      // Generating EMPTY and LOOP
      msat_push_backtrack_point(env);
      add_to_msat(empty_and_loop(k));

      // if 'encoding' is sat, then stop with SAT.
      if(is_sat()) 
        return true;
      
      msat_pop_backtrack_point(env);
      k++;
    }
    
  }




  /*
   * Naive (not terminating) algorithm for Bounded
   * Satisfiability Checking. 
   */
  bool solver::bsc()
  {
    int k=0;
    formula encoding = alpha.top();
    
    while(true){
      // Generating the k-unraveling
      if(k)
        encoding = encoding && k_unraveling(k);
      else // first iteration
        encoding = k_unraveling(k);

      //fmt::print("{}-unraveling: {}\n", k, to_string(encoding));

      if(!is_sat(encoding)) 
        return false;

      // Generating EMPTY and LOOP
      formula looped = encoding && empty_and_loop(k);

      //fmt::print("{}-unraveling + Loop: {}\n", k, to_string(looped));

      // if 'encoding' is sat, then stop with SAT.
      if(is_sat(looped)) 
        return true;

      k++;
    }
  }


  // Generates the PRUNE encoding
  formula solver::prune(int k) {
    formula k_prune = alpha.bottom();
    for(int l=0; l<k-1; l++) {
      formula k_prune_inner = alpha.bottom();
      for(int j=l+1; j<k; j++) {
        formula llp = l_to_k_loop(l,j) && l_to_k_loop(j,k) && l_j_k_prune(l,j,k);
        k_prune_inner = k_prune_inner || llp;
      }
      k_prune = k_prune || k_prune_inner;
    }
    return k_prune;
  }


  // Generates the _lPRUNE_j^k encoding
  formula solver::l_j_k_prune(int l, int j, int k) {
    formula prune = alpha.top();
    for(tomorrow xreq : xrequests) {
      // If the X-requests is an X-eventuality
      if(auto req = get_xev(xreq); req) {
        // Creating the encoding
        formula first_conj = alpha.var(std::pair(formula{xreq},k));
        formula inner_impl = alpha.bottom();
        for(int i=j+1; i<=k; i++) {
          formula xnf_req = to_ground_xnf(*req, i, false);
          inner_impl = inner_impl || xnf_req;
        }
        first_conj = first_conj && inner_impl;
        formula second_conj = alpha.bottom();
        for(int i=l+1; i<=j; i++) {
          formula xnf_req = to_ground_xnf(*req, i, false);
          second_conj = second_conj || xnf_req;
        }
        prune = prune && then(first_conj, second_conj);
      }
    }
    return prune;
  }


  // Generates the EMPTY and LOOP encoding
  formula solver::empty_and_loop(int k) {
    return k_empty(k) || k_loop(k);
  }



  // Generates the encoding for EMPTY_k
  formula solver::k_empty(int k) {
    formula k_empty = alpha.top();
    for(auto it = xrequests.begin(); it != xrequests.end(); it++) {
      k_empty = k_empty && (!( alpha.var(std::pair<formula,int>(formula{*it},k)) ));
    }
    return k_empty;
  }

  std::optional<formula> solver::get_xev(tomorrow xreq) {
    return xreq.operand().match(
      [](eventually e) { return std::optional{e.operand()}; },
      [](until u) { return std::optional{u.right()}; },
      [](otherwise) { return std::optional<formula>{std::nullopt}; }
    );
  }

  // Generates the encoding for LOOP_k
  formula solver::k_loop(int k) {
    formula k_loop = alpha.bottom();
    for(int l=0; l<k; l++) {
      k_loop = k_loop || (l_to_k_loop(l,k) && l_to_k_period(l,k));
    }
    return k_loop;
  }


  // Generates the encoding for _lP_k
  formula solver::l_to_k_period(int l, int k) {
    formula period_lk = alpha.top();
    for(tomorrow xreq : xrequests) {
      // If the X-requests is an X-eventuality
      if(auto req = get_xev(xreq); req) {
        // Creating the encoding
        formula atom_phi_k = alpha.var( std::pair(formula{xreq},k) );
        formula body_impl = alpha.bottom();
        for(int i=l+1; i<=k; i++) {
          formula req_atom_i = to_ground_xnf(*req, i, false);
          body_impl = body_impl || req_atom_i;
        }
        period_lk = period_lk && then(atom_phi_k, body_impl);
      }
    }
    return period_lk;
  }


  // Generates the encoding for _lL_k
  formula solver::l_to_k_loop(int l, int k) {
    formula loop_lk = alpha.top();
    //for(auto it = xrequests.begin(); it != xrequests.end(); it++) {
    for(tomorrow xreq : xrequests) {
      formula first_atom = alpha.var( std::pair(formula{xreq},l) );
      formula second_atom = alpha.var( std::pair(formula{xreq},k) );
      // big and formula
      loop_lk = loop_lk && iff(first_atom,second_atom);
    }
    return loop_lk;
  }


  // Generates the k-unraveling for the given k.
  formula solver::k_unraveling(int k) {
    // Copy of the X-requests generated in phase k-1.
    // clears all the X-requests from the vector
    std::vector<tomorrow> current_xreq = std::move(xrequests);
    //std::swap(current_xreq, xrequests);

    if(k==0)
      return to_ground_xnf(frm,k,true);

    formula big_and = alpha.top();
    //for(auto it = current_xreq.begin(); it != current_xreq.end(); it++){
    for(tomorrow xreq : current_xreq) {
      // X(alpha)_P^{k-1}
      formula left_hand = alpha.var(std::pair(formula{xreq},k-1));
      // xnf(\alpha)_P^{k}
      formula right_hand = to_ground_xnf(xreq.operand(),k,true);
      // left_hand IFF right_hand
      big_and = big_and && iff(left_hand, right_hand);
    }
    return big_and;
  }


  // Turns the current formula into Next Normal Form
  formula solver::to_ground_xnf(formula f, int k, bool update) {
    return f.match(
      // Future Operators
      [&](boolean)        { return f; },
      [&](atom a)         { return formula{alpha.var(std::pair<formula,int>(formula{a},k))}; },
      [&,this](tomorrow t)   {
        if(update)
          xrequests.push_back(t);
        return formula{alpha.var(std::pair<formula,int>(formula{t},k))};
      },
      [&](negation n)    {
        return formula{!to_ground_xnf(n.operand(),k, update)};
      },
      [&](conjunction c) {
        return formula{
          to_ground_xnf(c.left(),k,update) &&
          to_ground_xnf(c.right(),k,update)
        };
      },
      [&](disjunction d) {
        return formula{
          to_ground_xnf(d.left(),k,update) ||
          to_ground_xnf(d.right(),k,update)
        };
      },
      [&](then d) {
        return formula{then(
          to_ground_xnf(d.left(),k,update),
          to_ground_xnf(d.right(),k,update)
        )};
      },
      [&](iff d) {
        return formula{iff(
          to_ground_xnf(d.left(),k,update),
          to_ground_xnf(d.right(),k,update)
        )};
      },
      [&,this](until u) {
        if(update)
          xrequests.push_back(X(u));

        return
          formula{to_ground_xnf(u.right(),k,update) ||
            (to_ground_xnf(u.left(),k,update) &&
              alpha.var(std::pair<formula,int>(formula{X(u)},k)))
          };
      },
      [&,this](eventually e) {
        if(update)
          xrequests.push_back(X(e));
        return
          formula{
            to_ground_xnf(e.operand(),k,update) ||
            alpha.var(std::pair<formula,int>(formula{X(e)},k))
          };
      },
      [&,this](always a) {
        if(update)
          xrequests.push_back(X(a));
        return
          formula{
            to_ground_xnf(a.operand(),k,update) &&
            alpha.var(std::pair<formula,int>(formula{X(a)},k))
          };
      },
      [&,this](release r) {
        if(update)
          xrequests.push_back(X(r));
        return formula{
          to_ground_xnf(r.right(),k,update) &&
          (to_ground_xnf(r.left(),k,update) ||
            alpha.var(std::pair<formula,int>(formula{X(r)},k)))
        };
      },
      // TODO: past operators
      [&](otherwise) -> formula {
        fmt::print("unrecognized formula: {}\n", to_string(f));
        black_unreachable();
      }
    );
  }


  // Asks MathSAT for the satisfiability of current formula
  bool solver::is_sat(formula encoding)
  {
    msat_term msat_formula = to_mathsat(env, encoding);
    
    msat_assert_formula(env, msat_formula);

    msat_result res = msat_solve(env);

    return (res == MSAT_SAT);
  }
  
  
  
  // Asks MathSAT for the satisfiability of current formula
  bool solver::is_sat()
  {
    msat_result res = msat_solve(env);
    return (res == MSAT_SAT);
  }
  
  
  
  // Asks MathSAT for a model (if any) of current formula
  // The result is given as a cube.
  formula get_model(formula f) {
    mdl = alpha.top();
    
  }


  // Simple implementation of an allSAT solver.
  formula solver::all_sat(formula f, formula models) {
    if(is_sat(f)){
      formula sigma = get_model(f);
      return all_sat(f && !sigma, models || sigma);
    }
    return models;
  }


  void add_to_msat(formula f)
  {
    return msat_assert_formula(env, to_mathsat(env,f));
  }

} // end namespace black::details
