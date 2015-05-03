#include "fdsb/fabric.hpp"
#include "fdsb/nid.hpp"
#include "fdsb/harc.hpp"

#include <map>

using namespace fdsb;

std::map<unsigned long long,Harc*> fabric;

Harc &fdsb::get(const Nid &a, const Nid &b)
{
	return *fabric[Nid::dual_hash(a,b)];
}

void fdsb::add(Harc &h)
{
	fabric[Nid::dual_hash(h.tail(0),h.tail(1))] = &h;
}

void fdsb::add(const Nid &n1, const Nid &n2)
{
	fdsb::add(new Harc(n1,n2));
}

Harc &Nid::operator[](const Nid &n)
{
	return fdsb::get(*this,n);
}

Harc &Harc::operator[](const Nid &n)
{
	return fdsb::get(query(),n);
}
