/**
* Copyright © 2025 Valentin Gorelov
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
* documentation files (the “Software”), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice
* shall be included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @brief
 * @author Valentin Gorelov <gorelov.valentin@gmail.com>
 */

#ifndef ATCMD_TRIEBUILDER_H
#define ATCMD_TRIEBUILDER_H

#include <array>
#include <cstdint>
#include <assert.h>

#include <atcmd/detail/characters.h>

namespace atcmd::detail {

struct TrieBuilder
{
	enum MASKS : uint8_t
	{
		MASKS_CHAR = 0x3F,
		MASKS_LEAF = 1u << 6,
		MASKS_LAST = 1u << 7,
	};

private:
	struct Node
	{
		uint8_t ch;
		Node* children[Characters::getAlphabetSize()];
		std::size_t child_count;

		uint16_t leaf_cmd_index;

		std::size_t subtree_size;
		std::size_t subtree_size_rooted;
	};

	static consteval Node buildTrie(const char* const names[], const uint_fast16_t size)
	{
		using Characters = atcmd::detail::Characters;

		assert((size <= 0x3FFF) && "No more than 16383 Extended Syntax commands are supported");

		Node root = {};
		for (uint_fast16_t i = 0; i < size; i++)
		{
			const char* name = names[i];

			Node* current = &root;
			std::size_t j = 0;
			while (name[j] != '\0')
			{
				uint8_t encoded = Characters::encode(name[j]);
				assert((encoded != 0xFF) && "Unsupported character in Extended Syntax command name");
				uint_fast8_t k;
				for (k = 0; k < current->child_count; k++)
				{
					if (current->children[k]->ch == encoded)
					{
						break;
					}
				}
				if (k == current->child_count)
				{
					current->children[k] = new Node();
					current->children[k]->ch = encoded;
					current->children[k]->child_count = 0;
					current->children[k]->leaf_cmd_index = 0;
					current->child_count++;
				}
				current = current->children[k];
				j++;
			}
			assert((current->leaf_cmd_index == 0) && "Duplicated Extended Syntax command name");
			current->leaf_cmd_index = i + 1;
		}
		return root;
	}

	static consteval void releaseTrieChild(Node* root)
	{
		for (uint_fast8_t i = 0; i < root->child_count; i++)
		{
			releaseTrieChild(root->children[i]);
		}
		delete root;
	}

	static consteval void releaseTrie(Node* root)
	{
		for (uint_fast8_t i = 0; i < root->child_count; i++)
		{
			releaseTrieChild(root->children[i]);
		}
	}

	static consteval void calcSubtreeSize(Node* root)
	{
		root->subtree_size = 0;
		for (uint_fast8_t i = 0; i < root->child_count; i++)
		{
			calcSubtreeSize(root->children[i]);
			root->subtree_size += root->children[i]->subtree_size_rooted;
		}
		root->subtree_size_rooted = root->subtree_size + 1; // + ch header

		// Do we need to encode command index?
		if (root->leaf_cmd_index != 0)
		{
			root->subtree_size_rooted++;
			if (root->leaf_cmd_index > 0x40)
			{
				root->subtree_size_rooted++;
			}
		}

		// We need 1 to 4 bytes to encode the subtree size
		assert((root->subtree_size <= 0x1FFFFFFF) && "Subtree is too large");
		if (root->subtree_size > 0x1FFFFF)
		{
			root->subtree_size_rooted += 4;
		}
		else if (root->subtree_size > 0x3FFF)
		{
			root->subtree_size_rooted += 3;
		}
		else if (root->subtree_size > 0x7F)
		{
			root->subtree_size_rooted += 2;
		}
		else
		{
			root->subtree_size_rooted++;
		}
	}

	static consteval uint8_t* pack(Node* root, uint8_t* dest)
	{
		// Pack the character and, if present, cmd index
		uint8_t h = root->ch;
		if (root->leaf_cmd_index != 0)
		{
			h |= MASKS::MASKS_LEAF;
		}
		*dest = h;
		dest++;

		// Pack the command index if it is present
		if (root->leaf_cmd_index != 0)
		{
			uint16_t cmd_index = root->leaf_cmd_index - 1;
			*dest = cmd_index & 0x7F;
			cmd_index >>= 7u;
			if (cmd_index != 0)
			{
				*dest |= 1u << 7;
				dest++;
				*dest = cmd_index & 0xFF;
				dest++;
			}
			else
			{
				dest++;
			}
		}

		// Pack the subtree size
		std::size_t s = root->subtree_size;
		*dest = s & 0x7F;
		s >>= 7u;
		if (s != 0)
		{
			*dest |= 1u << 7;
			dest++;
			*dest = s & 0x7F;
			s >>= 7u;
			if (s != 0)
			{
				*dest |= 1u << 7;
				dest++;
				*dest = s & 0x7F;
				s >>= 7u;
				if (s != 0)
				{
					*dest |= 1u << 7;
					dest++;
					*dest = s & 0xFF;
				}
				else
				{
					dest++;
				}
			}
			else
			{
				dest++;
			}
		}
		else
		{
			dest++;
		}

		// Pack all the children
		if (root->child_count != 0)
		{
			for (uint_fast8_t i = 0; i < root->child_count - 1; i++)
			{
				dest = pack(root->children[i], dest);
			}
			// The last child should be marked
			uint8_t* last = dest;
			dest = pack(root->children[root->child_count - 1], dest);
			*last |= MASKS::MASKS_LAST;
		}

		return dest;
	}

public:
	static consteval std::size_t getTrieSize(const char* const names[], const uint_fast16_t size)
	{
		assert((size <= 0x3FFF) && "No more than 16383 Extended Syntax commands are supported");
		Node root = buildTrie(names, size);
		calcSubtreeSize(&root);
		releaseTrie(&root);

		return root.subtree_size_rooted;
	}

	template<std::size_t N>
	static consteval auto getTrie(const char* const names[], const uint_fast16_t size)
	{
		Node root = buildTrie(names, size);
		calcSubtreeSize(&root);
		std::array<uint8_t, N> r = {};
		pack(&root, r.data());
		releaseTrie(&root);
		return r;
	}
};

}

#endif // ATCMD_TRIEBUILDER_H
