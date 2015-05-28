/*
 * Copyright 2015 Nicolas Pope
 */

#include "dharc/script.hpp"

#include <vector>
#include <iostream>
#include <string>

#include "dharc/node.hpp"
#include "dharc/parse.hpp"
#include "dharc/arch.hpp"

using dharc::Node;
using dharc::parser::Context;
using std::vector;
using std::string;
using std::cout;

using dharc::parser::word;
using dharc::parser::value;
using dharc::parser::noact;

dharc::arch::Script::Script(std::istream &is, const char *src)
: ctx_(is), source_(src), params_(nullptr) {
}

namespace {
struct command_query {
	Context &parse;

	void operator()() {
		Node t1;
		Node t2;

		if (!parse(value<Node>{t1}, value<Node>{t2}, noact)) {
			parse.syntaxError("'%query' requires two node ids");
			return;
		}
		if (!parse(';', noact)) {
			parse.warning("Expected ';'");
		}
		Node r = dharc::query(t1, t2);
		std::cout << "  " << r << std::endl;
	}
};

struct command_partners {
	Context &parse;

	void operator()() {
		Node t1;

		if (!parse(value<Node>{t1}, noact)) {
			parse.syntaxError("'%partners' needs a node id");
		}
		if (!parse(';', noact)) {
			parse.warning("Expected ';'");
		}
		auto part = dharc::partners(t1);
		for (auto i : part) {
			cout << "  - " << i << std::endl;
		}
	}
};
};  // namespace

bool dharc::arch::Script::parseNode(Node &val) {
	return (ctx_(value<Node>{val}, noact)
	// || ctx_(word{"<new>"}, [&]() { value = dharc::unique(); })
	|| ctx_('$', [&]() {
		int id = 0;
		if (!ctx_(value<int>{id}, noact)) {
			ctx_.syntaxError("'$' must be followed by a parameter number");
		}
		if (id >= static_cast<int>(params_->size())) {
			ctx_.runtimeError(string("parameter '$")
				+ std::to_string(id)
				+ "' not found");
			val = dharc::null_n;
		} else {
			val = (*params_)[id];
		}
	}));
}

void dharc::arch::Script::parseStatement(Node &cur) {
	Node n;

	if (ctx_(';', noact)) return;
	if (ctx_(word{"<typeint>"}, noact)) {
		cur = Node(static_cast<int>(cur.t));
		parseStatement(cur);
	} else if (ctx_(word{"<int>"}, noact)) {
		cur = Node(static_cast<int>(cur.i));
		parseStatement(cur);
	} else if (parseNode(n)) {
		if (ctx_('=', noact)) {
			Node r;

			if (parseNode(r)) {
				dharc::define(cur, n, r);
				parseStatement(cur);
			} else {
				ctx_.syntaxError("'=' must be followed by a node id");
			}
		} else {
			if ((cur == dharc::null_n) || (n == dharc::null_n)) {
				ctx_.information("querying a 'null' node");
			}
			cur = dharc::query(cur, n);
			parseStatement(cur);
		}
	} else {
		if (ctx_.eof()) {
			ctx_.warning("expected a node id or ';'");
		} else {
			ctx_.syntaxError("expected a node id");
		}
	}
}

Node dharc::arch::Script::operator()(const vector<Node> &p) {
	Node cur = null_n;
	params_ = &p;

	while (!ctx_.eof()) {
		if (ctx_('%', noact)) {
			if (!(
				ctx_(word{"query"}, command_query{ctx_})
				|| ctx_(word{"partners"}, command_partners{ctx_})))
			{
				ctx_.syntaxError("Unrecognised command");
			}
		} else if (parseNode(cur)) {
			parseStatement(cur);
		} else if (ctx_(word{"<typeint>"}, noact)) {
			ctx_.syntaxError("node cast '<typeint>' must follow node");
		} else if (ctx_(';', noact)) {
			// Nothing
		} else {
			ctx_.syntaxError("Invalid statement");
		}

		if (!ctx_) {
			ctx_.skipLine();
		}
		ctx_.printMessages(source_);
	}
	return cur;
}

