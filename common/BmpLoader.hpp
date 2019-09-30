//
// Created by 고선재 on 2019-09-30.
//

#ifndef TUTORIALS_BMPLOADER_HPP
#define TUTORIALS_BMPLOADER_HPP

#include <vector>
#include <string>

using namespace std;

class BmpLoader
{
public:
    static vector<unsigned int> loadBmp( string imagePath );
};


#endif //TUTORIALS_BMPLOADER_HPP
