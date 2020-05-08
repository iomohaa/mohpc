#include <Shared.h>
#include <MOHPC/Misc/MSG/HuffmanTree.h>
#include <cassert>
#include <fstream>
#include <istream>
#include <bitset>

using namespace MOHPC;

constexpr node_t::node_t()
	: left(nullptr), right(nullptr), parent(nullptr)
	, next(nullptr), prev(nullptr)
	, head(nullptr)
	, weight(0)
	, symbol(0)
{
}

constexpr constNode_t::constNode_t()
	: left(nullptr), right(nullptr), parent(nullptr)
	, symbol(0)
{
}

MOHPC::Huff::Huff()
	: blocNode(0)
	, blocPtrs(0)
	, tree(nullptr)
	, lhead(nullptr)
	, loc{ nullptr }
	, freelist(nullptr)
	, nodePtrs{ nullptr }
{
	tree = lhead = loc[NYT] = &(nodeList[blocNode++]);
	tree->symbol = NYT;
	tree->weight = 0;
	lhead->next = lhead->prev = NULL;
	tree->parent = tree->left = tree->right = NULL;
}

MOHPC::Huff::Huff(
	size_t inBlocNode, size_t inBlocPtrs,
	intptr_t treeNum, intptr_t headNum,
	intptr_t inFreeList,
	intptr_t (&inLocs)[HMAX + 1],
	intptr_t *inNodePtrs,
	archivedNode_t *nodes
)
	: blocNode(inBlocNode)
	, blocPtrs(inBlocPtrs)
	, tree(&nodeList[treeNum])
	, lhead(&nodeList[headNum])
	, freelist(&nodePtrs[inFreeList])
{
	for (size_t i = 0; i < HMAX + 1; ++i) {
		loc[i] = inLocs[i] != -1 ? &nodeList[inLocs[i]] : nullptr;
	}

	for (size_t i = 0; i < blocPtrs; ++i) {
		nodePtrs[i] = inNodePtrs[i] != -1 ? &nodeList[inNodePtrs[i]] : nullptr;
	}

	for (size_t i = 0; i < blocNode; ++i)
	{
		const archivedNode_t& in = nodes[i];
		node_t& out = nodeList[i];
		if (in.left		!= -1) out.left		= &nodeList[in.left];
		if (in.right	!= -1) out.right	= &nodeList[in.right];
		if (in.parent	!= -1) out.parent	= &nodeList[in.parent];
		if (in.next		!= -1) out.next		= &nodeList[in.next];
		if (in.prev		!= -1) out.prev		= &nodeList[in.prev];
		if (in.head		!= -1) out.head		= &nodePtrs[in.head];
		out.weight = in.weight;
		out.symbol = in.symbol;
	}
}

void MOHPC::Huff::printNodes(std::ostream& stream)
{
	stream << "static intptr_t locNums[] = { ";
	for (size_t i = 0; i < HMAX + 1; ++i)
	{
		const intptr_t locNum = loc[i] ? loc[i] - nodeList : -1;
		stream << locNum;
		if (i < HMAX) {
			stream << ", ";
		}
	}
	stream << " }" << ";" << std::endl;

	stream << "static intptr_t ptrNums[] = { ";
	for (size_t i = 0; i < blocPtrs; ++i)
	{
		const intptr_t ptrNum = nodePtrs[i] ? nodePtrs[i] - nodeList : -1;
		stream << ptrNum;
		if (i < blocPtrs - 1) {
			stream << ", ";
		}
	}
	stream << " }" << ";" << std::endl;

	stream << "static archivedNode_t archivedNodes[] =" << std::endl;
	stream << "{" << std::endl;
	for (size_t i = 0; i < blocNode; ++i)
	{
		const node_t& node = nodeList[i];

		archivedNode_t arcNode;
		arcNode.left = node.left ? node.left - nodeList : -1; arcNode.right = node.right ? node.right - nodeList : -1; arcNode.parent = node.parent ? node.parent - nodeList : -1;
		arcNode.next = node.next ? node.next - nodeList : -1; arcNode.prev = node.prev ? node.prev - nodeList : -1;
		arcNode.head = node.head ? node.head - nodePtrs : -1;
		arcNode.weight = node.weight;
		arcNode.symbol = node.symbol;

		assert((arcNode.left == -1 || arcNode.left < NUM_NODES)
			&& (arcNode.right == -1 || arcNode.right < NUM_NODES)
			&& (arcNode.parent == -1 || arcNode.parent < NUM_NODES)
			&& (arcNode.next == -1 || arcNode.next < NUM_NODES)
			&& (arcNode.prev == -1 || arcNode.prev < NUM_NODES)
			&& (arcNode.head == -1 || arcNode.head < NUM_NODES));

		stream << '\t' << "{ ";
		stream << arcNode.left << ", " << arcNode.right << ", " << arcNode.parent << ", ";
		stream << arcNode.next << ", " << arcNode.prev << ", ";
		stream << arcNode.head << ", ";
		stream << arcNode.weight << ", ";
		stream << arcNode.symbol;
		stream << " }";

		if (i < blocNode - 1) {
			stream << ", ";
		}

		stream << std::endl;
	}
	stream << "};" << std::endl;

	stream << "ConstHuff huff<" << blocNode << ">(";

	const intptr_t treeNum = tree - nodeList;
	stream << treeNum << ", locNums, archivedNodes";
	/*
	stream << blocNode << ", " << blocPtrs << ", ";

	const intptr_t treeNum = tree - nodeList;
	const intptr_t headNum = lhead - nodeList;
	stream << treeNum << ", " << headNum << ", ";

	const intptr_t freelistNum = freelist - nodePtrs;
	stream << freelistNum << ", ";
	stream << "locNums, ptrNums, archivedNodes";
	*/

	stream << ");" << std::endl;
}

void MOHPC::Huff::addRef(uint8_t ch)
{
	node_t* tnode, * tnode2;
	if (loc[ch] == NULL)
	{
		// if this is the first transmission of this node
		tnode = &(nodeList[blocNode++]);
		tnode2 = &(nodeList[blocNode++]);

		tnode2->symbol = INTERNAL_NODE;
		tnode2->weight = 1;
		tnode2->next = lhead->next;
		if (lhead->next) {
			lhead->next->prev = tnode2;
			if (lhead->next->weight == 1) {
				tnode2->head = lhead->next->head;
			}
			else {
				tnode2->head = get_ppnode();
				*tnode2->head = tnode2;
			}
		}
		else {
			tnode2->head = get_ppnode();
			*tnode2->head = tnode2;
		}
		lhead->next = tnode2;
		tnode2->prev = lhead;

		tnode->symbol = ch;
		tnode->weight = 1;
		tnode->next = lhead->next;
		assert(lhead->next);
		if (lhead->next) {
			lhead->next->prev = tnode;
			assert(lhead->next->weight == 1);
			if (lhead->next->weight == 1) {
				tnode->head = lhead->next->head;
			}
			else {
				// this should never happen
				tnode->head = get_ppnode();
				*tnode->head = tnode2;
			}
		}
		else {
			// this should never happen
			tnode->head = get_ppnode();
			*tnode->head = tnode;
		}
		lhead->next = tnode;
		tnode->prev = lhead;
		tnode->left = tnode->right = NULL;

		if (lhead->parent) {
			if (lhead->parent->left == lhead) {
				// lhead is guaranteed to by the NYT
				lhead->parent->left = tnode2;
			}
			else {
				lhead->parent->right = tnode2;
			}
		}
		else {
			tree = tnode2;
		}

		tnode2->right = tnode;
		tnode2->left = lhead;

		tnode2->parent = lhead->parent;
		lhead->parent = tnode->parent = tnode2;

		loc[ch] = tnode;

		increment(tnode2->parent);
	}
	else {
		increment(loc[ch]);
	}
}

uintptr_t MOHPC::Huff::receive(const uint8_t* fin, size_t& bloc) const
{
	node_t* node = tree;
	while (node && node->symbol == INTERNAL_NODE) {
		if (getBit(fin, bloc)) {
			node = node->right;
		}
		else {
			node = node->left;
		}
	}
	if (!node) {
		return 0;
	}
	return node->symbol;
}

void MOHPC::Huff::transmit(uint8_t ch, uint8_t* fout, size_t& offset) const
{
	if (loc[ch] == NULL) {
		// node_t hasn't been transmitted, send a NYT, then the symbol
		//transmit(NYT, fout, offset);
		send(loc[NYT], NULL, fout, offset);
		for (int8_t i = 7; i >= 0; i--) {
			addBit((uint8_t)((ch >> i) & 0x1), fout, offset);
		}
	}
	else {
		send(loc[ch], NULL, fout, offset);
	}
}

uintptr_t MOHPC::Huff::offsetReceive(const uint8_t* fin, size_t* offset) const
{
	node_t* node = tree;
	size_t bloc = *offset;
	while (node && node->symbol == INTERNAL_NODE) {
		if (getBit(fin, bloc)) {
			node = node->right;
		}
		else {
			node = node->left;
		}
	}
	if (!node) {
		return 0;
	}
	*offset = bloc;
	return node->symbol;
}

void MOHPC::Huff::offsetTransmit(uint8_t ch, uint8_t* fout, size_t* offset) const
{
	send(loc[ch], NULL, fout, *offset);
}

void MOHPC::Huff::send(const node_t* node, const node_t* child, uint8_t* fout, size_t& offset) const
{
	if (node->parent) {
		send(node->parent, node, fout, offset);
	}
	if (child) {
		if (node->right == child) {
			addBit(1, fout, offset);
		}
		else {
			addBit(0, fout, offset);
		}
	}
}

void MOHPC::Huff::increment(node_t* node)
{
	if (!node) {
		return;
	}

	if (node->next != NULL && node->next->weight == node->weight) {
		node_t* lnode = *node->head;
		if (lnode != node->parent) {
			swap(lnode, node);
		}
		swaplist(lnode, node);
	}
	if (node->prev && node->prev->weight == node->weight) {
		*node->head = node->prev;
	}
	else {
		*node->head = NULL;
		free_ppnode(node->head);
	}
	node->weight++;
	if (node->next && node->next->weight == node->weight) {
		node->head = node->next->head;
	}
	else {
		node->head = get_ppnode();
		*node->head = node;
	}
	if (node->parent) {
		increment(node->parent);
		if (node->prev == node->parent) {
			swaplist(node, node->parent);
			if (*node->head == node) {
				*node->head = node->parent;
			}
		}
	}
}

MOHPC::node_t** MOHPC::Huff::get_ppnode()
{
	if (!freelist) {
		return &(nodePtrs[blocPtrs++]);
	}
	else {
		node_t** tppnode = freelist;
		freelist = (node_t**)*tppnode;
		return tppnode;
	}
}

void MOHPC::Huff::free_ppnode(node_t** ppnode)
{
	*ppnode = (node_t*)freelist;
	freelist = ppnode;
}

void MOHPC::Huff::swap(node_t* node1, node_t* node2)
{
	node_t* par1 = node1->parent;
	node_t* par2 = node2->parent;

	if (par1) {
		if (par1->left == node1) {
			par1->left = node2;
		}
		else {
			par1->right = node2;
		}
	}
	else {
		tree = node2;
	}

	if (par2) {
		if (par2->left == node2) {
			par2->left = node1;
		}
		else {
			par2->right = node1;
		}
	}
	else {
		tree = node1;
	}

	node1->parent = par2;
	node2->parent = par1;
}

void MOHPC::Huff::swaplist(node_t* node1, node_t* node2) const
{
	node_t* par1 = node1->next;
	node1->next = node2->next;
	node2->next = par1;

	par1 = node1->prev;
	node1->prev = node2->prev;
	node2->prev = par1;

	if (node1->next == node1) {
		node1->next = node2;
	}
	if (node2->next == node2) {
		node2->next = node1;
	}
	if (node1->next) {
		node1->next->prev = node1;
	}
	if (node2->next) {
		node2->next->prev = node2;
	}
	if (node1->prev) {
		node1->prev->next = node1;
	}
	if (node2->prev) {
		node2->prev->next = node2;
	}
}

/*
static uint8_t shiftMapping[] =
{
	0 >> 3, 1 >> 3, 2 >> 3, 3 >> 3, 4 >> 3, 5 >> 3, 6 >> 3, 7 >> 3,
	8 >> 3, 9 >> 3, 10 >> 3, 11 >> 3, 12 >> 3, 13 >> 3, 14 >> 3, 15 >> 3,
	16 >> 3, 17 >> 3, 18 >> 3, 19 >> 3, 20 >> 3, 21 >> 13, 22 >> 3, 23 >> 3,
};

static uint8_t andMapping[] =
{
	0 & 7, 1 & 7, 2 & 7, 3 & 7, 4 & 7, 5 & 7, 6 & 7, 7 & 7,
	8 & 7, 9 & 7, 10 & 7, 11 & 7, 12 & 7, 13 & 7, 14 & 7, 15 & 7,
	16 & 7, 17 & 7, 18 & 7, 19 & 7, 20 & 7, 21 & 7, 22 & 7, 23 & 7,
};
*/

void MOHPC::Huff::addBit(uint8_t bit, uint8_t* fout, size_t& bloc)
{
	const size_t index = bloc >> 3;
	const uint8_t bf = bloc++ & 7;
	if (bf == 0) {
		fout[index] = 0;
	}
	fout[index] |= bit << bf;
}

uint8_t MOHPC::Huff::getBit(const uint8_t* fin, size_t& bloc)
{
	const uint8_t t = (fin[(bloc >> 3)] >> (bloc & 7)) & 1;
	//const uint8_t t = (fin[shiftMapping[bloc]] >> andMapping[bloc]) & 1;
	++bloc;
	return t;
}
