//
//  main.cpp
//  algos
//
//  Created by psrivastav18 on 9/10/17.
//  Copyright © 2017 psrivastav18. All rights reserved.
//

#include <iostream>

#include <vector>
#include <string>

using namespace std;

class Solution {
public:
    static string convert(string s, int numRows) {
        std::vector< std::string > strings_(  numRows );
        
        std::size_t index_ = 0;
        
        //for numRows ==3 , we just need to print 0,1,2,1,0,1,2
        
        while( index_ < s.length() ) {
            int queue_ = 0;
            for ( int i = 0 ; i < numRows ; ++i ) {
                queue_ = i;
                strings_[queue_].push_back(s[index_++]);
            }
            for ( int i = numRows  - 2 ; i > 0 ; --i ) {
                queue_ = i;
                strings_[queue_].push_back(s[index_++]);
            }
        }
        
        std::string response_ ;
        for ( auto i = 0 ; i < numRows ; ++i ) {
            response_ += strings_[i];
        }
        
        std::cerr << "response " << response_ << std::endl;
        
        return response_;
    }
};


/*
int main(int argc, const char * argv[]) {
    // insert code here...
    Solution::convert("PAYPALISHIRING", 3);
    return 0;
} */
