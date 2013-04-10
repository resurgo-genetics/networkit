/*
 * GraphEventHandler.h
 *
 *  Created on: 02.04.2013
 *      Author: cls
 */

#ifndef GRAPHEVENTHANDLER_H_
#define GRAPHEVENTHANDLER_H_

#include "../graph/Graph.h"

namespace NetworKit {


class GraphEventHandler {

public:

	GraphEventHandler();

	virtual ~GraphEventHandler();

	virtual void onNodeAddition(node u) = 0;

	virtual void onNodeRemoval(node u) = 0;

	virtual void onEdgeAddition(node u, node v) = 0;

	virtual void onEdgeRemoval(node u, node v) = 0;

	virtual void onWeightUpdate(node u, node v, edgeweight w) = 0;
};

} /* namespace NetworKit */
#endif /* GRAPHEVENTHANDLER_H_ */
