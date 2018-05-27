#pragma once
#include <iostream>
#include <queue>
#include <unordered_map>

using namespace std;

template <class D>
struct FHat
{
	typedef typename D::State State;
	
	struct Node
	{
		double g;
		double h;
		double horig;
		Node* parent;
		State stateRep;
		bool open;
		float eps = 0.5;
		float derr;

	public:
		double getGValue() const { return g; }
		double getHValue() const { return h; }
		double getFValue() const { return g + h; }
		double getFHatValue() const { return g + h + (derr * eps); }
		void setGValue(double val) { g = val; }
		void setHValue(double val) { h = val; }
		double getDErrValue() { return derr; }
		void setDErrValue(double val) { derr = val; }
		State getState() const { return stateRep; }
		Node* getParent() { return parent; }
		bool onOpen() { return open; }
		void close() { open = false; }
		void reopen() { open = true; }

		Node(double g, State treeNode, Node* parent)
			: g(g), parent(parent)
		{
			stateRep = treeNode;
			open = true;
			// Initially, derr(n) = d(n)
			derr = stateRep->d();
			h = stateRep->h();
			horig = h;
		}
	};

	struct CompareNodesFHat
	{
		bool operator()(const Node* n1, const Node* n2) const
		{
			// Tie break on heuristic
			if (n1->getFHatValue() == n2->getFHatValue())
			{
				if (n1->getFValue() == n2->getFValue())
					return n1->getGValue() > n2->getGValue();
				return n1->getFValue() > n2->getFValue();
			}
			return n1->getFHatValue() > n2->getFHatValue();
		}
	};

	struct CompareNodesGreedy
	{
		bool operator()(const Node* n1, const Node* n2) const
		{
			return n1->getHValue() > n2->getHValue();
		}
	};

	struct resultContainer
	{
	public:
		Node* solution;
		int nodesGen;
		int nodesExpand;
		double newBound;
	};

	FHat(D& domain) : domain(domain) {}

	~FHat()
	{
		tree.clear();
	}

	void dijkstra()
	{
		// Initialize all h(n) in closed to infinity
		for (typename map<unsigned long, Node*>::iterator it = closed.begin(); it != closed.end(); it++)
			it->second->setHValue(numeric_limits<float>::infinity());

		while (!closed.empty())
		{
			// Remove best value from open
			Node* s = open.top();
			s->close();
			open.pop();
			// Delete it from closed
			closed.erase(s->getState()->hash());

			if (s->getParent())
			{
				Node* p = s->getParent();
				if (closed.find(p->getState()->hash()) != closed.end() && p->getHValue() > (s->getState()->getEdgeCost() + s->getHValue()))
				{
					// Update the parent's heuristic
					p->setHValue(s->getState()->getEdgeCost() + s->getHValue());
					// Update parent's derr
					p->setDErrValue(s->getDErrValue());
					if (!p->onOpen())
					{
						p->reopen();
						open.push(p);
					}
				}
			}
		}
	}

	resultContainer aStar(Node* start, int lookahead)
	{
		resultContainer res;
		res.solution = NULL;
		res.nodesGen = 0;
		res.nodesExpand = 0;

		int expansions = 0;

		// Empty OPEN and CLOSED
		while (!open.empty())
			open.pop();

		closed.clear();

		// Push start on open and closed
		open.push(start);
		tree[start->getState()->hash()] = start;

		while (!open.empty() && (expansions < lookahead))
		{
			// Pop lowest g-value off open
			Node* node = open.top();
			open.pop();
			node->close();
			closed[node->getState()->hash()] = node;
			res.nodesExpand++;

			expansions++;

			// Check if current node is goal
			if (node->getState()->isGoal())
			{
				res.solution = node;
				return res;
			}

			vector<State> children = node->getState()->generateSuccessors();
			res.nodesGen = res.nodesGen + children.size();
			for (State child : children)
			{
				// No need for duplicate detection in tree world
				Node* childNode = new Node(node->getGValue() + child->getEdgeCost(), child, node);
				open.push(childNode);
				tree[child->hash()] = childNode;
			}
		}

		return res;
	}

	resultContainer search(Node* start, int lookahead)
	{
		resultContainer finalRes;
		finalRes.solution = NULL;
		finalRes.nodesExpand = 0;
		finalRes.nodesGen = 0;

		resultContainer tmpRes;
		tmpRes.solution = NULL;
		tmpRes.nodesExpand = 0;
		tmpRes.nodesGen = 0;

		while (1)
		{
			tmpRes = aStar(start, lookahead);
			finalRes.solution = tmpRes.solution;
			finalRes.nodesExpand += tmpRes.nodesExpand;
			finalRes.nodesGen += tmpRes.nodesGen;
			if (finalRes.solution != NULL)
				return finalRes;
			if (open.empty())
			{
				break;
			}
			Node* goalPrime = open.top();

			// Only move one step towards best on open
			while (goalPrime->getParent() != start)
				goalPrime = goalPrime->getParent();

			// Learning step
			dijkstra();

			start = goalPrime;
		}
		return finalRes;
	}

	private:

		D& domain;
		priority_queue<Node*, vector<Node*>, CompareNodesFHat> open;
		unordered_map<unsigned long, Node*> closed;
		unordered_map<unsigned long, Node*> tree;
};