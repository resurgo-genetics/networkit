/*
 * Graph.h
 *
 *  Created on: 04.02.2013
 *      Author: Christian Staudt (christian.staudt@kit.edu), Henning Meyerhenke (henning.meyerhenke@kit.edu)
 */

#ifndef GRAPH_H_
#define GRAPH_H_

#include <functional>
#include <cassert>
#include <vector>
#include <cinttypes>
#include <string>
#include <queue>
#include <stdexcept>
#include <map>
#include <sstream>

#include "../aux/Log.h"

#define none -1

namespace NetworKit {

/** Typedefs **/

typedef int64_t index; // more expressive name for an index into an array
typedef int64_t count; // more expressive name for an integer quantity
typedef index node; // node indices are 0-based
typedef double edgeweight; // edge weight type
//template<typename T> using nodemap = std::vector<T>; // more expressive name for container that is indexed by a node
//template<typename T> using edgemap = std::vector<std::vector<T> >;// more expressive name for an edge data structure

class Graph {

protected:

	template<class T>
	class Coordinates {
	private:
		count nv; //!< number of vertices
		count dxy; //!< dimension of coordinates
		std::vector<T> data; //!< array of coordinates, length nv * dxy

	public:
		Coordinates() {}
		void init(count numVertices, count dimensions) {
			nv = numVertices;
			dxy = dimensions;
			data.reserve(nv * dxy);
		}

		void setCoordinate(node v, count dim, T value) {
			data[v * dxy + dim] = value;
		}

		T getCoordinate(node v, count dim) {
			return data[v * dxy + dim];
		}

		T minCoordinate(count dim) {
			T value = data[dim];
			for (index i = dim + dxy; i < nv; i += dxy) {
				if (data[i] < value) {
					value = data[i];
				}
			}
			return value;
		}

		T maxCoordinate(count dim) {
			T value = data[dim];
			for (index i = dim + dxy; i < nv; i += dxy) {
				if (data[i] > value) {
					value = data[i];
				}
			}
			return value;
		}

		virtual ~Coordinates() {}
	};


	// defaults
	static constexpr double defaultEdgeWeight = 1.00;static constexpr edgeweight nullWeight = 0.0;

	// scalars
	count n; //!< current number of nodes
	node z; //!< current upper bound of node ids

	// per node data
	std::vector<count> deg; //!< degree of each node
	std::vector<bool> exists; //!< exists[v] is true if node v has not been removed from the graph
	Coordinates<float> coordinates; //!< coordinates of nodes (if present)

	// per edge data
	std::vector<std::vector<node> > adja; //!< neighbors/adjacencies
	std::vector<std::vector<edgeweight> > eweights; //!< edge weights

	// graph attributes
	std::string name;

	// user-defined edge atributes

	//	attribute maps storage

	std::vector<std::vector<std::vector<double> > > edgeMaps_double; // contains edge maps (u, v) -> double

	// defaults

	std::vector<double> edgeAttrDefaults_double; // stores default value for edgeMaps_double[i] at index i

	/**
	 * Return the index of v in the adjacency array of u.
	 */
	index find(node u, node v) const;

public:

	/** ATTRIBUTE ABSTRACT BASE CLASSES **/

	class NodeAttribute {
		// abstract
	};

	class EdgeAttribute {
		// abstract
	};


	/** GRAPH INTERFACE **/

	Graph(count n);

	virtual ~Graph();

	/**
	 * Set name of graph.
	 */
	void setName(std::string name);

	/*
	 * @return name of graph
	 */
	std::string getName();

	/**
	 * Get string representation
	 */
	std::string toString();

	/**
	 * Insert an undirected edge between two nodes.
	 */
	void addEdge(node u, node v, edgeweight weight = defaultEdgeWeight);

	/**
	 * Check if undirected edge {u,v} exists in G
	 *
	 */
	bool hasEdge(node u, node v) const;

	/**
	 * Remove undirected edge between two nodes.
	 */
	void removeEdge(node u, node v);

	/**
	 * @return Number of neighbors.
	 */
	count degree(node v) const;

	/**
	 * @return Weighted degree of @a v.
	 */
	edgeweight weightedDegree(node v) const;

	/** EDGE ATTRIBUTE GETTERS **/

	/**
	 * Return edge weight.
	 *
	 * Return 0 if edge does not exist.
	 */
	edgeweight weight(node u, node v) const;

	/**
	 * @return attribute of type double for an edge.
	 *
	 * @param[in]	u	node
	 * @param[in]	v	node
	 * @param[in]	attrId	attribute id
	 */
	double attribute_double(node u, node v, int attrId) const;

	/**  EDGE ATTRIBUTE SETTERS */

	/**
	 * Set the weight of an edge. If the edge does not exist,
	 * it will be inserted.
	 *
	 * @param[in]	u	endpoint of edge
	 * @param[in]	v	endpoint of edge
	 * @param[in]	weight	edge weight
	 */
	void setWeight(node u, node v, edgeweight w);

	/**
	 * Set edge attribute of type double If the edge does not exist,
	 * it will be inserted.
	 *
	 * @param[in]	u	endpoint of edge
	 * @param[in]	v	endpoint of edge
	 * @param[in]	attr	double edge attribute
	 */
	void setAttribute_double(node u, node v, int attrId, double attr);

	/** SUMS **/

	/**
	 * @return sum of all edge weights
	 */
	edgeweight totalEdgeWeight() const;

	/**
	 * DEPRECATED - TODO: update clients
	 */
	edgeweight totalNodeWeight();

	/** NODE MODIFIERS **/

	/**
	 * Add a new node to the graph and return it.
	 */
	node addNode();

	/**
	 * Remove a node from the graph.
	 */
	void removeNode(node u);

	/**
	 * Check if node exists in the graph.
	 */
	bool hasNode(node u);

	/**
	 * Return true if graph contains no nodes.
	 */
	bool isEmpty();

	/**
	 * Return the number of nodes in the graph.
	 *
	 */
	count numberOfNodes() const;

	/**
	 * Return the number of edges in the graph.
	 *
	 * This involves calculation, so store result if needed multiple times.
	 */
	count numberOfEdges() const;

	/**
	 * @return the number of loops {v, v} in the graph.
	 *
	 * This involves calculation, so store result if needed multiple times.
	 */
	count numberOfSelfLoops() const;


	void setCoordinate(node v, count dim, float value) {
		coordinates.setCoordinate(v, dim, value);
	}

	float getCoordinate(node v, count dim) {
		return coordinates.getCoordinate(v, dim);
	}

	float minCoordinate(count dim) {
		return coordinates.minCoordinate(dim);
	}

	float maxCoordinate(count dim) {
		return coordinates.maxCoordinate(dim);
	}

	void initCoordinates(count dimensions) {
		coordinates.init(n, dimensions);
	}

	/** ATTRIBUTES **/

	/**
	 * Add new edge map for an attribute of type double.
	 */
	int addEdgeAttribute_double(double defaultValue);

	/** NODE ITERATORS **/

	/**
	 * Iterate over all nodes of the graph and call handler (lambda closure).
	 */
	template<typename L> void forNodes(L handle);

	/**
	 * Iterate over all nodes of the graph and call handler (lambda closure).
	 */
	template<typename L> void forNodes(L handle) const;

	/**
	 * Iterate over all nodes of the graph and call handler (lambda closure) as long as the condition remains true.
	 * This allows for breaking from a node loop.
	 */
	template<typename C, typename L> void forNodesWhile(C condition, L handle);

	/**
	 * Iterate over all nodes of the graph and call handler (lambda closure) as long as the condition remains true.
	 * This allows for breaking from a node loop.
	 */
	template<typename C, typename L> void forNodes(C condition, L handle) const;

	/**
	 * Iterate in parallel over all nodes of the graph and call handler (lambda closure).
	 */
	template<typename L> void parallelForNodes(L handle);

	/**
	 * Iterate in parallel over all nodes of the graph and call handler (lambda closure).
	 * Using schedule(guided) to remedy load-imbalances due to e.g. unequal degree distribution.
	 */
	template<typename L> void balancedParallelForNodes(L handle);

	/**
	 * Iterate in parallel over all nodes of the graph and call handler (lambda closure).
	 */
	template<typename L> void parallelForNodes(L handle) const;

	/**
	 * Iterate over all undirected pairs of nodesand call handler (lambda closure).
	 */
	template<typename L> void forNodePairs(L handle);

	/**
	 * Iterate over all undirected pairs of nodesand call handler (lambda closure).
	 */
	template<typename L> void forNodePairs(L handle) const;

	/**
	 * Iterate over all undirected pairs of nodes in parallel and call handler (lambda closure).
	 */
	template<typename L> void parallelForNodePairs(L handle);

	/**
	 * Iterate over all undirected pairs of nodes in parallel and call handler (lambda closure).
	 */
	template<typename L> void parallelForNodePairs(L handle) const;

	/**
	 * Iterate over nodes in breadth-first search order starting from r until connected component
	 * of r has been visited.
	 */
	template<typename L> void breadthFirstNodesFrom(node r,
			std::vector<int>& marked, L handle);

	/**
	 * Iterate over edges in breadth-first search order starting from node r until connected component
	 * of r has been visited.
	 */
	template<typename L> void breadthFirstEdgesFrom(node r, L handle);

	/**
	 * Iterate over all nodes of the graph and call handler (lambda closure).
	 *
	 * @param[in]	attrKey		attribute key
	 * @param[in]	handle		takes parameters (v, a) where a is a node attribute
	 */
	template<typename L> void forNodesWithAttribute(std::string attrKey,
			L handle);

	/** EDGE ITERATORS **/

	/**
	 * Iterate over all edges of the graph and call handler (lambda closure).
	 */
	template<typename L> void forEdges(L handle);

	/**
	 * Iterate over all edges of the graph and call handler (lambda closure).
	 */
	template<typename L> void forEdges(L handle) const;

	/**
	 * Iterate in parallel over all edges of the graph and call handler (lambda closure).
	 */
	template<typename L> void parallelForEdges(L handle);

	/**
	 * Iterate in parallel over all edges of the graph and call handler (lambda closure).
	 */
	template<typename L> void parallelForEdges(L handle) const;

	/**
	 * Iterate over all edges of the graph and call handler (lambda closure).
	 *
	 * Handler takes arguments (u, v, w) where u and v are the nodes of the edge and w is its weight.
	 *
	 */
	template<typename L> void forWeightedEdges(L handle);

	/**
	 * Iterate over all edges of the graph and call handler (lambda closure).
	 *
	 * Handler takes arguments (u, v, w) where u and v are the nodes of the edge and w is its weight.
	 */
	template<typename L> void forWeightedEdges(L handle) const;

	/**
	 * Iterate over all edges of the graph and call handler (lambda closure).
	 *
	 * Handler takes arguments (u, v, w) where u and v are the nodes of the edge and w is its weight.
	 *
	 */
	template<typename L> void parallelForWeightedEdges(L handle);

	/**
	 * Iterate over all edges of the graph and call handler (lambda closure).
	 *
	 * Handler takes arguments (u, v, w) where u and v are the nodes of the edge and w is its weight.
	 */
	template<typename L> void parallelForWeightedEdges(L handle) const;

	/**
	 * Iterate over all edges of the graph and call handler (lambda closure).
	 *
	 *	@param[in]	attrId		attribute id
	 *	@param[in]	handle 		takes arguments (u, v, a) where a is an edge attribute of edge {u, v}
	 *
	 */
	template<typename L> void forEdgesWithAttribute_double(int attrId,
			L handle);

	/** NEIGHBORHOOD ITERATORS **/

	/**
	 * Iterate over all neighbors of a node and call handler (lamdba closure).
	 *
	 * (Note that a node is its own neighbor if there is a self-loop.)
	 */
	template<typename L> void forNeighborsOf(node u, L handle);

	/**
	 * Iterate over all neighbors of a node and call handler (lamdba closure).
	 */
	template<typename L> void forNeighborsOf(node u, L handle) const;

	/**
	 * Iterate over all edge weights of a node and call handler (lamdba closure).
	 */
	template<typename L> void forWeightedNeighborsOf(node u, L handle);

	/**
	 * Iterate over all edge weights of a node and call handler (lamdba closure).
	 */
	template<typename L> void forWeightedNeighborsOf(node u, L handle) const;

	/**
	 * Iterate over all incident edges of a node and call handler (lamdba closure).
	 */
	template<typename L> void forEdgesOf(node u, L handle);

	/**
	 * Iterate over all incident edges of a node and call handler (lamdba closure).
	 */
	template<typename L> void forEdgesOf(node u, L handle) const;

	/**
	 * Iterate over all incident edges of a node and call handler (lamdba closure).
	 *
	 * Handle takes parameters (u, v, w) where w is the edge weight.
	 *
	 */
	template<typename L> void forWeightedEdgesOf(node u, L handle);

	/**
	 * Iterate over all incident edges of a node and call handler (lamdba closure).
	 *
	 * Handle takes parameters (u, v, w) where w is the edge weight.
	 *
	 */
	template<typename L> void forWeightedEdgesOf(node u, L handle) const;

	/** REDUCTION ITERATORS **/

	/**
	 * Iterate in parallel over all nodes and sum (reduce +) the values returned by the handler
	 */
	template<typename L> double parallelSumForNodes(L handle);

	/**
	 * Iterate in parallel over all nodes and sum (reduce +) the values returned by the handler
	 */
	template<typename L> double parallelSumForNodes(L handle) const;

	/**
	 * Iterate in parallel over all edges and sum (reduce +) the values returned by the handler
	 */
	template<typename L> double parallelSumForWeightedEdges(L handle) const;

};

} /* namespace NetworKit */

template<typename L>
inline void NetworKit::Graph::forNeighborsOf(node u, L handle) {
	for (node v : this->adja[u]) {
		if (v != none) {
			handle(v);
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forNeighborsOf(node u, L handle) const {
	for (node v : this->adja[u]) {
		if (v != none) {
			handle(v);
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forWeightedNeighborsOf(node u, L handle) {
	for (index i = 0; i < (index) adja[u].size(); ++i) {
		node v = adja[u][i];
		if (v != none) {
			edgeweight ew = eweights[u][i];
			handle(v, ew);
			assert(ew == weight(u, v));
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forWeightedNeighborsOf(node u, L handle) const {
	for (index i = 0; i < adja[u].size(); ++i) {
		node v = adja[u][i];
		if (v != none) {
			edgeweight ew = eweights[u][i];
			handle(v, ew);
			assert(ew == weight(u, v));
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forNodes(L handle) {
	for (node v = 0; v < z; ++v) {
		if (exists[v]) {
			handle(v);
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forNodes(L handle) const {
	for (node v = 0; v < z; ++v) {
		if (exists[v]) {
			handle(v);
		}
	}
}

template<typename L>
inline void NetworKit::Graph::parallelForNodes(L handle) {
	node z = n; // store current n in local variable since handle might modify node count
#pragma omp parallel for
	for (node v = 0; v < z; ++v) {
		// call here
		if (exists[v]) {
			handle(v);
		}
	}
}

template<typename L>
inline void NetworKit::Graph::parallelForNodes(L handle) const {
#pragma omp parallel for
	for (node v = 0; v < z; ++v) {
		// call here
		if (exists[v]) {
			handle(v);
		}
	}
}

template<typename L>
inline void NetworKit::Graph::balancedParallelForNodes(L handle) {
#pragma omp parallel for schedule(guided) // TODO: define min block size (and test it!)
	for (node v = 0; v < z; ++v) {
		// call here
		if (exists[v]) {
			handle(v);
		}
	}
}

template<typename L>
inline double NetworKit::Graph::parallelSumForNodes(L handle) {
	double sum = 0.0;
#pragma omp parallel for reduction(+:sum)
	for (node v = 0; v < z; ++v) {
		// call here
		if (exists[v]) {
			sum += handle(v);
		}
	}
	return sum;
}

template<typename L>
inline double NetworKit::Graph::parallelSumForNodes(L handle) const {
	double sum = 0.0;
#pragma omp parallel for reduction(+:sum)
	for (node v = 0; v < z; ++v) {
		// call here
		if (exists[v]) {
			sum += handle(v);
		}
	}
	return sum;
}

template<typename L>
double NetworKit::Graph::parallelSumForWeightedEdges(L handle) const {
	double sum = 0.0;
#pragma omp parallel for reduction(+:sum)
	for (node u = 0; u < z; ++u) {
		for (index i = 0; i < this->adja[u].size(); ++i) {
			node v = this->adja[u][i];
			edgeweight ew = this->eweights[u][i];
			if (u <= v) { // {u, v} instead of (u, v); if v == -1, u < v is not fulfilled
				sum += handle(u, v, ew);
			}
		}
	}
	return sum;
}

template<typename L>
inline void NetworKit::Graph::forEdges(L handle) {
	node z = n; // store current n in local variable since handle might modify node count
	for (node u = 0; u < z; ++u) {
		for (node v : this->adja[u]) {
			if (u <= v) { // {u, v} instead of (u, v); if v == -1, u < v is not fulfilled
				handle(u, v);
			}
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forEdges(L handle) const {
	node z = n; // store current n in local variable since handle might modify node count
	for (node u = 0; u < z; ++u) {
		for (node v : this->adja[u]) {
			if (u <= v) { // {u, v} instead of (u, v); if v == -1, u < v is not fulfilled
				handle(u, v);
			}
		}
	}
}

template<typename L>
inline void NetworKit::Graph::parallelForEdges(L handle) {
	node z = n; // store current n in local variable since handle might modify node count
#pragma omp parallel for
	for (node u = 0; u < z; ++u) {
		for (node v : this->adja[u]) {
			if (u <= v) { // {u, v} instead of (u, v); if v == -1, u < v is not fulfilled
				handle(u, v);
			}
		}
	}
}

template<typename L>
inline void NetworKit::Graph::parallelForEdges(L handle) const {
	node z = n; // store current n in local variable since handle might modify node count
#pragma omp parallel for
	for (node u = 0; u < z; ++u) {
		for (node v : this->adja[u]) {
			if (u <= v) { // {u, v} instead of (u, v); if v == -1, u < v is not fulfilled
				handle(u, v);
			}
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forNodePairs(L handle) {
	for (node u = 0; u < z; ++u) {
		if (exists[u]) {
			for (node v = u + 1; v < z; ++v) {
				// call node pair function
				if (exists[v]) {
					handle(u, v);
				}
			}
		}

	}
}

template<typename L>
inline void NetworKit::Graph::forNodePairs(L handle) const {
	for (node u = 0; u < z; ++u) {
		if (exists[u]) {
			for (node v = u + 1; v < z; ++v) {
				// call node pair function
				if (exists[v]) {
					handle(u, v);
				}
			}
		}

	}
}

template<typename L>
inline void NetworKit::Graph::breadthFirstNodesFrom(node r,
		std::vector<int>& marked, L handle) {
	std::queue<node> q;
	count n = this->numberOfNodes();
	q.push(r); // enqueue root
	marked[r] = 1;
	do {
		node u = q.front();
		q.pop();
		// apply function
		handle(u);
		this->forNeighborsOf(u, [&](node v) {
			if (marked[v] == 0) {
				q.push(v);
				marked[v] = 1;
			}
		});
	} while (!q.empty());
}

template<typename L>
inline void NetworKit::Graph::forEdgesOf(node u, L handle) {
	for (node v : this->adja[u]) {
		if (v != none) {
			handle(u, v);
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forEdgesOf(node u, L handle) const {
	for (node v : this->adja[u]) {
		if (v != none) {
			handle(u, v);
		}
	}
}

template<typename L>
inline void NetworKit::Graph::parallelForNodePairs(L handle) {
#pragma omp parallel for
	for (node u = 0; u < z; ++u) {
		if (exists[u]) {
			for (node v = u + 1; v < z; ++v) {
				// call node pair function
				if (exists[v]) {
					handle(u, v);
				}
			}
		}

	}
}

template<typename L>
inline void NetworKit::Graph::parallelForNodePairs(L handle) const {
#pragma omp parallel for
	for (node u = 0; u < z; ++u) {
		if (exists[u]) {
			for (node v = u + 1; v < z; ++v) {
				// call node pair function
				if (exists[v]) {
					handle(u, v);
				}
			}
		}

	}
}

template<typename L>
inline void NetworKit::Graph::breadthFirstEdgesFrom(node r, L handle) {
	// TODO: implement BFS iterator for edges
	throw std::runtime_error("TODO");
}

template<typename L>
inline void NetworKit::Graph::forWeightedEdges(L handle) {
	for (node u = 0; u < n; ++u) {
		for (index vi = 0; vi < adja[u].size(); ++vi) {
			node v = this->adja[u][vi];
			if (u <= v) {
				edgeweight w = this->eweights[u][vi];
				handle(u, v, w);
			}
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forWeightedEdges(L handle) const {
	for (node u = 0; u < n; ++u) {
		for (index vi = 0; vi < adja[u].size(); ++vi) {
			node v = this->adja[u][vi];
			if (u <= v) {
				edgeweight w = this->eweights[u][vi];
				handle(u, v, w);
			}
		}
	}
}

template<typename L>
inline void NetworKit::Graph::parallelForWeightedEdges(L handle) {
#pragma omp parallel for
	for (node u = 0; u < n; ++u) {
		for (index vi = 0; vi < adja[u].size(); ++vi) {
			node v = this->adja[u][vi];
			if (u <= v) {
				edgeweight w = this->eweights[u][vi];
				handle(u, v, w);
			}
		}
	}
}

template<typename L>
inline void NetworKit::Graph::parallelForWeightedEdges(L handle) const {
#pragma omp parallel for
	for (node u = 0; u < n; ++u) {
		for (index vi = 0; vi < adja[u].size(); ++vi) {
			node v = this->adja[u][vi];
			if (u <= v) {
				edgeweight w = this->eweights[u][vi];
				handle(u, v, w);
			}
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forWeightedEdgesOf(node u, L handle) {
	const count asize = (count) adja[u].size();
	for (index i = 0; i < asize; ++i) {
		node v = adja[u][i];
		if (v != none) {
			edgeweight ew = eweights[u][i];
			handle(u, v, ew);
			assert(ew == weight(u, v));
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forWeightedEdgesOf(node u, L handle) const {
	for (index i = 0; i < adja[u].size(); ++i) {
		node v = adja[u][i];
		if (v != none) {
			edgeweight ew = eweights[u][i];
			handle(u, v, ew);
			assert(ew == weight(u, v));
		}
	}
}

template<typename L>
inline void NetworKit::Graph::forNodesWithAttribute(std::string attrKey,
		L handle) {
	// get nodemap for attrKey

//	auto nodeMap; // ?
//
//	auto findIdPair = this->attrKey2IdPair.find(attrKey);
//	if (findIdPair != this->attrKey2IdPair.end()) {
//		std::pair<index, index> idPair = findIdPair->second;
//		index typeId = idPair.first;
//		index mapId = idPair.second;
//
//		// nodemaps are in a vector, one for each node attribute type int, double, NodeAttribute
//		switch (typeId) {
//		case 0:
//			nodeMap = this->nodeMapsInt[mapId];
//			break;
//		case 1:
//			nodeMap = this->nodeMapsdouble[mapId];
//			break;
//		}
//
//		// iterate over nodes and call handler with attribute
//		this->forNodes([&](node u) {
//			auto attr = nodeMap[u];
//			handle(u, attr);
//		});
//	} else {
//		throw std::runtime_error("node attribute not found");
//	}

	// TODO: iterate over nodes with atributes
	throw std::runtime_error("TODO");
}

template<typename L>
inline void NetworKit::Graph::forEdgesWithAttribute_double(int attrId,
		L handle) {
	std::vector<std::vector<double> > edgeMap = this->edgeMaps_double[attrId];
	for (node u = 0; u < n; ++u) {
		for (index vi = 0; vi < (index) adja[u].size(); ++vi) {
			node v = this->adja[u][vi];
			double attr = edgeMap[u][vi];
			if (u <= v) {
				handle(u, v, attr);
			}
		}
	}
}

template<typename C, typename L>
inline void NetworKit::Graph::forNodesWhile(C condition, L handle) {
	for (node v = 0; v < z; ++v) {
		if (exists[v]) {
			if (!condition()) {
				break; // if condition does not hold, break from loop and do not call handle
			}
			handle(v);

		}
	}
}

template<typename C, typename L>
inline void NetworKit::Graph::forNodes(C condition, L handle) const {
	for (node v = 0; v < z; ++v) {
		if (exists[v]) {
			if (!condition()) {
				break; // if condition does not hold, break from loop and do not call handle
			}
			handle(v);
		}
	}
}

#endif /* GRAPH_H_ */
