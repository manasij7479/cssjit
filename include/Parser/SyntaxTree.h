#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H
namespace mm {
struct SyntaxTree {
  std::string Node;
  std::map<std::string, std::string> Attributes;
  std::vector<SyntaxTree> Children;

  bool partial = false;

  explicit operator bool() const {
    return Node != "";
  }

  std::string error() {
    auto e = Attributes.find("error");
    if (e != Attributes.end()) {
      return e->second;
    } else {
      return "Syntax Error :(";
    }
  }

  void promoteSingleChild() {
    assert(Children.size() == 1);

    auto Child = Children[0];

    for (auto x : Child.Attributes) {
      Attributes.insert(x);
    }

    std::swap(Children, Child.Children);
  }

  void promoteSecondChild() {
    assert(Children.size() == 2);

    auto Child = Children[1];

    for (auto x : Child.Attributes) {
      Attributes.insert(x);
    }
    Children.pop_back();
    for (auto C : Child.Children) {
      Children.push_back(C);
    }
  }

  SyntaxTree getSecondChild() {
    assert(Children.size() >= 2);
    return Children[1];
  }

  SyntaxTree getFirstChild() {
    assert(Children.size() >= 1);
    return Children[0];
  }

  void removeFirstChild() {
    assert(Children.size() >= 1);
    for (size_t i = 1; i < Children.size(); ++i) {
      Children[i - 1] = Children[i];
    }
    Children.pop_back();
  }

  void removeLastChild() {
    assert(Children.size() >= 1);
    Children.pop_back();
  }

  void removeSecondChild() {
    assert(Children.size() >= 2);
    for (size_t i = 2; i < Children.size(); ++i) {
      Children[i - 1] = Children[i];
    }
    Children.pop_back();
  }

  void removeThirdChild() {
    assert(Children.size() >= 3);
    for (size_t i = 3; i < Children.size(); ++i) {
      Children[i - 1] = Children[i];
    }
    Children.pop_back();
  }

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
    out << s << Node << "  ";
    for (auto x : Attributes) {
      if (x.first != "error" || Node == "") {
        out <<  "(" << x.first << " : ";
        marshall(out, x.second);
        out << ") ";
      }
    } out << "\n";

    for (size_t i = 0; i < Children.size(); ++i) {
      auto C = Children[i];
      if (Children.size() >= 2 && i != Children.size() - 1) mark.push_back(indent_);
      C.dump(out, indent_ + 1, mark);
      if (Children.size() >= 2 && i != Children.size() - 1) mark.pop_back();
    }
  }
};
}
#endif
