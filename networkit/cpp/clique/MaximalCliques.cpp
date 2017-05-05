#include "MaximalCliques.h"
#include "../centrality/CoreDecomposition.h"

#include <cassert>
#include <algorithm>

namespace {
	using NetworKit::node;
	using NetworKit::index;

	struct SwapFunctor {
		std::vector<node> &pxvector;
		std::vector<node> &pxlookup;

		SwapFunctor(std::vector<node> &pxvector, std::vector<index>& pxlookup) : pxvector(pxvector), pxlookup(pxlookup) {
		}

		void operator()(node u, index pos) {
			node pxvec2 = pxvector[pos];
			std::swap(pxvector[pxlookup[u]], pxvector[pos]);
			pxlookup[pxvec2] = pxlookup[u];
			pxlookup[u] = pos;
		}
	};
}

namespace NetworKit {

MaximalCliques::MaximalCliques(const Graph& G) : G(G) {
}


const std::vector<std::vector<node>>& MaximalCliques::getCliques() const {
	assureFinished();
	return result;
}

void MaximalCliques::run() {
	hasRun = false;

	result.clear();

	CoreDecomposition cores(G, false, false, true);
	cores.run();

	const auto& orderedNodes = cores.getNodeOrder();

	std::vector<node> pxvector(G.numberOfNodes());
	std::vector<index> pxlookup(G.upperNodeIdBound());

	SwapFunctor swapNodeToPos(pxvector, pxlookup);

	index ii = 0;
	for (const node u : orderedNodes) {
		pxvector[ii] = u;
		pxlookup[u] = ii;
		ii += 1;
	}

	#ifndef NDEBUG
	for (auto u : orderedNodes) {
		assert(pxvector[pxlookup[u]] == u);
	}
	#endif

	// Store out-going neighbors in the direction of higher core numbers.
	// This means that the out-degree is bounded by the maximum core number.
	const StaticOutGraph outGraph(G, pxlookup);

	index xpbound = 1;
	for (const node& u : orderedNodes) {
		swapNodeToPos(u, xpbound-1);

		#ifndef NDEBUG
		for (auto v : orderedNodes) {
			if (v == u)
				break;

			assert(pxlookup[v] < xpbound);
		}
		#endif

		count xcount = 0;
		count pcount = 0;
		G.forNeighborsOf(u, [&] (node v) {

			#ifndef NDEBUG
			assert(pxlookup[v] >= 0);
			assert(pxlookup[v] < pxvector.size());

			assert(xcount <= xpbound);
			assert(pcount <= pxvector.size() - xpbound);
			#endif

			if (pxlookup[v] < xpbound) { // v is in X
				swapNodeToPos(v, xpbound - xcount - 1);
				xcount += 1;
			} else { // v is in P
				swapNodeToPos(v, xpbound + pcount);
				pcount += 1;
			}
		});

		#ifndef NDEBUG
		bool inRange = false;
		bool wasInRange = false;
		for (node v : pxvector) {
			if (G.hasEdge(u, v)) {
				assert(!wasInRange);
				inRange = true;
			} else {
				if (inRange) {
					wasInRange = true;
					inRange = false;
				}
			}
		}
		#endif

		std::vector<node> r = {u};
		tomita(outGraph, pxvector, pxlookup, xpbound - xcount, xpbound, xpbound + pcount, r);

		xpbound += 1;
	}

	hasRun = true;
}

void MaximalCliques::tomita(const StaticOutGraph& outGraph, std::vector<node>& pxvector, std::vector<index>& pxlookup, index xbound, index xpbound, index pbound, std::vector<node>& r) {
	if (xbound == pbound) { //if (X, P are empty)
		result.push_back(r);
		return;
	}

	if (xpbound == pbound) return;

	SwapFunctor swapNodeToPos(pxvector, pxlookup);

	#ifndef NDEBUG
	assert(xbound >= 0);
	assert(xbound <= xpbound);
	assert(xpbound <= pbound);
	assert(pbound <= pxvector.size());
	#endif

	node u = findPivot(outGraph, pxvector, pxlookup, xbound, xpbound, pbound);
	std::vector<node> movedNodes;

	// Find all nodes in P that are not neighbors of the pivot
	// this step is necessary as the next loop changes pxvector,
	// which prohibits iterating over it in the same loop.
	std::vector<node> toCheck;

	// Step 1: mark all outgoing neighbors of the pivot in P
	std::vector<bool> pivotNeighbors(pbound - xpbound);
	outGraph.forOutEdgesOf(u, [&](node v) {
		index vpos = pxlookup[v];
		if (vpos >= xpbound && vpos < pbound) {
			pivotNeighbors[vpos - xpbound] = true;
		}
	});

	// Step 2: for all not-yet marked notes check if they have the pivot as neighbor.
	// If not: they are definitely a non-neighbor.
	for (index i = xpbound; i < pbound; i++) {
		if (!pivotNeighbors[i - xpbound]) {
			node p = pxvector[i];

			if (!outGraph.hasNeighbor(p, u)) {
				toCheck.push_back(p);
			}
		}
	}

	for (auto pxveci : toCheck) {
		count xcount = 0, pcount = 0;

		// Group all neighbors of pxveci in P \cup X around xpbound.
		// Step 1: collect all outgoing neighbors of pxveci
		outGraph.forOutEdgesOf(pxveci, [&](node v) {
			if (pxlookup[v] < xpbound && pxlookup[v] >= xbound) { // v is in X
				swapNodeToPos(v, xpbound - xcount - 1);
				xcount += 1;
			} else if (pxlookup[v] >= xpbound && pxlookup[v] < pbound){ // v is in P
				swapNodeToPos(v, xpbound + pcount);
				pcount += 1;
			}
		});

		// Step 2: collect all nodes in X that have not yet been collected
		// and that have pxveci as outgoing neighbor.
		for (index i = xbound; i < xpbound;) {
			// stop if we have reached the collected neighbors
			if (i == xpbound - xcount) break;
			node x = pxvector[i];

			if (outGraph.hasNeighbor(x, pxveci)) {
				swapNodeToPos(x, xpbound - xcount - 1);
				xcount += 1;
			} else {
				// Advance only if we did not swap otherwise we have already
				// a next candidate at position i.
				++i;
			}
		}

		// Step 3: collect all nodes in P that have not yet been collected
		// and that have pxveci as outgoing neighbor.
		for (index i = xpbound + pcount; i < pbound; ++i) {
			node p = pxvector[i];

			if (outGraph.hasNeighbor(p, pxveci)) {
				swapNodeToPos(p, xpbound + pcount);
				pcount += 1;
			}
		}

		r.push_back(pxveci);

		#ifndef NDEBUG
		assert(xpbound + pcount <= pbound);
		assert(xpbound - xcount >= xbound);
		#endif

		tomita(outGraph, pxvector, pxlookup, xpbound - xcount, xpbound, xpbound + pcount, r);

		r.pop_back();

		swapNodeToPos(pxveci, xpbound);
		xpbound += 1;
		assert(pxvector[xpbound - 1] == pxveci);
		movedNodes.push_back(pxveci);
	}

	for (node v : movedNodes) {
		//move from X -> P
		swapNodeToPos(v, xpbound - 1);
		xpbound -= 1;
	}

	#ifndef NDEBUG
	for (node v : movedNodes) {
		assert(pxlookup[v] >= xpbound);
		assert(pxlookup[v] < pbound);
	}
	#endif
}

node MaximalCliques::findPivot(const StaticOutGraph& outGraph, std::vector<node>& pxvector, std::vector<index>& pxlookup, index xbound, index xpbound, index pbound) {
	// Counts for every node in X \cup P how many outgoing neighbors it has in P
	std::vector<count> pivotNeighbors(pbound - xbound);
	const count psize = pbound-xpbound;

	// Step 1: for all nodes in X count how many outgoing neighbors they have in P
	for (index i = 0; i < xpbound - xbound; i++) {
		node u = pxvector[i + xbound];
		outGraph.forOutEdgesOf(u, [&](node v) {
			if (pxlookup[v] >= xpbound && pxlookup[v] < pbound) {
				++pivotNeighbors[i];
			}
		});

		// If a node has |P| neighbors, we cannot find a better candidate
		if (pivotNeighbors[i] == psize) return u;
	}

	// Step 2: for all nodes in P
	// a) increase counts for every neighbor in P \cup X to account for incoming neighbors
	// b) count all outgoing neighbors in P
	for (index i = xpbound - xbound; i < pivotNeighbors.size(); ++i) {
		node u = pxvector[i + xbound];
		outGraph.forOutEdgesOf(u, [&](node v) {
			index neighborPos = pxlookup[v];
			if (neighborPos >= xbound && neighborPos < pbound) {
				++pivotNeighbors[neighborPos-xbound];

				if (neighborPos >= xpbound) {
					++pivotNeighbors[i];
				}
			}
		});
	}

	node maxnode = pxvector[xbound];
	count maxval = pivotNeighbors[0];

	// Step 3: find maximum
	for (index i = 1; i < pivotNeighbors.size(); ++i) {
		if (pivotNeighbors[i] > maxval) {
			maxval = pivotNeighbors[i];
			maxnode = pxvector[i + xbound];
		}
	}

	#ifndef NDEBUG
	assert(maxnode < G.upperNodeIdBound() + 1);
	#endif

	return maxnode;
}

}
