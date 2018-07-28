#ifndef PAW_PRINT_NODE
#define PAW_PRINT_NODE

#include <memory>
#include <unordered_map>
#include <vector>

#include "./defines.h"
#include "./token.h"


namespace paw_print {

using std::make_shared;
using std::shared_ptr;
using std::unordered_map;
using std::vector;

class Node;
class Nonterminal;


class TerminalBase {
public:
    inline static shared_ptr<Nonterminal> start () {
        _init();
        return start_;
    };
    

    string name;

    TerminalBase (const string &name);
    virtual ~TerminalBase () {}

    virtual bool isTerminal () = 0;

private:
    static shared_ptr<Nonterminal> start_;
    static void _init ();

};

class RuleElem {
public:
    enum IndentType {
        ANY,
        SAME,
        BIGGER,
    };

    shared_ptr<TerminalBase> termnon;
    IndentType indent_type;

    RuleElem (const shared_ptr<TerminalBase> &termnon, IndentType indent_type)
    :termnon(termnon), indent_type(indent_type) {
    }
};

class Terminal : public TerminalBase {
public:
    Token::Type type;

    Terminal (const string &name, Token::Type type);

    bool isTerminal () override { return true; }
};

class Nonterminal : public TerminalBase {
public:
    vector<vector<RuleElem>> rules;

    Nonterminal (const string &name);

    bool isTerminal () override { return false; }
};

class Node {
public:
    static shared_ptr<Node> parse (const char *text, const vector<Token> &tokens);

    PAW_GETTER(shared_ptr<TerminalBase>&, termnon)
    PAW_GETTER(int, rule_idx)
    PAW_GETTER(int, token_idx)
    PAW_GETTER(vector<shared_ptr<Node>>&, children)

    Node (const shared_ptr<TerminalBase>& termnon, int rule_idx, int token_idx);

    inline void setChild (int idx, const shared_ptr<Node> &child) {
        children_[idx] = child;
    }

    void setRuleAndPrepareChildren (int rule_idx);

private:
    shared_ptr<TerminalBase> termnon_;
    int rule_idx_;
    int token_idx_;
    vector<shared_ptr<Node>> children_;
};

}

#include "./undefines.h"
#endif
