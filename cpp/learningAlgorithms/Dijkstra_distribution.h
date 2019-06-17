#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include "../utility/PriorityQueue.h"
#include "LearningAlgorithm.h"

using namespace std;

template <class Domain, class Node, class TopLevelAction>
class DijkstraDistribution : public LearningAlgorithm<Domain, Node, TopLevelAction>
{
	typedef typename Domain::State State;
	typedef typename Domain::Cost Cost;
	typedef typename Domain::HashState Hash;

public:
	DijkstraDistribution(Domain& domain)
		: domain(domain)
	{}

	void learn(PriorityQueue<shared_ptr<Node> > open, unordered_map<State, shared_ptr<Node>, Hash> closed)
	{
		// Start by initializing every state in closed to inf h
		for (typename unordered_map<State, shared_ptr<Node>, Hash>::iterator it = closed.begin(); it != closed.end(); it++)
		{
			if (!it->second->onOpen())
				domain.updateDistribtuion(it->first, Null);
		}

		// Order open by f-hat
		open.swapComparator(Node::compareNodesFHatFromDist);

		// Perform reverse dijkstra while closed is not empy
		while (!closed.empty() && !open.empty())
		{
			shared_ptr<Node> cur = open.top();
			open.pop();

			closed.erase(cur->getState());

			// Now get all of the predecessors of cur
			for (State s : domain.predecessors(cur->getState()))
			{
				typename unordered_map<State, shared_ptr<Node>, Hash>::iterator it = closed.find(s);

				if (it != closed.end() &&
					domain.hhat(s) > domain.getEdgeCost(cur->getState()) + domain.hhat(cur->getState()))
				{
					// Update the heuristic of this pedecessor
					domain.updateDistribtuion(s, domain.getEdgeCost(cur->getState()) + domain.getDistribution(cur->getState()));

					it->second->setDistribtuion(newdistribution);

					if (open.find(it->second) == open.end())
					{
						open.push(it->second);
					}
					else
					{
						open.update(it->second);
					}
				}
			}
		}
	}
	
protected:
	Domain & domain;
};
