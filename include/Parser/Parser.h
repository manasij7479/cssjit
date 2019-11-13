#ifndef PARSER_H
#define PARSER_H

#include "ParserUtils.h"
namespace mm {
SyntaxTree ParseHTMLText(Stream &in) {
  StreamRAII s(in);
  std::string tok = in.loop([](char c) {
    return c != '<' && c != '>'; // Anything else here?
  });
  if (tok != "") {
    s.invalidate();
    return SyntaxTree{"text", {{"val", tok}}, {}};
  } else {
    return Error("Expected Text", in.getIndex());
  }
}

SyntaxTree ParseHTMLIdentifier(Stream& in) {
  StreamRAII s(in);
  std::string tok = in.loop([](char in) {
    return (in >= 'a' && in <= 'z') ||
           (in >= 'A' && in <= 'Z') ||
           (in >= '0' && in <= '9') ||
           (in == '_' || in == '-' || in == ':');
  });
  if (tok != "") {
    s.invalidate();
    return SyntaxTree{"id", {{"val", tok}}, {}};
  } else {
    return Error("Expected Identifier", in.getIndex());
  }
}

SyntaxTree ParseCSSIdentifier(Stream& in) {
  StreamRAII s(in);
  std::string tok = in.loop([](char in) {
    return (in >= 'a' && in <= 'z') ||
           (in >= 'A' && in <= 'Z') ||
           (in >= '0' && in <= '9') ||
           (in == '_' || in == '-');
  });
  if (tok != "") {
    s.invalidate();
    return SyntaxTree{"id", {{"val", tok}}, {}};
  } else {
    return Error("Expected Identifier", in.getIndex());
  }
}

//Make sure tags nest properly
//Remove closing tag from AST
mm::Action TagNest(mm::Action A) {
  return [A](mm::Stream &in) {
    StreamRAII s(in);
    auto t = A(in);
    if (t) {
      if (t.Children[0].Children[0].Attributes["val"]
            ==t.Children[t.Children.size() - 1].Children[0].Attributes["val"]) {
          s.invalidate();
          auto ret = t;
          ret.Children.pop_back();
          return ret;
        } else {
          return mm::Error("");
        }
    } else {
      return t;
    }
  };
}

SyntaxTree HTML(Stream &in) {
  auto Id = ParseHTMLIdentifier;
  auto Text = ParseHTMLText;
  auto Str = ParseStringLiteral;
  auto Attribs = Star("attribs", Seq("attrib", {Id, Opt("value", R2(Seq("_", {S("="), R1(Choice("value", {Str, Id}))})))}));
  auto TagId = Seq("tagid", {Id, Attribs});
  auto SelfTag = A(Seq("tag-self", {TagId, S("/")}));
  auto SelfTagNotClosed = A("tag-self-nc", TagId);
  auto OtherTags = A(Seq("tag-other", {SS("pre", {"?", "!"}), Text}));
  auto MayNotNestTag = Seq("tag", {A(TagId), HTML, A(PFX("/", Id))});
  auto GoodTag = TagNest(MayNotNestTag);
  auto ForgotToCloseTag = Seq("tag-f2c", {A(TagId), HTML});

  return Star("elements", Choice("element",
    {SelfTag, GoodTag, Text, SelfTagNotClosed, OtherTags, ForgotToCloseTag, MayNotNestTag}))(in);
}

SyntaxTree CSS(Stream &in) {
  auto Id = ParseCSSIdentifier;
  auto Property = RM2(RML(Seq("prop", {Id, S(":"),
    Choice("value", {Id, ParseNumber, ParseStringLiteral}), S(";")})));
  auto Entry = Seq("entry", {Id, B(Star("props", Property))});
  return Star("entries", Entry)(in);
}

}

#endif
