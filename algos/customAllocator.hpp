//
//  customAllocator.hpp
//  algos
//
//  Created by psrivastav18 on 9/16/17.
//  Copyright © 2017 psrivastav18. All rights reserved.
//

#ifndef customAllocator_h
#define customAllocator_h


#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <assert.h>

#define sizeOfPtr sizeof(void*)

#define  EXPECT_EQ( c, x, y ) { \
    int32_t ret = 0;\
    ret = (x == y) ? 0 : 1; \
    if( ret ) {\
       std::cerr << "check " << #c << " failed, expected " << x << " actual " << y << std::endl;\
       return ret;\
    }\
} \

namespace n_memory {
    
    //dump c struct.
    struct MemoryBlock {
        char* d_buffer;
    };
    
    static std::size_t memoryBlockHeaderSize() {
        return sizeOfPtr + sizeof(std::size_t);
    }
    
    static std::size_t blockSize( char* buffer ) {
        if ( ! buffer ) {
            return 0;
        }
        std::size_t size = 0;
        ::memcpy(&size, buffer - sizeof(  std::size_t ) , sizeof( std::size_t ));
        return size;
    }
    
    static MemoryBlock* blockAddress( char* buffer ) {
        if ( ! buffer ) {
            return 0;
        }
        MemoryBlock* memBlock_ = 0;
        ::memcpy( &memBlock_, buffer - memoryBlockHeaderSize() , sizeof(MemoryBlock*) );
        return memBlock_;
    }

    static void initMemoryBlock( MemoryBlock* memBlk , std::size_t size ) {
        assert ( memBlk != 0 );
        assert ( size > memoryBlockHeaderSize() );
        //###########################################################
        //0th - 8th byte -> MemoryBlock*
        //9th - sizeof(std::size_t ) ,i.e 4 bytes -> Size of block.
        //###########################################################
        if ( memBlk->d_buffer == 0 ) {
            memBlk->d_buffer = new char[ size ];
            char* bufferTemp = memBlk->d_buffer;
           // memBlk->d_buffer = reinterpret_cast<char*>(memBlk);
            
            //*(reinterpret_cast<std::size_t*>(memblkAddress + sizeOfPtr)) = size;
            
            void* memBlkAddress = memBlk;
            ::memcpy( bufferTemp, &memBlkAddress, sizeof(void*) );
            ::memcpy( bufferTemp + sizeOfPtr, &size, sizeof(std::size_t) );
        } else {
            ::memset(memBlk->d_buffer + memoryBlockHeaderSize() , 0 , size - memoryBlockHeaderSize() );
        }
    }
    
    static void deinitMemoryBlock( MemoryBlock* memBlk )
    {
        assert( memBlk != 0 );
        if ( memBlk->d_buffer ) {
            delete[] memBlk->d_buffer;
            memBlk->d_buffer = 0;
        }
    }
    
    class CustomAllocator;
    
    namespace test {
        
        class CustomAllocatorTest {
            
        public:
            
            static int32_t testMemoryAlignment( char* buffer, std::size_t size , n_memory::CustomAllocator* allocator);
            static  int32_t testAllocation();
            static  int32_t testDeallocation();
        };
        
    }
    
    class CustomAllocator {
        
        typedef std::deque< MemoryBlock* > FreeList_t;
        typedef std::unordered_map< std::size_t, FreeList_t > MemoryPool_t;
        
        
        //friends.
        friend test::CustomAllocatorTest;
        
        
    public:
        
        char* allocate( std::size_t size );
        void deallocate( char* );
        
        void setBlockIncrementSize( std::size_t blockIncrementSize );
    
        CustomAllocator();
        ~CustomAllocator();
        
    private:
        CustomAllocator( const CustomAllocator& ) = delete;
        const CustomAllocator& operator=( const CustomAllocator& ) = delete;
        
        //Manipulators.
        char* getAddress( MemoryBlock* memBlk , std::size_t size );
        void clear();
        
        //data.
        MemoryPool_t d_MemoryPool;
        std::vector<MemoryBlock*> d_MemoryBlocks;

        std::size_t d_blocksIncrementSize = 10;
        //#############################################
        // Block Increment Size .
        // Controls the number of blocks that would be
        // allocated if there are no more blocks
        // in the free list.
        //#############################################

    };
    
    inline CustomAllocator::CustomAllocator()
    {
        
    }
    
    inline CustomAllocator::~CustomAllocator()
    {
        clear();
    }
    
    inline void CustomAllocator::clear()
    {
        //clean up all the memory.
        MemoryPool_t::iterator iter = d_MemoryPool.begin(),
        iter_end = d_MemoryPool.end();
        for ( ; iter != iter_end ; ++iter ) {
            while( iter->second.size() ) {
                MemoryBlock* ptr = iter->second.front();
                deinitMemoryBlock(ptr);
                iter->second.pop_front();
            }
        }
        
        std::vector<MemoryBlock*>::iterator blks = d_MemoryBlocks.begin(),
        blks_end = d_MemoryBlocks.end();
        
        for ( ; blks != blks_end ; ++blks ) {
            delete[] (*blks);
        }
    
        d_MemoryPool.clear();
    
    }
    
    
    inline void CustomAllocator::setBlockIncrementSize(std::size_t blockIncrementSize) {
        
        d_blocksIncrementSize = blockIncrementSize;
        
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
                iter->second.pop_front();
            }
            else {
                //all the blocks have been taken.
                //allocate fresh ones.
                MemoryBlock* memBlock_ =  new MemoryBlock[d_blocksIncrementSize];
                ::memset(memBlock_,0,sizeof(MemoryBlock)*d_blocksIncrementSize);
                d_MemoryBlocks.push_back(memBlock_);
                for( std::size_t i = 0 ; i < d_blocksIncrementSize ; ++i  ) {
                    iter->second.push_back( memBlock_ + i );
                }
                MemoryBlock* memblk = iter->second.front();
                memLocation = getAddress( memblk , size );
                iter->second.pop_front();
            }
        }
        else {
            
            //there has never been an allocation of this
            //size.
            MemoryBlock* memBlock_ =  new MemoryBlock[d_blocksIncrementSize];
            ::memset(memBlock_, 0, sizeof(MemoryBlock)* d_blocksIncrementSize );
            d_MemoryBlocks.push_back(memBlock_);
            FreeList_t& freeList = d_MemoryPool[size];
            for( std::size_t i = 0 ; i < d_blocksIncrementSize ; ++i  ) {
                freeList.push_back( memBlock_ + i );
            }
            MemoryBlock* memblk = freeList.front();
            memLocation = getAddress( memblk , size );
            freeList.pop_front();
        }
        return memLocation;
    }
    
    void CustomAllocator::deallocate( char* ptr ) {
        
        if ( ptr == 0 ) {
            return ;
        }
        
        std::size_t size = blockSize(ptr);
        if  ( size <= 0  )
            return ;
        
        MemoryBlock* memBlock = blockAddress(ptr);
        if ( ! memBlock )
            return ;
        
        //based on size get the freeList.
        d_MemoryPool[size].push_back(memBlock);
    }
    
    namespace test {
        
        
        //int32_t CustomAllocatorTest::testMemoryAlignment(<#char *buffer#>, <#std::size_t size#>)
       /* {
            std::size_t sizeFromBuffer = *(buffer - sizeof(std::size_t));
            EXPECT_EQ("compare size from buffer", sizeFromBuffer, size );
            return 0;
        } */
        
        
        int32_t CustomAllocatorTest::testMemoryAlignment( char* buffer, std::size_t size , n_memory::CustomAllocator* allocator)
        {
            std::size_t sizeFromBuffer  = blockSize(  buffer );
            EXPECT_EQ("compare size from buffer", size, sizeFromBuffer );
            
            MemoryBlock* memBlock = blockAddress( buffer );
            //check the memory block exists in the allocator.
            
            n_memory::CustomAllocator::MemoryPool_t& memPool = allocator->d_MemoryPool;
            n_memory::CustomAllocator::MemoryPool_t::iterator iter = memPool.find(sizeFromBuffer);
            
            if ( iter == memPool.end() )
                return 1;
            
            return 0;
        }
        
        
        int32_t CustomAllocatorTest::testAllocation()
        {
            CustomAllocator c_test;
            c_test.setBlockIncrementSize(1);
            std::size_t size = 10;
            char* mem_ = c_test.allocate(size);
            
            //test the internal of the allocator.
            int32_t retcode = 0;
            retcode = CustomAllocatorTest::testMemoryAlignment( mem_, size , &c_test );
            EXPECT_EQ( "blockHeaders1", retcode, 0);
            
            CustomAllocator::MemoryPool_t& memPool = c_test.d_MemoryPool;
            std::size_t queueCount = 0;
            queueCount = memPool.count(size);
            EXPECT_EQ("queueCount1",queueCount,1);
            
            std::size_t freeListCount = 0;
            freeListCount = memPool.size();
            EXPECT_EQ("memPoolFreeListCount",freeListCount,1);
            
            ::memcpy(mem_,"TESTABCDEF", strlen("TESTABCDEF"));
            int32_t cmp = ::memcmp(mem_,"TESTABCDEF",strlen("TESTABCDEF"));
            EXPECT_EQ("memory write mem1", cmp, 0 );
            
            //check the number of blocks in the freeList.
            CustomAllocator::MemoryPool_t::iterator iter = memPool.find( size );
            EXPECT_EQ("RemainigFreeBlocks",0,iter->second.size());
            
            //allocate another 100 bytes.
            //should realloacte 100 bytes block.
            char* mem2_ = c_test.allocate(size);
            retcode = CustomAllocatorTest::testMemoryAlignment( mem2_ , size , &c_test );
            EXPECT_EQ( "blockHeaders-Reallocation", retcode, 0);
            
            ::memcpy(mem2_,"TESTABCDEF", strlen("TESTABCDEF"));
            cmp = ::memcmp(mem2_,"TESTABCDEF",strlen("TESTABCDEF"));
            EXPECT_EQ("memory write mem2", cmp, 0 );
            
            size = 5;
            char* mem3_ = c_test.allocate(size);
            //test the internal of the allocator.
            retcode = 0;
            retcode = CustomAllocatorTest::testMemoryAlignment( mem3_, size , &c_test );
            EXPECT_EQ( "blockHeaders", retcode, 0);
            
            queueCount = memPool.count(size);
            EXPECT_EQ("queueCount",queueCount,1);
            
            freeListCount = memPool.size();
            EXPECT_EQ("memPoolFreeListCount",freeListCount,2);
            
            //check the number of blocks in the freeList.
            iter = memPool.find( size );
            EXPECT_EQ("RemainingFreeBlocks - size 5",0,iter->second.size());
            
            ::memcpy(mem3_,"TESTA", strlen("TESTA"));
            cmp = ::memcmp(mem3_,"TESTA",strlen("TESTA"));
            EXPECT_EQ("memory write mem3", cmp, 0 );
            
            c_test.deallocate( mem_ );
            iter = memPool.find( 10 );
            EXPECT_EQ("RemainigFreeBlocks size 10 post release 1 block ",1,iter->second.size());
            
            c_test.deallocate( mem2_ );
            iter = memPool.find( 10 );
            EXPECT_EQ("RemainigFreeBlocks size 10 post release 2 blocks ",2,iter->second.size());
            
            
            size = 10;
            mem_ = c_test.allocate(size);
            
            retcode = 0;
            retcode = CustomAllocatorTest::testMemoryAlignment( mem_, size , &c_test );
            EXPECT_EQ( "blockHeaders", retcode, 0);
            
            ::memcpy(mem_,"TESTABCDEF", strlen("TESTABCDEF"));
             cmp = ::memcmp(mem_,"TESTABCDEF",strlen("TESTABCDEF"));
            EXPECT_EQ("memory write mem1", cmp, 0 );
            
            iter = memPool.find( size );
            EXPECT_EQ("RemainingFreeBlocks - size 10 ",1,iter->second.size());
            
            return 0;
        }
        
        int32_t CustomAllocatorTest::testDeallocation()
        {
            
            
            
            return 0;
        }
        
    }
    
}

#endif /* customAllocator_h */
