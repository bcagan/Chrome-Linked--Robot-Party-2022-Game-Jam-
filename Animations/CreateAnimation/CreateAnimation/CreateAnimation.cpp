// CreateAnimation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "stb_image.h"
#include <assert.h>
#include <string>
#include <fstream>

int main()
{
    int h, w;
    int nameSize = 0;
    std::cout << "What is the size of this animation's name?\n";
    std::cin >> nameSize;
    std::string name;
    std::cout << "What is the name of this animation? No spaces.\n";
    std::cin >> name;
    std::cout << "What is the width of this sprite?\n";
    std::cin >> w;
    std::cout << "What is the height of this sprite?\n";
    std::cin >> h;
    int numLayers;
    std::cout << "How many layers is each frame?\n";
    std::cin >> numLayers;
    int numFrames;
    std::cout << "How many frames is this animation?\n";
    std::cin >> numFrames;
    int numFPS;
    std::cout << "How many frames per second is a single frame of animation?\n";
    std::cin >> numFPS;

    std::cout << "Pre data info: Name " << name << " (" << nameSize << ") " << w << "x" << h << " " << numLayers << " layers " << numFrames << " frames (" << numFPS << ")\n";

    //Create file
    std::string outFileStr = std::string("../../AnimationOut/ANIMATE_").append(name).append(".txt");
    FILE* outFile;
    fopen_s(&outFile, outFileStr.c_str(), "w");
    if (outFile == NULL) {
        std::cout << ("ERROR: File openning failed\n");
        assert(outFile);
    }

    std::cout << "Created output file " << outFileStr << std::endl;



    //Write this data
    //Name size
    std::string writeStr = std::to_string(nameSize);
    fwrite(writeStr.c_str(), sizeof(char), writeStr.size(), outFile);
    fwrite("\n", sizeof(char), 1, outFile);
    //Name
    fwrite(name.c_str(), sizeof(char), name.size(), outFile);
    fwrite("\n", sizeof(char), 1, outFile);
    //Width
    writeStr = std::to_string(w);
    fwrite(writeStr.c_str(), sizeof(char), writeStr.size(), outFile);
    fwrite("\n", sizeof(char), 1, outFile);
    //Height
    writeStr = std::to_string(h);
    fwrite(writeStr.c_str(), sizeof(char), writeStr.size(), outFile);
    fwrite("\n", sizeof(char), 1, outFile);
    //Num layers
    writeStr = std::to_string(numLayers);
    fwrite(writeStr.c_str(), sizeof(char), writeStr.size(), outFile);
    fwrite("\n", sizeof(char), 1, outFile);
    //Num frames
    writeStr = std::to_string(numFrames);
    fwrite(writeStr.c_str(), sizeof(char), writeStr.size(), outFile);
    fwrite("\n", sizeof(char), 1, outFile);
    //Frame time
    writeStr = std::to_string(numFPS);
    fwrite(writeStr.c_str(), sizeof(char), writeStr.size(), outFile);
    fwrite("\n", sizeof(char), 1, outFile);

    for (int l = 0; l < numLayers; l++) {
        std::cout << "LAYER: " << l << std::endl;
        std::cout << "What material is this layer?\n";
        int mat;
        std::cin >> mat;
        std::cout << "Material is " << mat << std::endl;

        writeStr = std::to_string(mat);
        fwrite(writeStr.c_str(), sizeof(char), writeStr.size(), outFile);
        fwrite("\n", sizeof(char), 1, outFile);
        
        //Write this data

        for (int f = 0; f < numFrames; f++) {
            std::cout << "LAYER, FRAME: " << l << ", " << f << std::endl;
            std::cout << "Please give the file name. File dir should be ../Textures/[INPUT]\n";
            std::string fileExtend;
            std::cin >> fileExtend;
            std::string file = std::string("../../Textures/").append(fileExtend);
            std::cout << "Loading " << file << std::endl;
            int width, height, nrChannels;
            unsigned char* data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
            if (data == NULL) {
                std::cout << "ERROR: Image load failed\n";
                fclose(outFile);
            }
            assert(data);
            //Each channel is 8 bits, so 1 byte per channel.
            int sizeofTex = nrChannels * height * width;
            if (w != width || h != height) {
                std::cout << "ERROR: Width and height do not match.\n";
                fclose(outFile);
            }
            assert(w == width && h == height);
            std::cout << "Data loaded succesfully. Texture size " << sizeofTex << std::endl;

            //Write texture size

            writeStr = std::to_string(sizeofTex);
            fwrite(writeStr.c_str(), sizeof(char), writeStr.size(), outFile);
            fwrite("\n", sizeof(char), 1, outFile);

            //Write texture
            fwrite(data, sizeof(char), sizeofTex, outFile);
            fwrite("\n", sizeof(char), 1, outFile);
        }

    }
    std::cout << "Animation written\n";
    fclose(outFile);
}

//Animation data format
//Name size
//Name
//Width
//Height
//Num layers
//Num frames
//Frame time
//Layers

//Layer:
//  Material
//  Textures

//Texture
    //Size
    //Texture data

//Example
/*
4
"Test"
1280
720
2
3
2
0
sizeof(Texture0)
TextureData0
sizeof(Texture1)
TextureData1
sizeof(Texture2)
TextureData2
1
sizeof(Texture0)
TextureData0
sizeof(Texture1)
TextureData1
sizeof(Texture2)
TextureData2*/
