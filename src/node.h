#ifndef PAW_PRINT_NODE
#define PAW_PRINT_NODE

#include <memory>
#include <unordered_map>
#include <vector>

#include "./defines.h"
#include "./token.h"


namespace parse_table {

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
		SMALLER,
        BIGGER,
    };

    shared_ptr<TerminalBase> termnon;
    IndentType indent_type;

    RuleElem (const shared_ptr<TerminalBase> &termnon, IndentType indent_type)
    :termnon(termnon), indent_type(indent_type) {
    }
};

class Rule {
public:
	shared_ptr<Nonterminal> left_side;
	vector<RuleElem> right_side;

	Rule (const shared_ptr<Nonterminal> &left_side, const vector<RuleElem> &right_side);
};

class Terminal : public TerminalBase {
public:
    Token::Type type;

    Terminal (const string &name, Token::Type type);

    bool isTerminal () override { return true; }
};

class Nonterminal : public TerminalBase {
public:
    vector<Rule> rules;

	inline unsigned int no () { return no_; }


    Nonterminal (const string &name);

    bool isTerminal () override { return false; }


private:
	static unsigned int next_no_;
	
	unsigned int no_;
};


class Node {
public:
    static shared_ptr<Node> parse (const char *text, const vector<Token> &tokens);

    PAW_GETTER(Node*, parent)
    PAW_GETTER(const shared_ptr<TerminalBase>&, termnon)
    PAW_GETTER(RuleElem::IndentType, indent_type)
    PAW_GETTER_SETTER(const int, rule_idx)
    PAW_GETTER_SETTER(const int, token_idx)
    PAW_GETTER(const vector<shared_ptr<Node>>&, children)

    Node (const shared_ptr<TerminalBase>& termnon, RuleElem::IndentType indent_type);

    inline void setChild (int idx, const shared_ptr<Node> &child) {
        children_[idx] = child;
        child->parent_ = this;
    }

    void setRuleAndPrepareChildren (int rule_idx);
    Node* findPrePriorityNode ();
    Node* findNextLeafNode ();
	Node* findPreBrother ();

private:
    Node *parent_;
    shared_ptr<TerminalBase> termnon_;
    RuleElem::IndentType indent_type_;
    int rule_idx_;
    int token_idx_;
    vector<shared_ptr<Node>> children_;

    int _findChild (Node *child);
};

}

#include "./undefines.h"
#endif
