/********************************************************************
Copyright (C) 1998-2006 SRC, LLC

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License version 2.1 as published by the Free Software Foundation

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*********************************************************************/

/*
# $Rev: 53 $ 
# $Date: 2006-10-06 07:00:31 +0200 (Fri, 06 Oct 2006) $ 
*/

///////////////////////////////////////////////////////////////////////////////
// Trie.h: Implementation of a "trie" data structure using child-sibling lists.
///////////////////////////////////////////////////////////////////////////////

#ifndef INCL_TRIE_H
#define INCL_TRIE_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include <string.h>
#include "RefPtr.h"
#include "Freelist.h"

#ifndef NDEBUG
	#include <iostream>
#endif

#include "Global_DllExport.h"

namespace PortfolioExplorer {

	///////////////////////////////////////////////////////////////////////////////
	// Template class for Trie implementation.
	// <T> is the data type to be associated with each string.
	// T must support the comparisons (T==0) and (T!=0) to indicate a null value.
	// There must be a constructor for T, such that the declaration 
	//		T t(0)
	// is valid and the expression t==0 is true.  Thus, this really works for
	// pointers, smart-pointers and scalars.
	///////////////////////////////////////////////////////////////////////////////
	template <class T> class Trie : public VRefCount {
	public:
		///////////////////////////////////////////////////////////////////////////////
		// Constructor/destructor
		///////////////////////////////////////////////////////////////////////////////
		Trie() :
			rootData(T(0)),
			nullData(T(0))
		{
			memset(roots, 0, sizeof(roots));
			trieNodeAllocator = new FreeList<TrieNode>;
		}

		virtual ~Trie() 
		{
			Free();
		}
		
		inline void Free()
		{
			for (unsigned int x=0; x<(sizeof(roots)/sizeof(*roots)); ++x)
			{
				if (roots[x]!=NULL)
					roots[x]->Free();
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Insert a string and associated data item to the trie.
		// Inptus:
		//	const unsigned char*	key		The key to insert
		//	T						data	The data to assoicate with the key
		//
		// Note: If the key already exists, then the new data will replace the old
		// data in the trie.
		///////////////////////////////////////////////////////////////////////////////
		void Insert(const unsigned char* key, T data);
		void Insert(const char* key, T data) { Insert((const unsigned char*)key, data); }

		///////////////////////////////////////////////////////////////////////////////
		// Find the data associated with the key.
		// Inputs:
		//	const unsigned char*	key		The key to find
		// Return value:
		//	T		The found value, or T(0) if the key is not found or there
		//			is no data associated with the key.
		///////////////////////////////////////////////////////////////////////////////
		T Find(const unsigned char* key);
		T Find(const char* key) { return Find((const unsigned char*)key); }

		///////////////////////////////////////////////////////////////////////////////
		// Determine if the given key exists as a prefix of a key in the Trie.
		// Inputs:
		//	const unsigned char*	key		The key to find
		// Outputs:
		//	int&					length	Length of the prefix found (if one was found)
		// Return value:
		//	bool		true if found, false if not found.
		///////////////////////////////////////////////////////////////////////////////
		bool FindTriePrefix(const unsigned char* key, int& length);
		bool FindTriePrefix(const char* key, int& length) { 
			return FindTriePrefix((const unsigned char*)key, length); 
		}

		///////////////////////////////////////////////////////////////////////////////
		// Determine if some key in the trie is a prefix of the given value.
		// Inputs:
		//	const unsigned char*	value	The value to search
		// Outputs:
		//	int&					length	Length of the prefix key found (if one was found)
		// Return value:
		//	bool		true if found, false if not found.
		///////////////////////////////////////////////////////////////////////////////
		bool FindKeyPrefix(const unsigned char* value, int& length);
		bool FindKeyPrefix(const char* value, int& length) { 
			return FindKeyPrefix((const unsigned char*)value, length); 
		}

		///////////////////////////////////////////////////////////////////////////////
		// Like FindKeyPrefix, but returns longest prefix instead of first one.
		// Inputs:
		//	const unsigned char*	value	The value to search.
		// Outputs:
		//	int&					length	Length of the prefix key found (if one was found)
		// Return value:
		//	bool		true if found, false if not found.
		///////////////////////////////////////////////////////////////////////////////
		bool FindLongestKeyPrefix(const unsigned char* value, int& length);
		bool FindLongestKeyPrefix(const char* value, int& length) { 
			return FindLongestKeyPrefix((const unsigned char*)value, length); 
		}

		///////////////////////////////////////////////////////////////////////////////
		// Determine if some key in the trie is a prefix of the given value, and if
		// so return the associated data.  Searches for the shortest prefix found.
		// Inputs:
		//	const unsigned char*	value	The value to search
		// Outputs:
		//	T&						result	The associated data, if found.
		//	int&					length	Length of the prefix key, if found.
		// Return value:
		//	bool		true if found, false if not found.
		///////////////////////////////////////////////////////////////////////////////
		bool FindKeyPrefix(const unsigned char* value, T& result, int& length);
		bool FindKeyPrefix(const char* value, T& result, int& length) { 
			return FindKeyPrefix((const unsigned char*)value, result, length); 
		}

		///////////////////////////////////////////////////////////////////////////////
		// Like FindKeyPrefix, but returns longest prefix instead of first one.
		// Returns the associated data if found.
		// Inputs:
		//	const unsigned char*	value	The value to search.
		// Outputs:
		//	T&						result	The associated data value
		//	int&					length	Length of the prefix key found (if one was found)
		// Return value:
		//	bool		true if found, false if not found.
		///////////////////////////////////////////////////////////////////////////////
		bool FindLongestKeyPrefix(const unsigned char* value, T& result, int& length);
		bool FindLongestKeyPrefix(const char* value, T& result, int& length) { 
			return FindLongestKeyPrefix((const unsigned char*)value, result, length); 
		}

		///////////////////////////////////////////////////////////////////////////////
		// Clear the trie of all keys and data
		///////////////////////////////////////////////////////////////////////////////
		void Clear() 
		{
			Free();
			memset(roots, 0, sizeof(roots));
			trieNodeAllocator = new FreeList<TrieNode>;
			rootData = nullData;
		}

	#ifndef NDEBUG
		///////////////////////////////////////////////////////////////////////////////
		// Dump the trie to a stream
		///////////////////////////////////////////////////////////////////////////////
		void Dump(std::ostream& os) {
			for (int i = 0; i < 256; i++) {
				Dump(os, roots[i]);
			}
		}
	#endif

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Struct representing a node in the trie.
		// It can also represent several "collapsed" parent-child nodes as a single
		// node for compactness.
		///////////////////////////////////////////////////////////////////////////////
		struct TrieNode {
			// Default constructor.
			inline TrieNode() : 
				data(0), sibling(0), child(0)
			{ 
				memset(c, 0, MaxCompression); 
			}

			inline void Free()
			{
				data = 0;

				if (child!=NULL)
					child->Free();
				if (sibling!=NULL)
					sibling->Free();
			}

			// Represents up to four nodes connected parent-to-child with no
			// intervening branching or attached data.  c[0] is always non-zero
			// and contains the letter of the first logical node.  c[1]..c[3] may
			// be non-zero to indicate extra child nodes that have been collapsed into
			// this single node.  Note that if c[N] is non-zero and N>0, then
			// c[N-1] is also non-zero.
			enum { MaxCompression = 4 };
			unsigned char c[MaxCompression];
			// Data, or zero if no data.  If the c[] array contains
			// more than one non-zero entry, then this data is logically attached
			// to the *last* non-zero character.
			T data;
			// Sibling pointer.  This points to other nodes that are 
			// children of this node's parent, but have a different c[0].
			// This is zero for the last sibling in the chain.  Siblings
			// are not ordered.
			TrieNode* sibling;
			// Child pointer.  This points to the first child of this node,
			// or zero if there are no children.
			TrieNode* child;

			///////////////////////////////////////////////////////////////////////////////
			// Find a child of the node
			// Inputs:
			//	unsigned char	c		The letter of the child to find.
			// Return value:
			//	TrieNode*		The node found, or zero if none.
			///////////////////////////////////////////////////////////////////////////////
			TrieNode* FindChild(unsigned char c) {
				for (TrieNode* node = child; node != 0; node = node->sibling) {
					if (node->c[0] == c) {
						return node;
					}
				}
				return 0;
			}

		};

		// Split the node at the given key position, creating a child node
		// in the process.
		void SplitNode(TrieNode* node, int position) {
			TrieNode* split = trieNodeAllocator->New();
			// The subchild will "steal" this node's children and data.
			split->child = node->child;
			split->data = node->data;
			node->data = nullData;
			// Make it a child of this.
			node->child = split;
			// Move trailing portion of key to the subchild.
			memcpy(split->c, node->c + position, TrieNode::MaxCompression - position);
			memset(node->c + position, 0, TrieNode::MaxCompression - position);
		}

	#ifndef NDEBUG
		///////////////////////////////////////////////////////////////////////////////
		// Private dump helper
		///////////////////////////////////////////////////////////////////////////////
		void Dump(std::ostream& os, TrieNode* node, int level = 0) {
			if (node == 0) return;
			os << TsString(level * 2, ' ').c_str();
			for (int i = 0; i < TrieNode::MaxCompression && node->c[i] != 0; i++) {
				os << (char)node->c[i];
				os << ' ';
				level++;
			}
			if (node->data != 0) {
				os << "(" << node->data << ")";
			}
			os << "\n";
			for (TrieNode* tmp = node->child; tmp != 0; tmp = tmp->sibling) {
				Dump(os, tmp, level);
			}
		}
	#endif

		// "Root" nodes, indexed by first letter for speed.
		// Root nodes have no siblings.  Will be null until allocated.
		TrieNode *roots[256];

		// Root data, associated with the empty string.
		T rootData;

		// Freelist allocator.
		refcnt_ptr<FreeList<TrieNode> > trieNodeAllocator;

		// Null data value
		T nullData;
	};


	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// Out-of-line template bodies
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Insert a key and value
	///////////////////////////////////////////////////////////////////////////////
	template<class T> void Trie<T>::Insert(
		const unsigned char* key, 
		T data
	) {
		if (*key == 0) {
			// Empty-string data
			rootData = data;
			return;
		}

		TrieNode* current = 0;
		while (*key != 0) {
			TrieNode* child;
			if (current == 0) {
				// Looking for a root.
				child = roots[*key];
			} else {
				// Descending the trie
				child = current->FindChild(*key);
			}
			if (child == 0) {
				// Child does not exist at this point.  Allocate the new child node.
				child = trieNodeAllocator->New();
				if (current == 0) {
					// Install new node at the root
					roots[*key] = child;
				} else {
					// Install new node as a child of the current node
					// by inserting it at the front of the sibling list.
					child->sibling = current->child;
					current->child = child;
				}
				// Initialize with available key data.
				for (int idx = 0; idx < TrieNode::MaxCompression && *key != 0; idx++, key++) {
					child->c[idx] = *key;
				}
				// Keep going
				current = child;
			} else {
				// Found the child.  Follow the portion of the child
				// letter-sequence that matches the key.
				assert(child->c[0] == *key && *key != 0);
				key++;
				for (
					int idx = 1; 
					idx < TrieNode::MaxCompression && child->c[idx] != 0; 
					idx++, key++
				) {
					if (child->c[idx] != *key) {
						// There is a partial key match.  Note that this covers the
						// case where we hit the end of the key.
						// We must split the child node into two nodes and keep going.
						SplitNode(child, idx);
						break;
					}
				}
				// Traverse to child and continue key processing
				current = child;
			}
		}
		// Set the data on the final node.
		current->data = data;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Find the value associated with a key.
	///////////////////////////////////////////////////////////////////////////////
	template <class T> T Trie<T>::Find(
		const unsigned char* key
	) {
		if (*key == 0) {
			return rootData;
		}

		TrieNode* node = roots[*key];
		while (node != 0) {
			assert(node->c[0] == *key);
			int idx = 0;
			while (true) {
				if (idx == TrieNode::MaxCompression || node->c[idx] == 0) {
					// End of this node's string.  Is this also the end of the key?
					if (*key == 0) {
						// End of the key.  Lookup succeeded.
						return node->data;
					} else {
						// Keep going.  Find the next child.
						node = node->FindChild(*key);
						break;
					}
				} else if (node->c[idx] != *key) {
					// Key mismatch, or end of the key.  Lookup failed.
					node = 0;
					break;
				}

				// Look at next key char in this node.
				idx++;
				key++;
			}
		}
		return nullData;
	}


	///////////////////////////////////////////////////////////////////////////////
	// Determine if the given key exists as a prefix of a key in the Trie.
	///////////////////////////////////////////////////////////////////////////////
	template <class T> bool Trie<T>::FindTriePrefix(
		const unsigned char* key,
		int& length
	) {
		if (*key == 0) {
			return true;
		}

		TrieNode* node = roots[*key];
		length = 0;
		while (node != 0) {
			assert(node->c[0] == *key);
			int idx = 0;
			while (true) {
				if (idx == TrieNode::MaxCompression || node->c[idx] == 0) {
					// End of this node's string.  Is this also the end of the key?
					if (*key == 0) {
						// End of the key.  Lookup succeeded.
						return true;
					} else {
						// Keep going.  Find the next child.
						node = node->FindChild(*key);
						break;
					}
				} else if (node->c[idx] != *key) {
					// Key mismatch, or end of the key.  Lookup failed.
					return false;
				}

				// Look at next key char in this node.
				idx++;
				key++;
				length++;
			}
		}

		// Ran off the trie.
		return false;
	}


	///////////////////////////////////////////////////////////////////////////////
	// Determine if a some key in the trie is a prefix of the given key
	///////////////////////////////////////////////////////////////////////////////
	template <class T> bool Trie<T>::FindKeyPrefix(
		const unsigned char* key,
		int& length
	) {
		if (*key == 0) {
			return rootData != nullData;
		}

		TrieNode* node = roots[*key];
		length = 0;
		while (node != 0) {
			assert(node->c[0] == *key);
			int idx = 0;
			while (true) {
				if (idx == TrieNode::MaxCompression || node->c[idx] == 0) {
					// End of this node's string.  We have success if the 
					// node contains data.
					if (node->data != nullData) {
						return true;
					}
					if (*key == 0) {
						// End of the key.  Lookup failed.
						return false;
					} else {
						// Keep going.  Find the next child.
						node = node->FindChild(*key);
						break;
					}
				} else if (node->c[idx] != *key) {
					// Key mismatch, or end of the key.  Lookup failed.
					return false;
				}

				// Look at next key char in this node.
				idx++;
				key++;
				length++;
			}
		}

		// Ran off the trie without finding a key node.
		return false;
	}


	///////////////////////////////////////////////////////////////////////////////
	// Like FindKeyPrefix, but find longest prefix instead of shortest one.
	///////////////////////////////////////////////////////////////////////////////
	template <class T> bool Trie<T>::FindLongestKeyPrefix(
		const unsigned char* key,
		int& length
	) {
		if (*key == 0) {
			return rootData != nullData;
		}

		TrieNode* node = roots[*key];

		int tmpLength = 0;

		// Stores last successful match length
		length = 0;

		while (node != 0) {
			assert(node->c[0] == *key);
			int idx = 0;
			while (true) {
				if (idx == TrieNode::MaxCompression || node->c[idx] == 0) {
					// End of this node's string.  We have success if the 
					// node contains data.
					if (node->data != nullData) {
						// Found a successful prefix.
						length = tmpLength;
					}
					if (*key == 0) {
						// End of the key.
						node = 0;
						break;
					} else {
						// Keep going.  Find the next child.
						node = node->FindChild(*key);
						break;
					}
				} else if (node->c[idx] != *key) {
					// Key mismatch, or end of the key.
					node = 0;
					break;
				}

				// Look at next key char in this node.
				idx++;
				key++;
				tmpLength++;
			}
		}

		return length != 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Determine if a some key in the trie is a prefix of the given key
	///////////////////////////////////////////////////////////////////////////////
	template <class T> bool Trie<T>::FindKeyPrefix(
		const unsigned char* key,
		T& result,
		int& length
	) {
		if (*key == 0) {
			return rootData != nullData;
		}

		TrieNode* node = roots[*key];
		length = 0;
		while (node != 0) {
			assert(node->c[0] == *key);
			int idx = 0;
			while (true) {
				if (idx == TrieNode::MaxCompression || node->c[idx] == 0) {
					// End of this node's string.  We have success if the 
					// node contains data.
					if (node->data != nullData) {
						result = node->data;
						return true;
					}
					if (*key == 0) {
						// End of the key.  Lookup failed.
						return false;
					} else {
						// Keep going.  Find the next child.
						node = node->FindChild(*key);
						break;
					}
				} else if (node->c[idx] != *key) {
					// Key mismatch, or end of the key.  Lookup failed.
					return false;
				}

				// Look at next key char in this node.
				idx++;
				key++;
				length++;
			}
		}

		// Ran off the trie without finding a key node.
		return false;
	}


	///////////////////////////////////////////////////////////////////////////////
	// Like FindKeyPrefix, but find longest prefix instead of shortest one.
	///////////////////////////////////////////////////////////////////////////////
	template <class T> bool Trie<T>::FindLongestKeyPrefix(
		const unsigned char* key,
		T& result,
		int& length
	) {
		if (*key == 0) {
			return rootData != nullData;
		}

		TrieNode* node = roots[*key];

		int tmpLength = 0;

		// Stores last successful match length
		length = 0;

		while (node != 0) {
			assert(node->c[0] == *key);
			int idx = 0;
			while (true) {
				if (idx == TrieNode::MaxCompression || node->c[idx] == 0) {
					// End of this node's string.  We have success if the 
					// node contains data.
					if (node->data != nullData) {
						// Found a successful prefix.
						result = node->data;
						length = tmpLength;
					}
					if (*key == 0) {
						// End of the key.
						node = 0;
						break;
					} else {
						// Keep going.  Find the next child.
						node = node->FindChild(*key);
						break;
					}
				} else if (node->c[idx] != *key) {
					// Key mismatch, or end of the key.
					node = 0;
					break;
				}

				// Look at next key char in this node.
				idx++;
				key++;
				tmpLength++;
			}
		}

		return length != 0;
	}

}

#endif
