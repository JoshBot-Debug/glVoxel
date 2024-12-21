#pragma once

#include <unordered_map>
#include <tuple>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Types.h"
#include "Debug.h"

class OBJLoader
{
private:
  std::ifstream file;
  std::string filepath;

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texCoords;
  std::vector<glm::vec3> normals;

  std::unordered_map<Vertex, unsigned int, Vertex, Vertex> uniqueVertices;

  void read(std::vector<Mesh> &meshes)
  {
    std::string line;

    while (std::getline(file, line))
    {
      if (line.starts_with("#"))
        continue;

      std::istringstream iss(line);
      std::string command, word;
      iss >> command;

      // if (command == "mtllib")
      //   while (iss >> word)
      //     const std::string path = (filepath.substr(0, filepath.find_last_of('/')) + "/" + text);

      if (command == "o")
      {
        iss >> word;
        meshes.emplace_back(word);
      }

      if (command == "v")
      {
        unsigned int index = 0;
        glm::vec3 &position = positions.emplace_back();
        while (iss >> word)
        {
          position[index] = std::stof(word);
          ++index;
        }
      }

      if (command == "vt")
      {
        unsigned int index = 0;
        glm::vec2 &texCoord = texCoords.emplace_back();
        while (iss >> word)
        {
          texCoord[index] = std::stof(word);
          ++index;
        }
      }

      if (command == "vn")
      {
        unsigned int index = 0;
        glm::vec3 &normal = normals.emplace_back();
        while (iss >> word)
        {
          normal[index] = std::stof(word);
          ++index;
        }
      }

      // if (command == "g")
      //   g(word);

      // if (command == "usemtl")
      //   usemtl(word);

      // if (command == "s")
      //   s(word);

      if (command == "f")
      {
        std::vector<glm::ivec3> faces;

        while (iss >> word)
        {
          glm::ivec3 &face = faces.emplace_back();
          getFace(face, word);
        }

        processFaces(meshes, faces);
      }
    }
  }

  void processFaces(std::vector<Mesh> &meshes, std::vector<glm::ivec3> faces)
  {
    Mesh &mesh = meshes.back();

    unsigned int vIndexOffset = 0;

    for (size_t i = 0; i < meshes.size() - 1; i++)
      vIndexOffset += meshes[i].vertices.size();

    if (faces.size() == 4)
      faces = {faces[0], faces[1], faces[2], faces[0], faces[2], faces[3]};

    for (const auto &face : faces)
    {
      Vertex vertex;
      vertex.position = positions[face[0]];
      vertex.texCoord = texCoords[face[1]];
      vertex.normal = normals[face[2]];

      auto it = uniqueVertices.find(vertex);

      if (it == uniqueVertices.end())
      {
        unsigned int index = static_cast<unsigned int>(vIndexOffset + mesh.vertices.size());
        mesh.vertices.push_back(vertex);
        mesh.indices.push_back(index);
        uniqueVertices[vertex] = index;
      }
      else
        mesh.indices.push_back(it->second);
    }
  }

  void getFace(glm::ivec3 &face, const std::string &text)
  {
    std::stringstream ss(text);
    std::string token;
    std::vector<std::string> tokens;

    unsigned int index = 0;

    while (std::getline(ss, token, '/'))
    {
      face[index] = std::stoi(token) - 1;
      ++index;
    }
  }

public:
  OBJLoader(const std::string &filepath, std::vector<Mesh> &meshes) : file(std::ifstream(filepath, std::ios::binary)), filepath(filepath)
  {
    if (!file)
    {
      LOG_BREAK_BEFORE;
      LOG("Failed to read obj file:", filepath);
      LOG_BREAK_AFTER;
    }

    read(meshes);
  }

  ~OBJLoader()
  {
    file.close();
  }
};