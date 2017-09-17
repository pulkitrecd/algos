//
//  binaryTree.cpp
//  algos
//
//  Created by psrivastav18 on 9/10/17.
//  Copyright © 2017 psrivastav18. All rights reserved.
//

#include "customAllocator.hpp"

int main()
{
    
    n_memory::CustomAllocator c;
    char* ptr = c.allocate(100);
    
    /*
    printf( "adress %p ", ptr );
    ptr[0] = 'a';
    printf(" ptr[0] %c" , ptr[0]);
    
    ptr[1] = 'b';
    printf(" ptr[1] %c" , ptr[1]);
    
    ptr[2] = 'c';
    printf(" ptr[2] %c" , ptr[2]);
    
    ptr[3] = 'd';
    printf(" ptr[3] %c" , ptr[3]);
    
    ptr[4] = 'e';
    printf(" ptr[4] %c" , ptr[4]); */
    
    ::memset( ptr, 0, 100 );
    
    ::memcpy( ptr, "Richa Garg", strlen("Richa Garg") );
    
    std::cerr << "content : " << ptr << std::endl;
    
    c.deallocate(ptr);
    
    /*
    int32_t result = n_memory::test::CustomAllocatorTest::testAllocation();
    EXPECT_EQ("TEST ALLOCATION", result , 0 ); */
    
    
}
