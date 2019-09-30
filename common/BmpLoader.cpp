//
// Created by 고선재 on 2019-09-30.
//

#include "BmpLoader.hpp"


vector<unsigned int> BmpLoader::loadBmp( string imagepath )
{
    vector<unsigned int> result;

    unsigned char header[54]; // Each BMP file begins by a 54-bytes header
    unsigned int dataPos;     // Position in the file where the actual data begins
    unsigned int width, height;
    unsigned int imageSize;   // = width*height*3

    // Actual RGB data
    unsigned char *data;

    FILE *file = fopen( imagepath.c_str(), "rb" );
    if( !file )
    {
        printf( "Image could not be opened\n" );
        assert( false );
    }

    if( fread( header, 1, 54, file ) != 54 )
    {
        // If not 54 bytes read : problem
        printf( "Not a correct BMP file\n" );
        assert( false );
    }

    if( header[0] != 'B' || header[1] != 'M' )
    {
        printf( "Not a correct BMP file\n" );
        assert( false );
    }

    dataPos = *( int * ) &( header[0x0A] );
    imageSize = *( int * ) &( header[0x22] );
    width = *( int * ) &( header[0x12] );
    height = *( int * ) &( header[0x16] );

    if( imageSize == 0 )
    {
        imageSize = width * height * 3; // 3 : one byte for each Red, Green and Blue component
    }
    if( dataPos == 0 )
    {
        dataPos = 54; // The BMP header is done that way
    }
    result.resize( imageSize );
    fread( result.data(), 1, imageSize, file );
    fclose( file );

    return result;
}