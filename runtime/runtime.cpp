#include <iostream>
#include <sstream>
#include <fstream>
#include <set>
#include <cstring>
#include <cmath>
#include "../include/Parser/Parser.h"
#include "../include/Common/Common.h"

extern"C" int getTop_c(const char *str);
extern"C" int getBottom_c(const char *str);
extern"C" int getLeft_c(const char *str);
extern"C" int getDelta_c(const char *str);

// extern"C" void match(char *str);
/*
extern "C" void match(char *str) {
  return;
}*/

struct Box {
  int x, y, w, h;
};

// abstract representation of HTML for this project
// really need to start using a language with gadts
struct AST {
  AST(std::string C, std::vector<AST> Ch) : cls(C), Children(Ch) {};
  AST(std::string t) : cls("text"), text(t) {};

  std::string cls; // make this a set eventually
  std::vector<AST> Children;
  std::string text; // only look if Children.empty()

  Box box;


  std::string indent(size_t indent, std::vector<size_t> mark) {
    std::string result = "";
    while (indent--) result += "  ";
    if (result.length() >= 2) {
      result[result.length() - 1] = '-';
      result[result.length() - 2] = '\\';
    }
    for (auto m : mark) {
      result[2*m] = '|';
    }
    return result;
  }

  void marshall(std::ostream &out, std::string in) {
    for (auto c : in) {
      if (c == '\n') out << "\\n";
      else if (c == '\t') out << "\\t";
      else out << c;
    }
  }

  void dump(std::ostream& out, size_t indent_ = 0, std::vector<size_t> mark = {}) {
    auto s = indent(indent_, mark);
    out << s;
    if (cls != "text") {
      out << "(div/class:";
      marshall(out, cls);
      out << ")  ";
    }
    if (Children.empty()) {
      out  << "(text of length:" << text.length() << ")  ";
    }
    out << " X:" << box.x << " Y:" << box.y << " W:" << box.w << " H:" << box.h  << "\n";
//     for (auto x : Attributes) {
//       if (x.first != "error" || Node == "") {
//         out <<  "(" << x.first << " : ";
//         marshall(out, x.second);
//         out << ") ";
//       }
//     } out << "\n";

    for (size_t i = 0; i < Children.size(); ++i) {
      auto C = Children[i];
      if (Children.size() >= 2 && i != Children.size() - 1) mark.push_back(indent_);
      C.dump(out, indent_ + 1, mark);
      if (Children.size() >= 2 && i != Children.size() - 1) mark.pop_back();
    }
  }

};

AST Abstract(mm::SyntaxTree &St) {
  if (St.Node == "elements" && St.Children.size() == 1) {
    return Abstract(St.Children[0]);
  }

  std::vector<AST> Children;
  if (St.Node == "element" && St.Children[0].Node == "tag"
      && St.Children[0].Children[0].Children[0].Attributes["val"] == "div") {

    auto cls = St.Children[0].Children[0].Children[1].Children[0].Children[1].Children[0].Attributes["val"];

    for (auto Child : St.Children[0].Children[1].Children) {
      Children.push_back(Abstract(Child));
    }
    return AST(cls, Children);
  } else if (St.Node == "element" && St.Children[0].Node == "text") {
    return AST(St.Children[0].Attributes["val"]);
  } else {
    return AST("UNSUPPORTED");
  }
}

int getTop(std::string cls, cssjit::CSSMap &Rules) {
  int Result = 3;
  if (Rules.find(cls) == Rules.end()) {
    return Result;
  }

  static const std::set<std::string> relevant = {"tm", "tb", "tp"};

  for (auto &&P : Rules[cls]) {
    if (relevant.find(P.first) != relevant.end()) {
      Result += std::stoi(P.second) - 1;
    }
  }
  return Result;
}

int getBottom(std::string cls, cssjit::CSSMap &Rules) {
  int Result = 3;
  if (Rules.find(cls) == Rules.end()) {
    return Result;
  }

  static const std::set<std::string> relevant = {"bm", "bb", "bp"};

  for (auto &&P : Rules[cls]) {
    if (relevant.find(P.first) != relevant.end()) {
      Result += std::stoi(P.second) - 1;
    }
  }
  return Result;
}

int getLeft(std::string cls, cssjit::CSSMap &Rules) {
  int Result = 3;
  if (Rules.find(cls) == Rules.end()) {
    return Result;
  }

  static const std::set<std::string> relevant = {"lm", "lb", "lp"};

  for (auto &&P : Rules[cls]) {
    if (relevant.find(P.first) != relevant.end()) {
      Result += std::stoi(P.second) - 1;
    }
  }
  return Result;
}

int getDelta(std::string cls1, std::string cls2, cssjit::CSSMap &Rules) {
  int bm1 = 1, tm2 = 1;

  if (Rules.find(cls1) != Rules.end()) {
    for (auto &&P : Rules[cls1]) {
      if (P.first == "bm1") {
        bm1 = std::stoi(P.second);
      }
    }
  }

  if (Rules.find(cls2) != Rules.end()) {
    for (auto &&P : Rules[cls2]) {
      if (P.first == "tm2") {
        bm1 = std::stoi(P.second);
      }
    }
  }

  return std::min(bm1, tm2);
}

std::pair<int, int> getContentCoords(std::string cls, cssjit::CSSMap &Rules) {
  return {getLeft(cls, Rules), getTop(cls, Rules)};
}

void SimpleLayout(AST &In, int x, int y, int width, cssjit::CSSMap &Rules) {

  static const int letterwidth = 1;
//   static const int lineheight = 1;
//   static const int linegap = 1;

  In.box.w = width;
  In.box.h = 0;
  In.box.x = x;
  In.box.y = y;

  if (In.Children.empty()) { // must be a text/inline node
    auto length = In.text.length() * letterwidth;
    In.box.h = std::ceil(length * 1.0 / width);
  } else { // div with a bunch of children
    In.box.h += getTop(In.cls, Rules);

    bool first = true;
    std::string lastcls;
    for (auto &&C : In.Children) {
      // TODO: fetch width from CSS
      auto CC = getContentCoords(C.cls, Rules);

      // Margin collapsing v0.1 alpha
      if (!first) {
        In.box.h -= getDelta(lastcls, C.cls, Rules);
      }

      SimpleLayout(C, x + CC.first, y + In.box.h + CC.second, width, Rules);
      In.box.h += (C.box.h);

      lastcls = C.cls;
      first = false;

    }
    In.box.h += getBottom(In.cls, Rules);
  }
}

void CompiledLayout(AST &In, int x, int y, int width) {

  static const int letterwidth = 1;
//   static const int lineheight = 1;
//   static const int linegap = 1;

  In.box.w = width;
  In.box.h = 0;
  In.box.x = x;
  In.box.y = y;

  if (In.Children.empty()) { // must be a text/inline node
    auto length = In.text.length() * letterwidth;
    In.box.h = std::ceil(length * 1.0 / width);

  } else { // div with a bunch of children
    In.box.h += getTop_c(In.cls.c_str());

    bool first = true;
    std::string lastcls;
    for (auto &&C : In.Children) {
      // TODO: fetch width from CSS
//       auto CC = getContentCoords(C.cls.c_str(), Rules);

      // Margin collapsing v0.1 alpha
      if (!first) {
        In.box.h -= getDelta_c((lastcls + "." +  C.cls).c_str());
      }

      CompiledLayout(C, x + getLeft_c(C.cls.c_str()), y + In.box.h + getTop_c(C.cls.c_str()), width);
      In.box.h += (C.box.h);

      lastcls = C.cls;
      first = false;

    }
    In.box.h += getBottom_c(In.cls.c_str());
  }
}

int main(int argc, char **argv) {
//   std::ifstream ins(argv[1]);
//   std::ostringstream foo;
//   foo << ins.rdbuf();
//
//   std::string str = foo.str();
//
//   mm::Stream in(str.c_str(), 0, str.length() - 1);

//   auto St = mm::HTML(in);
//   St.dump(std::cerr);


//   Bar.dump(std::cout);
//   traverse(St);
#include "AST.in"

//   AST Foo("b", {
//     AST("text0"),
//     AST("f", {AST("aaaa"), AST("g", {AST{"bbbb"}})}),
//     AST("g", {AST{"aaaaaaaa"}})
//   });

  if (argc > 1) {
    if (std::string(argv[1]) == "compiled") {
      CompiledLayout(Foo, 0, 0, 10);
      Foo.dump(std::cout);
    } else {
      std::ifstream ins(argv[1]);
      std::ostringstream foo;
      foo << ins.rdbuf();

      std::string s = foo.str();

      mm::Stream cssin(s.c_str(), 0, s.length() - 1);

      auto St = mm::CSS(cssin);


      cssjit::CSSMap Rules;

      for (auto &&C : St.Children) {
        auto Name = C.getFirstChild().Attributes["val"];
        for (auto &&Props : C.getSecondChild().Children) {
          auto Key = Props.getFirstChild().Attributes["val"];
          auto Value = Props.getSecondChild().getFirstChild().Attributes["val"];
          Rules[Name].push_back({Key, Value});
        }
      }
      SimpleLayout(Foo, 0, 0, 10, Rules);
//       Foo.dump(std::cout);
    }
  }

  return 0;
}

extern"C" int64_t input() {
  int64_t n;
  std::cin >> n;
  return n;
}

extern"C" void printint(int64_t x) {
  std::cout << x;
}

extern"C" int32_t eq(char *x, char *y) {
  return strcmp(x, y) == 0;
}

extern "C" void printrule(char *key, char *value) {
  std::cout << key << ':' << value << ' ';
}

extern "C" int64_t hash(char *key) {
  std::string str(key);
  return std::hash<std::string>()(str);
}

// // maybe link libc atoi directly
// extern "C" int32_t my_atoi(char *value) {
//   std::string str(value);
//   return std::stoi(str);
// }


