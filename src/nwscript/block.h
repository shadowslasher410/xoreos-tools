/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  A block of NWScript bytecode instructions.
 */

#ifndef NWSCRIPT_BLOCK_H
#define NWSCRIPT_BLOCK_H

#include <vector>
#include <deque>

#include "src/common/types.h"

#include "src/nwscript/stack.h"

namespace NWScript {

struct Instruction;
struct SubRoutine;

typedef std::deque<Instruction> Instructions;

/** The types of an edge between blocks. */
enum BlockEdgeType {
	kBlockEdgeTypeUnconditional,    ///< This block follows unconditionally.
	kBlockEdgeTypeConditionalTrue,  ///< This block is a true branch of a conditional.
	kBlockEdgeTypeConditionalFalse, ///< This block is a false branch of a conditional.
	kBlockEdgeTypeFunctionCall,     ///< This block is a function call.
	kBlockEdgeTypeFunctionReturn,   ///< This block is a function return.
	kBlockEdgeTypeStoreState,       ///< This block is a subroutine create by STORESTATE.
	kBlockEdgeTypeDead              ///< This edge is logically dead and will never be taken.
};

/** A block of NWScript instructions. */
struct Block {
	/** The address that starts this block. */
	uint32 address;

	/** The instructions making up this block. */
	std::vector<const Instruction *> instructions;

	std::vector<const Block *> parents;  ///< The blocks leading into this block.
	std::vector<const Block *> children; ///< The blocks following this block.

	/** How this block leads into its children. */
	std::vector<BlockEdgeType> childrenTypes;

	/** The subroutine this block belongs to. */
	const SubRoutine *subRoutine;

	/** The current state of analyzing the stack of this block. */
	StackAnalyzeState stackAnalyzeState;


	Block(uint32 addr) : address(addr), subRoutine(0),
		stackAnalyzeState(kStackAnalyzeStateNone) {

	}

	/** Does this block have child blocks that are conditionally followed? */
	bool hasConditionalChildren() const {
		for (std::vector<BlockEdgeType>::const_iterator t = childrenTypes.begin();
		     t != childrenTypes.end(); ++t)
			if (*t == kBlockEdgeTypeConditionalTrue || *t == kBlockEdgeTypeConditionalFalse)
				return true;

		return false;
	}

	bool hasUnconditionalChildren() const {
		if ((childrenTypes.size() == 1) && (childrenTypes[0] == kBlockEdgeTypeUnconditional))
			return true;

		if ((childrenTypes.size() == 2) &&
		    ((childrenTypes[0] == kBlockEdgeTypeDead) || (childrenTypes[1] == kBlockEdgeTypeDead)))
			return true;

		return false;
	}

	/** Return all child blocks that jump backward, to an earlier, smaller address. */
	std::vector<const Block *> getEarlierChildren(bool includeSubRoutines = false) const;

	/** Return all child blocks that jump forward, to a later, larger address. */
	std::vector<const Block *> getLaterChildren(bool includeSubRoutines = false) const;

	/** Return all parent blocks that jump forward, from an earlier, smaller address. */
	std::vector<const Block *> getEarlierParents(bool includeSubRoutines = false) const;

	/** Return all parent blocks that jump backward, from a later, larger address. */
	std::vector<const Block *> getLaterParents(bool includeSubRoutines = false) const;
};

/** The whole set of blocks found in a script. */
typedef std::deque<Block> Blocks;

/** Construct a control flow graph of interconnected blocks from this complete
 *  set of script instructions.
 */
void constructBlocks(Blocks &blocks, Instructions &instructions);

/** Find the index of a block within another block's children.
 *
 *  If this child does not exist within the parent's children, return SIZE_MAX.
 */
size_t findParentChildBlock(const Block &parent, const Block &child);

/** Return the edge type that connects these two blocks. */
BlockEdgeType getParentChildEdgeType(const Block &parent, const Block &child);

/** Is there a linear path between these two blocks? */
bool hasLinearPath(const Block &block1, const Block &block2);

/** Given a complete set of script blocks, find the block directly following a block. */
const Block *getNextBlock(const Blocks &blocks, const Block &block);

/** Given a complete set of script blocks, find the block directly preceding a block. */
const Block *getPreviousBlock(const Blocks &blocks, const Block &block);

/** Given a complete set of script blocks, find edges between blocks that are logically
 *  dead and will never be taken.
 *
 *  Updates their edge type kBlockEdgeTypeDead.
 */
void findDeadBlockEdges(Blocks &blocks);

} // End of namespace NWScript

#endif // NWSCRIPT_BLOCK_H
