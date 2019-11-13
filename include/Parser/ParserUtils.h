#ifndef MM_PARSER_UTILS
#define MM_PARSER_UTILS
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <iostream>
#include <ostream>
#include <cassert>
#include <deque>
#include "SyntaxTree.h"

namespace mm {

// Missing Features
// * Sensible error messages for Choice (sort of done)
// * Cache results when backtracking (see: packrat parsing)
// * Match Regex (Shouldn't be too difficult)
// * Policy based whitespace skipping (why?)
// * Debug log

struct Stream {
  Stream(const char* p, int i, int max) : ptr(p), index(i), bounds(max) {}
  const char *ptr;
  int index;
  int bounds;

  char get(bool ignoreWhiteSpace = true) {
    if (index >= bounds)
      return '\0';
    if (std::isspace(ptr[index]) && ignoreWhiteSpace) {
      index++;
      return get(ignoreWhiteSpace);
    }
    return ptr[index++];
  }

  std::string loop(std::function<bool(char)> Pred) {
    skipWhiteSpace();
    std::string Result;
    while (Pred(ptr[index]) && !eof()) {
      Result += ptr[index];
      index++;
    }
    return Result;
  }

  bool fixed(std::string Str) {
    skipWhiteSpace();
    bool failed = false;
    for (unsigned int i = 0; i < Str.length(); ++i) {
      if (Str[i] != ptr[index+i])
        failed = true;
    }
    if (!failed) {
      index += Str.length();
      return true;
    }
    else return false;
  }

  bool eof() {
    return index >= bounds - 1;
  }

  void skipWhiteSpace() {
    while (index < bounds && (ptr[index] == ' ' || ptr[index] == '\t' || ptr[index] == '\n'))
      index++;
  }

  int getIndex() const {
    return index;
  }

};

class StreamRAII {
public:
  StreamRAII(Stream& in) : in_(in) {
    SavedIndex = in.index;
  }
  ~StreamRAII(){
    if (ShouldRevert) {
      in_.index = SavedIndex;
    }
  }
  void invalidate() {
    ShouldRevert = false;
  }
private:
  bool ShouldRevert = true;
  int SavedIndex;
  Stream& in_;
};

typedef std::function<SyntaxTree(Stream&)> Action;

SyntaxTree Error(std::string msg, int index = -1) {
  SyntaxTree t = {};
  t.Attributes["error"] = msg;
  if (index != -1) {
    t.Attributes["loc"] = std::to_string(index);
  }
  return t;
}

Action Choice(std::string name, std::vector<Action> Actions) {
  return [name, Actions](Stream& in) {
    std::string errors;
    for (auto a : Actions) {
      StreamRAII s(in);
      auto t = a(in);
      if (t) {
        s.invalidate();
        return SyntaxTree{name, {}, {t}};
      } else {
        if (t.partial) {
          errors +=  t.error() + " or  \n";
        }
      }
    }
    return Error(errors + " in " + name, in.index);
    // TODO: Better error message
  };
}

Action Seq(std::string name, std::deque<Action> Actions) {
  return [name, Actions](Stream& in) {
    StreamRAII s(in);
    SyntaxTree result{name, {}, {}};

    for (auto a : Actions) {
      auto t = a(in);
      if (!t) {
        // std::cout << t.error () << in.index << std::endl;
        std::string extra;
        if (!result.Children.empty()) {
          auto t = result.Children.back();
          if (t.Attributes.find("error") != t.Attributes.end()) {
            extra += " or \n" + t.error();
          }
        }
        auto e = Error(t.error() + " in " + name + extra, in.index);
        if (!result.Children.empty()) {
          e.partial = true;
        }
        return e;
      }
      result.Children.push_back(t);
    }
    s.invalidate();
    return result;
  };
};

Action Star(std::string name, Action A) {
  return [name, A](Stream& in) {
    SyntaxTree result{name, {}, {}};
    while (true) {
      StreamRAII s(in);
      auto t = A(in);
      if (!t) {
        result.Attributes["error"] = t.error();
        break;
      } else {
        s.invalidate();
        result.Children.push_back(t);
      }
    }
    return result;
  };
}

// SyntaxTree ParseNewLine(Stream& in) {
//   StreamRAII s(in);
//   if (in.fixed("\n")) {
//     s.invalidate();
//     return SyntaxTree{"newline", {}, {}};
//   } else {
//     return Error("Expected newline", in.getIndex());
//   }
// }


Action Pr1(Action A) {
  return [A] (Stream& in) {
    auto t = A(in);
    if (t) t.promoteSingleChild();
    return t;
  };
}

Action Pr2(Action A) {
  return [A] (Stream& in) {
    auto t = A(in);
    if (t) t.promoteSecondChild();
    return t;
  };
}

Action R1(Action A) {
  return [A] (Stream& in) {
    auto t = A(in);
    if (t) t = t.getFirstChild();
    return t;
  };
}

Action R2(Action A) {
  return [A] (Stream& in) {
    auto t = A(in);
    if (t) t = t.getSecondChild();
    return t;
  };
}

Action RM1(Action A) {
 return [A] (Stream& in) {
    auto t = A(in);
    if (t) t.removeFirstChild();
    return t;
  };
}

Action RML(Action A) {
 return [A] (Stream& in) {
    auto t = A(in);
    if (t) t.removeLastChild();
    return t;
  };
}

Action RM2(Action A) {
 return [A] (Stream& in) {
    auto t = A(in);
    if (t) t.removeSecondChild();
    return t;
  };
}

Action RM3(Action A) {
 return [A] (Stream& in) {
    auto t = A(in);
    if (t) t.removeThirdChild();
    return t;
  };
}

Action Plus(std::string name, Action A) {
  auto TreeResult = Seq(name, {A, Star("nest" , A)});
  return Pr2(TreeResult);
}

Action Opt(std::string name, Action A) {
  return [name, A] (Stream& in) {
    SyntaxTree result {name, {}, {}};
    StreamRAII s(in);
    auto t = A(in);
    if (t) {
      s.invalidate();
      result.Children.push_back(t);
    }
    return result;
  };
}

Action ROpt(std::string name, Action A) {
  return R1(Opt(name, A));
}

Action Empty(std::string name) {
  return [name] (Stream&) {
    SyntaxTree t{name, {}, {}};
    return t;
  };
}

// Parse Exact String
Action S(std::string name) {
  return [name] (Stream& in) {
    StreamRAII s(in);
    auto str = in.fixed(name);
    if (str) {
      s.invalidate();
      return SyntaxTree{name, {}, {}};
    } else {
      return Error("Expected " + name, in.getIndex());
    }
  };
}

Action SS(std::string name, std::vector<std::string> choices) {
  std::vector<Action> actions;
  for (auto x : choices) {
    actions.push_back(S(x));
  }
  return Choice(name, actions);
}

// Comma Separated Non-empty List
Action CSL(std::string name, Action A) {
  return Pr2(Seq(name, {A,
            Star("nest1",
            R2(Seq("nest2", {S(","), A})) )}));
}

// Parenthesize (A)
Action P(Action A) {
  return R2(Seq("nest1", {S("("), A , S(")")}));
}

// Parenthesized Comma Separated Non-Empty List
Action PCSL(std::string name, Action A) {
  return P(CSL(name, A));
}

Action PCSLE(std::string name, Action A) {
  return R1(Choice(".", {PCSL(name, A), P(Empty(name))}));
}

// Brace for impact {A}
Action B(Action A) {
  return R2(Seq("nest1", {S("{"), A , S("}")}));
}

// [A]
Action T(Action A) {
  return R2(Seq("nest1", {S("["), A , S("]")}));
}

// <A>
Action A(Action A) {
  return R2(Seq("nest1", {S("<"), A , S(">")}));
}

// <A>
Action A(std::string name, Action A) {
  return R2(Seq(name, {S("<"), A , S(">")}));
}

// A\n
Action N(Action A) {
  return (R1(Seq(".", {A, S("\n")})));
}

Action PSeq(std::string prefix, std::deque<Action> Actions) {
  Actions.push_front(S(prefix));
  return RM1(Seq(prefix, Actions));
}

Action PFX(std::string prefix, Action A, std::string = "") {
  std::deque<Action> Actions = {A};
  Actions.push_front(S(prefix));
  return RM1(Seq(prefix, Actions));
}

Action SFX(Action A, std::string suffix, std::string = "") {
  std::deque<Action> Actions = {A};
  Actions.push_back(S(suffix));
  return RML(Seq(suffix, Actions));
}

SyntaxTree ParseIdentifier(Stream& in) {
  bool firstChar = true;
  StreamRAII s(in);
  std::string tok = in.loop([&firstChar](char in) {
    if (firstChar) {
      firstChar = false;
      return (in >= 'a' && in <= 'z') ||
             (in >= 'A' && in <= 'Z') ||
             (in == '_' || in == '-');
    } else {
      return (in >= 'a' && in <= 'z') ||
             (in >= 'A' && in <= 'Z') ||
             (in >= '0' && in <= '9') ||
             (in == '_' && in == '-');
    }
  });
  if (tok != "") {
    s.invalidate();
    return SyntaxTree{"id", {{"val", tok}}, {}};
  } else {
    return Error("Expected Identifier", in.getIndex());
  }
}

SyntaxTree ParseNumber(Stream& in) {
  bool firstChar = true;
  StreamRAII s(in);
  std::string tok = in.loop([&firstChar](char in) {
    if (firstChar) {
      firstChar = false;
      return (in >= '0' && in <= '9') ||
             (in == '-');
    } else {
      return (in >= '0' && in <= '9');
    }
  });
  if (tok == "-") {
    return Error("- is not a number", in.getIndex());
  }
  else if (tok != "") {
    s.invalidate();
    return SyntaxTree{"num", {{"val", tok}}, {}};
  } else {
    return Error("Expected Number", in.getIndex());
  }
}

SyntaxTree ParseStringLiteral(Stream& in) {
  int state = 0;
  // 0 = on start,
  // 1 = on opening "
  // 2 = on closing "
  StreamRAII s(in);
  std::string result = in.loop([&state](char in) {
    if (state == 0) {
      state = 1;

      return in == '\"' || in == '\'';
    } else if (state == 1) {
      if (in == '\"' || in == '\'') {
        state = 2;
      }
      return true;
    } else {
      return false;
    }
  });

  if (result != "" && (result[0] == '\"' || result[0] == '\'')
      && (result[result.length() - 1] == '\"'
        || result[result.length() - 1] == '\'')) {
    s.invalidate();
    return SyntaxTree{"str", {{"val", result}}, {}};
  } else {
    return Error("Expected String Literal", in.getIndex());
  }
}

}

#endif
