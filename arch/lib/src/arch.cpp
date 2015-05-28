/*
 * Copyright 2015 Nicolas Pope
 */

#include "dharc/arch.hpp"

#include <getopt.h>

#include <vector>
#include <list>
#include <iostream>
#include <string>

#include "dharc/node.hpp"

using dharc::Node;
using dharc::rpc::Command;
using dharc::rpc::send;
using std::string;
using std::cout;

static struct {
	string host;
	string port;
} configs {
	"localhost",
	"7878"
};

static option opts[] = {
	{"port", 1, nullptr, 'p'},
	{"host", 1, nullptr, 'h'},
	{nullptr, 0, nullptr, 0}
};

void dharc::start(int argc, char *argv[]) {
	int o;

	opterr = 0;
	while((o = getopt_long(argc, argv, "h:p:", opts, nullptr)) != -1) {
		switch(o) {
		case 'h':	configs.host = optarg; break;
		case 'p':	configs.port = optarg; break;
		case ':':	cout << "Option '" << optopt;
					cout << "' requires an argument\n";
					break;
		default:	break;
		}
	}
	opterr = 1;
	optind = 1;

	string uri = "tcp://";
	uri += configs.host;
	uri += ':';
	uri += configs.port;
	dharc::rpc::connect(uri.c_str());

	// Do a version check!
	if (send<Command::version>() != static_cast<int>(Command::end)) {
		cout << "!!! dharcd uses different version of rpc protocol !!!";
		cout << std::endl;
	}
}

void dharc::stop() {
}

Node dharc::unique() {
	return send<Command::unique>();
}

Node dharc::query(const Node &a, const Node &b) {
	return send<Command::query>(a, b);
}

void dharc::define(const Node &a, const Node &b, const Node &h) {
	send<Command::define_const>(a, b, h);
}

void dharc::define(const Node &a,
					const Node &b,
					const vector<vector<Node>> &p) {
	send<Command::define>(a, b, p);
}

list<Node> dharc::partners(const Node &n) {
	return send<Command::partners>(n);
}

/*Node Node::operator[](const Node &n) {
	return query(*this, n);
}*/

