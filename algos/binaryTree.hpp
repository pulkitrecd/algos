//
//  binaryTree.hpp
//  algos
//
//  Created by psrivastav18 on 9/10/17.
//  Copyright © 2017 psrivastav18. All rights reserved.
//

#ifndef binaryTree_hpp
#define binaryTree_hpp

#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <assert.h>

//datatypes.

#define sizeOfPtr 8


template <typename K>
class less {
public:
    bool operator()( const K& value1, const K& value2 ) {
        return std::less<K> ( value1, value2);
    }
};

//dump c struct.
struct MemoryBlock {
    char* d_buffer;
};

static void initMemoryBlock( MemoryBlock* memBlk , std::size_t size ) {
    assert ( memBlk != 0 );
    assert ( size > sizeOfPtr + sizeof( std::size_t) );
    //###########################################################
    //0th - 8th byte -> MemoryBlock*
    //9th - sizeof(std::size_t ) ,i.e 4 bytes -> Size of block.
    //###########################################################
    if ( memBlk->d_buffer == 0 ) {
       memBlk->d_buffer = new char[ size ];
        ::memset( memBlk->d_buffer, 0, size );
        char* memblkAddress = memBlk->d_buffer;
        memBlk->d_buffer = reinterpret_cast<char*>(memBlk);
        *(memblkAddress + sizeOfPtr) = size;
    }
 
}

static uint16_t memoryBlockHeaderSize() {
    return sizeOfPtr + sizeof(std::size_t);
}

class CustomAllocator {
    
    friend CustomAllocatorTest ;
    
    typedef std::queue< MemoryBlock* > FreeList;
    typedef std::unordered_map< std::size_t, FreeList > MemoryPool_t;
    
    //#############################################
    // Block Increment Size .
    // Controls the number of blocks that would be
    // allocated if there are no more blocks
    // in the free list.
    //#############################################
    static const std::size_t k_blocksIncrementSize = 10;
    
public:
    
     char* allocate( std::size_t size );
     void deallocate( char* );
    
    CustomAllocator();
    ~CustomAllocator();
    
 private:
    CustomAllocator( const CustomAllocator& ) = delete;
    const CustomAllocator& operator=( const CustomAllocator& ) = delete;
    
    //Manipulators.
    char* getAddress( MemoryBlock* memBlk , std::size_t size );

    //data.
    MemoryPool_t d_MemoryPool;
};

inline CustomAllocator::CustomAllocator()
{
    
}

inline CustomAllocator::~CustomAllocator()
{
    
}

inline char* CustomAllocator::getAddress( MemoryBlock* memBlk , std::size_t size )
{
    size_t blockSize = size + memoryBlockHeaderSize();
    initMemoryBlock( memBlk, blockSize );
    //take the block out of the queue.
    //we have a way of getting back the block from
    //returned address.
    return reinterpret_cast<char*>(memBlk->d_buffer + memoryBlockHeaderSize());
}

inline char* CustomAllocator::allocate( std::size_t size )
{
    char* memLocation = 0 ;
    MemoryPool_t::iterator iter = d_MemoryPool.find( size );
    if ( iter != d_MemoryPool.end() ) {
        //check if the free list is empty
        if ( iter->second.size() ) {
            //there are some free memory blocks.
            //reuse them.
            MemoryBlock* memblk = iter->second.front();
            memLocation = getAddress( memblk , size );
            iter->second.pop();
        }
        else {
            //all the blocks have been taken.
            //allocate fresh ones.
            MemoryBlock* memBlock_ =  new MemoryBlock[k_blocksIncrementSize];
            ::memset(memBlock_,0,sizeof(MemoryBlock)*k_blocksIncrementSize);
            for( std::size_t i = 0 ; i < k_blocksIncrementSize ; ++i  ) {
                iter->second.push( memBlock_ + i );
            }
            MemoryBlock* memblk = iter->second.front();
            memLocation = getAddress( memblk , size );
            iter->second.pop();
        }
    }
    else {
     
        //there has never been an allocation of this
        //size.
        
        MemoryBlock* memBlock_ =  new MemoryBlock[k_blocksIncrementSize];
        ::memset(memBlock_, 0, sizeof(MemoryBlock)* k_blocksIncrementSize );
        FreeList& freeList = d_MemoryPool[size];
        for( std::size_t i = 0 ; i < k_blocksIncrementSize ; ++i  ) {
            freeList.push( memBlock_ + i );
        }
        MemoryBlock* memblk = freeList.front();
        memLocation = getAddress( memblk , size );
        freeList.pop();
    }
    return memLocation;
}

void CustomAllocator::deallocate( char* ptr ) {
    if ( ptr == 0 ) {
        return ;
    }
    uint16_t size = *(reinterpret_cast<uint16_t*>( ptr - sizeof(  std::size_t ) ) );
    //get the memory block.
    MemoryBlock* memBlock_ = reinterpret_cast<MemoryBlock*>(ptr - memoryBlockHeaderSize());
    //based on size get the freeList.
    d_MemoryPool[size].push(memBlock_);
}


/*

template < typename K, typename V, typename A = CustomAllocator ,typename Comp = less<K> >
class BinaryTree {
    
    typedef K key_type;
    typedef V value_type;
    
    struct Pair {
        K d_First;
        V d_Value;
    };
    
private:
    struct Node {
        //owns the pair.
        K d_Key;
        Pair* d_Pair = 0;
        BinaryTree::Node* d_Left = 0;
        BinaryTree::Node* d_Right = 0;
        
        ~Node();
    };
    
public:
    struct iterator {
        
        friend BinaryTree;
        
    private:
        //does the own the reference.
        Node* d_Handle = 0;
        
        void reset( Node* node );
        
    public:
        //default constructor
        //An iterator not holding a reference to
        //an actual node* is a valid iterator.
        iterator() ;
        iterator( const iterator& );
        iterator& operator=( const iterator& );
        ~iterator(); */
        
      //  Pair& operator*();
      //   Pair* operator->();

/*
        operator bool();
        
        iterator operator++(); //pre-increment operator.
        iterator operator++(int x); //post-increament operator.
        
        iterator operator--(); //pre decrement operator.
        iterator operator--( int x ); //post decrement operator.
        
        //statics.
        static bool isValid();
    };
    
private:
    
    iterator  d_root;
    Comp      d_comp;
    
private :

    //Manipulators.
    
    void deepCopy( Node* node ) const;
    
    
    iterator insertInternal( const Pair&  p );
    iterator eraseInternal( const K& key );
    iterator findInternal ( const K& key );
    
public:
    
    //Creators
    BinaryTree();
    
    virtual ~BinaryTree();
    
    BinaryTree( const BinaryTree& tree );
    
    const BinaryTree& operator=( const BinaryTree& tree );
    
    iterator begin();
    
    iterator end();
    
    //Manipulators.
    
    iterator insert( const Pair& value );
    
    iterator erase( const K& key );

    void clear();
    
    //Accessors
    
    iterator find( K& value );
    
    V& operator[]( K& value );
    
};

template <typename K, typename V, typename Comp >
inline BinaryTree<K,V,Comp>::BinaryTree()
{
    Comp c;
    d_comp = c;
};

template <typename K,typename V, typename Comp >
inline BinaryTree<K,V,Comp>::BinaryTree( const BinaryTree& bt )
{
    d_comp = bt.d_comp;
    if ( ! bt.d_root.isValid() ) {
        return;
    }
    try {
        Node n = new Node();
        bt.deepCopy(n);
        d_root.reset(n);
    } catch (...) {
        std::cerr << "exception creating binary search tree." << std::endl;
        throw;
    }
}

template <typename K,typename V, typename Comp >
inline const BinaryTree<K,V,Comp>& BinaryTree<K,V,Comp>::operator=(const BinaryTree<K, V, Comp> &bt)
{
    if ( this == &bt ) {
        return *this;
    }
    
    d_comp = bt.d_comp;
    if ( ! bt.d_root ) {
        clear();
        return *this;
    }
    try {
        Node n = new Node();
        bt.deepCopy(n);
        d_root.reset(n);
    } catch (...) {
        std::cerr << "exception creating binary search tree." << std::endl;
        throw;
    }
    
    return *this;

}

template <typename K, typename V, typename Comp>
inline BinaryTree<K,V,Comp>::~BinaryTree()
{
    try {
        clear();
        d_root = 0;
    } catch(...) {
        
    }
}

template <typename K , typename V, typename Comp>
inline BinaryTree<K,V,Comp>::Node::~Node()
{
    ~d_Pair();
}

template <typename K , typename V, typename Comp>
inline typename BinaryTree<K,V,Comp>::iterator BinaryTree<K,V,Comp>::begin()
{
    return d_root;
}

template <typename K , typename V, typename Comp>
inline typename BinaryTree<K,V,Comp>::iterator BinaryTree<K,V,Comp>::end()
{
    iterator end_iter;
    return end_iter;
}

template <typename K , typename V, typename Comp>
inline typename BinaryTree<K,V,Comp>::iterator BinaryTree<K,V,Comp>::insert(const Pair& pair )
{
    const key_type& k = pair.d_First;
    const value_type& v = pair.d_Value;
    
    iterator iter = insertInternal( pair );
    return iter;
}

template <typename K , typename V, typename Comp>
inline typename BinaryTree<K,V,Comp>::iterator BinaryTree<K,V,Comp>::insertInternal(const Pair& pair )
{
    const key_type& k = pair.d_First;
    const value_type& v = pair.d_Value;
    
    if ( ! d_root ) {
        //the tree does not have a valid root
        char* buffer = new char[ sizeof( Node ) + 1 + sizeof( Pair )  ];
        d_root.reset( new (buffer)Node );
        d_root->d_Key = k;
        d_root->d_Pair = new ( buffer + sizeof(Node) + 1 )Pair( k, v );
        
    }
    
} */


#endif /* binaryTree_hpp */
