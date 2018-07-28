#ifndef PAW_PRINT_NODE
#define PAW_PRINT_NODE

#include <memory>
#include <unordered_map>
#include <vector>

#include "./token.h"


namespace paw_print {

using std::make_shared;
using std::shared_ptr;
using std::unordered_map;
using std::vector;

class Nonterminal;


class Node {
public:
    static shared_ptr<Nonterminal> start;

    static shared_ptr<Node> parse (const char *text, const vector<Token> &tokens);
    

    string name;

    Node (const string &name);
    virtual ~Node () {}

    virtual bool isTerminal () = 0;

private:
    static void _init ();
};

class RuleElem {
public:
    enum IndentType {
        ANY,
        SAME,
        BIGGER,
    };

    shared_ptr<Node> node;
    IndentType indent_type;

    RuleElem (const shared_ptr<Node> &node, IndentType indent_type)
    :node(node), indent_type(indent_type) {
    }
};

class Terminal : public Node {
public:
    Token::Type type;

    Terminal (const string &name, Token::Type type);

    bool isTerminal () override { return true; }
};

class Nonterminal : public Node {
public:
    vector<vector<RuleElem>> rules;

    Nonterminal (const string &name);

    bool isTerminal () override { return false; }
};

}

#endif
