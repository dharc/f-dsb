/*
 * Copyright 2015 Nicolas Pope
 */

#ifndef FDSB_HARC_H_
#define FDSB_HARC_H_

#include <list>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <utility>
#include <chrono>
#include <ostream>

#include "fdsb/nid.hpp"
#include "fdsb/definition.hpp"

using std::pair;
using std::list;
using std::vector;
using std::chrono::time_point;
using std::chrono::system_clock;

namespace fdsb {

class Fabric;

/**
 * Hyper-arc class to represent relations between 3 nodes.
 * Each Harc can be given a definition and can be made dependant upon other
 * Harc's so that they update when others are changed.
 */
class Harc {
	friend class Fabric;

	public:
	enum struct Flag : unsigned int {
		none = 0x00,
		log = 0x01,			/** Record changes to this Harc */
		meta = 0x02,		/** This Harc has meta-data */
		defined = 0x04,		/** This Harc has a non-constant definition */
	};

	/**
	 * Get the head of this hyper-arc. Evaluate the definition if it is
	 * out-of-date. Also causes activation to change and a possible update of
	 * all partner Harcs, possibly involving a resort of partners.
	 * @return Head node of Harc
	 */
	const Nid &query();

	/**
	 * Get the pair of tail nodes that uniquely identify this Harc.
	 * @return Tail node pair.
	 */
	inline const pair<Nid, Nid> &tail() const;

	/**
	 * Does the tail of this Harc contain the given node?
	 * @param n The node id to check for.
	 * @return True if the node is part of the tail.
	 */
	inline bool tail_has(const Nid &n) const;

	/**
	 * What is the other node in this Harcs tail?
	 * @param n The unwanted tail node.
	 * @return The other tail node, not n.
	 */
	inline const Nid &tail_other(const Nid &n) const;

	/**
	 * Define the Harc as having a fixed head node.
	 */
	void define(const Nid &);

	/**
	 * Define the Harc as having a normalised path definition to work out
	 * its head node.
	 */
	void define(const vector<vector<Nid>> &);

	void define(Definition *def);

	inline void set_flag(Flag f);
	inline bool check_flag(Flag f) const;
	inline void clear_flag(Flag f);

	bool is_out_of_date() const;

	inline const list<Harc*> &dependants() const;

	/**
	 * Each time this is called the significance is reduced before being
	 * returned. It is boosted by querying the Harc.
	 */
	float significance();

	/**
	 * Time in seconds since this Harc was last queried.
	 * @return Seconds since last query.
	 */
	float last_query();

	Harc &operator[](const Nid &);
	Harc &operator=(const Nid &);
	bool operator==(const Nid &);

	private:
	pair<Nid, Nid> m_tail;				/* Pair of tail nodes, unique */
	union {
	Nid m_head;							/* Constant head node or ... */
	Definition *m_def;					/* Definition if defined flag is set */
	};
	Flag m_flags;						/* Flags for type of Harc etc */
	unsigned long long m_lastquery;		/* Ticks since last query */
	float m_strength;					/* Strength of the Harc relation */
	list<Harc*> m_dependants;  	/* Who depends upon me */

	// Might be moved to meta structure
	list<Harc*>::iterator m_partix[2];

	Harc() {}  							/* Only Fabric should call */
	explicit Harc(const pair<Nid, Nid> &t);

	void dirty();  						/* Mark as out-of-date and propagate */
	void add_dependant(Harc &);  		/* Notify given Harc on change. */
	void update_partners(const Nid &n, list<Harc*>::iterator &it);
	void reposition_harc(const list<Harc*> &p, list<Harc*>::iterator &it);
};

/* ==== Relational Operators ================================================ */

constexpr Harc::Flag operator | (Harc::Flag lhs, Harc::Flag rhs) {
	return (Harc::Flag)(static_cast<unsigned int>(lhs)
			| static_cast<unsigned int>(rhs));
}

inline Harc::Flag &operator |= (Harc::Flag &lhs, Harc::Flag rhs) {
	lhs = lhs | rhs;
	return lhs;
}

constexpr Harc::Flag operator & (Harc::Flag lhs, Harc::Flag rhs) {
	return (Harc::Flag)(static_cast<unsigned int>(lhs)
			& static_cast<unsigned int>(rhs));
}

inline Harc::Flag &operator &= (Harc::Flag &lhs, Harc::Flag rhs) {
	lhs = lhs & rhs;
	return lhs;
}

constexpr Harc::Flag operator ~(Harc::Flag f) {
	return (Harc::Flag)(~static_cast<unsigned int>(f));
}


/* ==== Inline Implementations ============================================== */

inline void Harc::set_flag(Flag f) { m_flags |= f; }
inline bool Harc::check_flag(Flag f) const { return (m_flags & f) == f; }
inline void Harc::clear_flag(Flag f) { m_flags &= ~f; }

std::ostream &operator<<(std::ostream &os, Harc &h);

inline bool Harc::is_out_of_date() const {
	if (check_flag(Flag::defined)) {
		return m_def->is_out_of_date();
	} else {
		return false;
	}
}

inline const pair<Nid, Nid> &Harc::tail() const { return m_tail; }

inline bool Harc::tail_has(const Nid &n) const {
	return (m_tail.first == n) || (m_tail.second == n);
}

inline const Nid &Harc::tail_other(const Nid &n) const {
	return (m_tail.first == n) ? m_tail.second : m_tail.first;
}

inline const list<Harc*> &Harc::dependants() const {
	return m_dependants;
}

};  // namespace fdsb

#endif /* FDSB_HARC_H_ */
