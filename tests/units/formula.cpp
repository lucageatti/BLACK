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

#include <catch2/catch.hpp>

#include <black/logic/formula.hpp>
#include <black/logic/alphabet.hpp>
#include <black/logic/parser.hpp>

#include <string>
#include <string_view>

using namespace black;
using namespace std::literals;

TEST_CASE("Handles")
{
  alphabet sigma;

  boolean top = sigma.top();
  boolean bottom = sigma.bottom();

  // requesting atoms with different types
  atom p = sigma.var("p");
  atom q = sigma.var("q"sv);
  atom ftwo = sigma.var(42);

  SECTION("Basic booleans and atoms allocation") {
    REQUIRE(top.value());
    REQUIRE(!bottom.value());
    REQUIRE(p.label<std::string>().value() == "p");
    REQUIRE(q.label<std::string>().value() == "q");
    REQUIRE(ftwo.label<int>().value() == 42);
    REQUIRE(p == p);
    REQUIRE(p != q);

    atom another = sigma.var("p"sv);

    REQUIRE(another == p);
  }

  formula ftop = top;
  formula fbottom = bottom;
  formula fp = p;
  formula fq = q;

  REQUIRE(ftop == ftop);
  REQUIRE(ftop != fbottom);

  SECTION("Formula casting and type checking") {
    REQUIRE(ftop.is<boolean>());
    REQUIRE(fbottom.is<boolean>());
    REQUIRE(fp.is<atom>());
    REQUIRE(fq.is<atom>());

    std::optional<boolean> otop = ftop.to<boolean>();
    std::optional<atom> op = fp.to<atom>();
    std::optional<boolean> opwrong = fp.to<boolean>();

    REQUIRE(otop.has_value());
    REQUIRE(op.has_value());
    REQUIRE(!opwrong.has_value());

    boolean top2 = *otop;
    atom p2 = *op;

    REQUIRE(top2.value());
    REQUIRE(top2 == top);
    REQUIRE(p2 == p);

    REQUIRE(p2.label<std::string>().value() == "p");
  }

  SECTION("Unary and binary formulas construction") {
    formula fnotp  = unary(unary::type::negation, p);
    formula fnotp2 = unary(unary::type::negation, fp);
    formula fnotq  = unary(unary::type::negation, fq);

    REQUIRE(fnotp != fnotq);
    REQUIRE(fnotp == fnotp2); // not implemented yet

    REQUIRE(fnotp.is<unary>());
    REQUIRE(fnotp2.is<unary>());
    REQUIRE(fnotp.is<negation>());
    REQUIRE(!fnotp.is<tomorrow>());

    std::optional<unary> onotp = fnotp.to<unary>();

    REQUIRE(onotp.has_value());
    unary notp2 = *onotp;

    REQUIRE(notp2.operand() == p);
    REQUIRE(notp2.operand() == fp);
  }

  SECTION("Boolean args in binary operators") {
    formula f1 = p && top;
    formula b1 = p && true;
    formula f2 = top && p;
    formula b2 = true && p;
    formula f3 = p && bottom;
    formula b3 = p && false;
    formula f4 = bottom && p;
    formula b4 = false && p;

    REQUIRE(f1 == b1);
    REQUIRE(f2 == b2);
    REQUIRE(f3 == b3);
    REQUIRE(f4 == b4);
  }

  SECTION("Type-specific handles") {
    auto neg = negation(p);
    auto next = tomorrow(p);
    formula fneg = neg;
    formula fnext = next;
    formula fland = conjunction(neg, fnext);

    REQUIRE(fneg.is<negation>());
    REQUIRE(fnext.is<tomorrow>());
    REQUIRE(!fnext.is<negation>());
    REQUIRE(fland.is<conjunction>());
  }

  SECTION("Formula pattern matching")
  {
    auto match = [&](formula f) {
      return f.match(
        [](negation)    { return "negation"s;    },
        [](unary)       { return "unary"s;       },
        [](conjunction) { return "conjunction"s; },
        [](binary)      { return "binary"s;      },
        [](otherwise)   { return "<unknown>"s;   }
      );
    };

    formula t = XF(p && GF(!q));

    REQUIRE(t.is<tomorrow>());
    REQUIRE(!t.is<negation>());

    REQUIRE(match(!p)          == "negation"s);
    REQUIRE(match(tomorrow(p)) == "unary"s);
    REQUIRE(match(p && q)      == "conjunction"s);
    REQUIRE(match(p || q)      == "binary"s);
  }

  SECTION("Unary formula pattern matching")
  {
    auto match = [&](unary u) {
      return u.match(
        [](negation)     { return "negation"s;    },
        [](tomorrow)     { return "tomorrow"s;    },
        [](yesterday)    { return "yesterday"s;    },
        [](always)       { return "always"s;    },
        [](eventually)   { return "eventually"s;    },
        [](past)         { return "past"s;    },
        [](historically) { return "historically"s;    }
      );
    };

    formula t = XF(p && GF(!q));

    REQUIRE(t.is<tomorrow>());
    REQUIRE(!t.is<negation>());

    REQUIRE(match(!p)        == "negation"s);
    REQUIRE(match(X(p))      == "tomorrow"s);
    REQUIRE(match(F(p && q)) == "eventually"s);
    REQUIRE(match(Y(p || q)) == "yesterday"s);
  }
}


TEST_CASE("Boolean constants simplification")
{
  alphabet sigma;

  atom p = sigma.var("p");

  REQUIRE(simplify_deep(!sigma.top()) == sigma.bottom());
  REQUIRE(simplify_deep(!sigma.bottom()) == sigma.top());
  REQUIRE(simplify_deep(!!sigma.top()) == sigma.top());
  REQUIRE(simplify_deep(!!sigma.bottom()) == sigma.bottom());

  REQUIRE(simplify_deep(sigma.top() && p) == p);
  REQUIRE(simplify_deep(sigma.bottom() && p) == sigma.bottom());

  REQUIRE(simplify_deep(sigma.top() || p) == sigma.top());
  REQUIRE(simplify_deep(sigma.bottom() || p) == p);

  REQUIRE(simplify_deep(then(sigma.top(), p)) == p);
  REQUIRE(simplify_deep(then(sigma.bottom(), p)) == sigma.top());
  REQUIRE(simplify_deep(then(p, sigma.top())) == sigma.top());
  REQUIRE(simplify_deep(then(p, sigma.bottom())) == sigma.bottom());
  
  REQUIRE(simplify_deep(iff(p, sigma.top())) == p);
  REQUIRE(simplify_deep(iff(p, sigma.bottom())) == !p);
}
