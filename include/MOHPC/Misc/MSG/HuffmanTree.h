#pragma once

#include <stdint.h>
#include <stddef.h>
#include <MOHPC/Math.h>
#include <istream>

namespace MOHPC
{
	struct MOHPC_EXPORTS node_t
	{
		/* tree structure */
		node_t* left, * right, * parent;
		/* doubly-linked list */
		node_t* next, * prev;
		/* highest ranked node in block */
		node_t** head;
		size_t weight;
		uint16_t symbol;

		constexpr node_t();
	};

	struct MOHPC_EXPORTS constNode_t
	{
		/* tree structure */
		constNode_t* left, * right, * parent;
		/* highest ranked node in block */
		uint16_t symbol;

		constexpr constNode_t();
	};

	struct archivedNode_t
	{
		intptr_t left, right, parent;
		intptr_t next, prev;
		intptr_t head;

		size_t weight;
		uint16_t symbol;
	};

	class Huff
	{
	public:
		/* Maximum symbol */
		static constexpr uint16_t HMAX = 256;
		/* NYT = Not Yet Transmitted */
		static constexpr uint16_t NYT = HMAX;
		static constexpr uint16_t INTERNAL_NODE = HMAX + 1;
		static constexpr uint16_t NUM_NODES = 768;

	private:
		size_t blocNode;
		size_t blocPtrs;

		node_t* tree;
		node_t* lhead;
		node_t* loc[HMAX + 1];
		node_t** freelist;

		node_t nodeList[NUM_NODES];
		node_t* nodePtrs[NUM_NODES];

	public:
		Huff();

		// Base constructor for importing data
		Huff(
			size_t inBlocNode,
			size_t inBlocPtrs,
			intptr_t treeNum,
			intptr_t headNum,
			intptr_t inFreeList,
			intptr_t (&inLocs)[HMAX + 1],
			intptr_t *inNodePtrs,
			archivedNode_t *nodes
		);

		void addRef(uint8_t ch);
		uintptr_t receive(const uint8_t* fin, size_t& bloc) const;
		void transmit(uint8_t ch, uint8_t* fout, size_t& offset) const;
		uintptr_t offsetReceive(const uint8_t* fin, size_t* offset) const;
		void offsetTransmit(uint8_t ch, uint8_t* fout, size_t* offset) const;
		void send(const node_t* node, const node_t* child, uint8_t* fout, size_t& offset) const;

		void printNodes(std::ostream& stream);

		static void addBit(uint8_t bit, uint8_t* fout, size_t& bloc);
		static uint8_t getBit(const uint8_t* fin, size_t& bloc);

	private:
		void increment(node_t* node);
		node_t** get_ppnode();
		void free_ppnode(node_t** ppnode);
		void swap(node_t* node1, node_t* node2);
		void swaplist(node_t* node1, node_t* node2) const;
	};

	/**
	 * Used to pre-initialize an huffman tree with existing data
	 * Also gain performance during initialization & runtime
	 */
	template<size_t numNodes>
	class ConstHuff
	{
	public:
		const constNode_t* tree;
		const constNode_t* loc[Huff::HMAX + 1];
		constNode_t nodeList[numNodes];

	public:
		// Base constructor for importing data
		constexpr ConstHuff(const intptr_t treeNum, const intptr_t(&inLocs)[Huff::HMAX + 1], const archivedNode_t(&nodes)[numNodes])
			: tree(&nodeList[treeNum])
		{
			for (size_t i = 0; i < Huff::HMAX + 1; ++i) {
				loc[i] = inLocs[i] != -1 ? &nodeList[inLocs[i]] : nullptr;
			}

			for (size_t i = 0; i < numNodes; ++i)
			{
				const archivedNode_t& in = nodes[i];
				constNode_t& out = nodeList[i];
				if (in.left != -1) out.left = &nodeList[in.left];
				if (in.right != -1) out.right = &nodeList[in.right];
				if (in.parent != -1) out.parent = &nodeList[in.parent];
				out.symbol = in.symbol;
			}
		}

		uintptr_t receive(const uint8_t* fin, size_t& bloc) const
		{
			const constNode_t* node = tree;
			while (node && node->symbol == Huff::INTERNAL_NODE) {
				node = Huff::getBit(fin, bloc) ? node->right : node->left;
			}
			return node ? node->symbol : 0;
		}

		void transmit(uint8_t ch, uint8_t* fout, size_t& offset) const
		{
			send(loc[ch], NULL, fout, offset);
		}

		uint16_t offsetReceive(const uint8_t* fin, size_t* offset) const
		{
			const constNode_t* node = tree;
			size_t bloc = *offset;
			while (node && node->symbol == Huff::INTERNAL_NODE) {
				node = Huff::getBit(fin, bloc) ? node->right : node->left;
			}
			if (!node) {
				return 0;
			}
			*offset = bloc;
			return node->symbol;
		}

		void offsetTransmit(uint8_t ch, uint8_t* fout, size_t* offset) const
		{
			send(loc[ch], NULL, fout, *offset);
		}

		void send(const constNode_t* node, const constNode_t* child, uint8_t* fout, size_t& offset) const
		{
			if (node->parent) {
				send(node->parent, node, fout, offset);
			}
			if (child) {
				Huff::addBit(node->right == child, fout, offset);
			}
		}
	};
};
