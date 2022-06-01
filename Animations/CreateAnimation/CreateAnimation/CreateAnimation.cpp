// CreateAnimation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "stb_image.h"
#include <assert.h>
#include <string>
#include <fstream>
#include <ostream>

int main()
{
    int h, w;
    int nameSize = 0;
    std::cout << "Press 0 to batch and 1 (other) to load file by file. \n";
    int batch = 0;
    std::cin >> batch;
    int batchPlaces = 4;
    batch = !batch; //:(
    if (batch) {
        std::cout << "How many places will the index of the file have?\n";
        std::cin >> batchPlaces;
    }
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

    //REDO USING fstream.write!
    std::string outFileStr = std::string("../../AnimationOut/ANIMATE_").append(name).append(".txt");
    std::ofstream fstream;
    fstream.open(outFileStr, std::ios::out | std::ios::binary);

    std::cout << "Created output file " << outFileStr << std::endl;



    //Write this data
    //Name size
    std::string writeStr = std::to_string(nameSize);
    fstream.write(writeStr.c_str(),writeStr.size());
    fstream.write("\n", 1);
    //Name
    fstream.write(name.c_str(), name.size() );
    fstream.write("\n", 1 );
    //Width
    writeStr = std::to_string(w);
    fstream.write(writeStr.c_str(), writeStr.size() );
    fstream.write("\n", 1 );
    //Height
    writeStr = std::to_string(h);
    fstream.write(writeStr.c_str(), writeStr.size() );
    fstream.write("\n", 1 );
    //Num layers
    writeStr = std::to_string(numLayers);
    fstream.write(writeStr.c_str(), writeStr.size() );
    fstream.write("\n", 1 );
    //Num frames
    writeStr = std::to_string(numFrames);
    fstream.write(writeStr.c_str(), writeStr.size() );
    fstream.write("\n", 1 );
    //Frame time
    writeStr = std::to_string(numFPS);
    fstream.write(writeStr.c_str(), writeStr.size() );
    fstream.write("\n", 1 );

    for (int l = 0; l < numLayers; l++) {
        std::cout << "LAYER: " << l << std::endl;
        std::cout << "What material is this layer?\n";
        int mat;
        std::cin >> mat;
        std::cout << "Material is " << mat << std::endl;

        writeStr = std::to_string(mat);
        fstream.write(writeStr.c_str(), writeStr.size() );
        fstream.write("\n", 1 );
        
        //Write this data

        if (!batch) {

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
                    fstream.close();
                    return 1;
                }
                assert(data);
                //Each channel is 8 bits, so 1 byte per channel.
                int sizeofTex = nrChannels * height * width;
                if (nrChannels < 4) {
                    std::cout << "ERROR: Data should be formatted with at least RGBA\n";
                    fstream.close();
                    return 1;
                }
                if (w != width || h != height) {
                    std::cout << "ERROR: Width and height do not match.\n";
                    fstream.close();
                    return 1;
                }
                assert(w == width && h == height);
                std::cout << "Data loaded succesfully. Texture size " << sizeofTex << std::endl;

                //Write texture size

                writeStr = std::to_string(sizeofTex);
                fstream.write(writeStr.c_str(), writeStr.size());
                fstream.write("\n", 1);

                //Write texture
                fstream.write((const char*)data, sizeofTex);
                fstream.write("\n", 1);
            }
        }
        else {

            std::cout << "Please enter the batch name\n";
            std::string fileBase;
            std::cin >> fileBase;

            for (int f = 0; f < numFrames; f++) {
                std::cout << "LAYER, FRAME: " << l << ", " << f << std::endl;
                std::string frameStr = std::to_string(f);
                std::cout << "Test: " << frameStr << std::endl;
                int frameSize = frameStr.size();
                for (int i = 0; i < batchPlaces - frameSize; i++) {
                    frameStr = std::string("0").append(frameStr);
                    std::cout << "after i = " << i << " " << frameStr << std::endl;
                }
                std::string fileExtend = std::string(fileBase.c_str());
                fileExtend = fileExtend.append(frameStr);
                std::string file = std::string("../../Textures/").append(fileExtend).append(".png");
                std::cout << "Loading " << file << std::endl;
                int width, height, nrChannels;
                unsigned char* data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
                if (data == NULL) {
                    std::cout << "ERROR: Image load failed\n";
                    fstream.close();
                    return 1;
                }
                assert(data);
                //Each channel is 8 bits, so 1 byte per channel.
                int sizeofTex = nrChannels * height * width;
                if (nrChannels < 4) {
                    std::cout << "ERROR: Data should be formatted with at least RGBA\n";
                    fstream.close();
                    return 1;
                }
                if (w != width || h != height) {
                    std::cout << "ERROR: Width and height do not match.\n";
                    fstream.close();
                    return 1;
                }
                assert(w == width && h == height);
                std::cout << "Data loaded succesfully. Texture size " << sizeofTex << std::endl;

                //Write texture size

                writeStr = std::to_string(sizeofTex);
                fstream.write(writeStr.c_str(), writeStr.size());
                fstream.write("\n", 1);

                //Write texture
                fstream.write((const char*)data, sizeofTex);
                fstream.write("\n", 1);
            }

        }

    }
    std::cout << "Animation written\n";
    fstream.close();
    return 0;
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
