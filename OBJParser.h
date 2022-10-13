#pragma once


#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "EMath.h"
#include "Primitive.h"

namespace Elite
{

	//https://stackoverflow.com/questions/21120699/c-obj-file-parser
	static void ParseOBJ(const std::string& filename, std::vector<Vertex_Input>& vertices, std::vector<uint32_t>& indices)
	{
		std::vector<FPoint3> positions{};
		std::vector<FVector2> uvs{};
		std::vector<FVector3> normals{};
		int idx{};
		// Open the file
		std::ifstream obj(filename.c_str());
		if (!obj) {
			std::cerr << "Cannot open " << filename << std::endl;
			exit(1);
		}

		//Clear vectors from any data that might already be inside
		vertices.clear();
		indices.clear();

		//Read lines from file
		std::string line;
		while (std::getline(obj, line))
		{
			//check 'v'
			if (line.substr(0, 2) == "v ")
			{
				float x, y, z;
				const char* chh = line.c_str();
				sscanf_s(chh, "v %f %f %f", &x, &y, &z);
				//std::cout << x << "  " << y << "  " << z << std::endl;
				FPoint3 vert = FPoint3{ x, y, -z };
				positions.push_back(vert);
			}
			//check 'vn' for normals
			else if (line.substr(0, 2) == "vn")
			{
				float n1, n2, n3;
				const char* chh = line.c_str();
				sscanf_s(chh, "vn %f %f %f", &n1, &n2, &n3);
				FVector3 normal{ FVector3{n1, n2, -n3} };
				normals.push_back(normal);
			}
			//check 'vt' for uv's
			else if (line.substr(0, 2) == "vt")
			{
				float t1, t2;
				const char* chh = line.c_str();
				sscanf_s(chh, "vt %f %f", &t1, &t2);
				//std::cout << t1 << "  " << t2 << std::endl;
				FVector2 uv{ FVector2{t1, 1 - t2} };
				uvs.push_back(uv);
			}
			//check 'f'
			else if (line.substr(0, 2) == "f ")
			{
				std::string line2{ line.substr(2, line.size()) };
				std::stringstream ss;
				ss << line2;
				for (int iter = 0; iter < 3; ++iter)
				{
					int v{}, n{}, t{};
					ss >> v;
					ss.ignore();
					ss >> n;
					ss.ignore();
					ss >> t;
					v--; n--; t--;
					const Vertex_Input vert{ positions[v], {1.f, 1.f, 1.f}, normals[t], {}, uvs[n] };
					vertices.push_back(vert);
					indices.push_back(idx++);
				}
			}
		}

		for (size_t iter = 0; iter < indices.size(); iter += 3)
		{
			uint32_t v0 = indices[iter];
			uint32_t v1 = indices[iter + 1];
			uint32_t v2 = indices[iter + 2];

			const FPoint3& p0 = vertices[v0].Position.xyz;
			const FPoint3& p1 = vertices[v1].Position.xyz;
			const FPoint3& p2 = vertices[v2].Position.xyz;
			const FVector2& uv0 = vertices[v0].UV;
			const FVector2& uv1 = vertices[v1].UV;
			const FVector2& uv2 = vertices[v2].UV;

			const FVector3 edge0 = p1 - p0;
			const FVector3 edge1 = p2 - p0;
			const FVector2 diffX = FVector2{uv1.x - uv0.x, uv2.x - uv0.x};
			const FVector2 diffY = FVector2{uv1.y - uv0.y, uv2.y - uv0.y};
			float r = 1.f / Cross(diffX, diffY);

			FVector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
			vertices[v0].Tangent += -tangent;
			vertices[v1].Tangent += tangent;
			vertices[v2].Tangent += tangent;
			//std::cout << iter << ": " << tangent.x << "  " << tangent.y << "  " << tangent.z << std::endl;
		}
		for (auto& v : vertices)
		{
			v.Tangent = Elite::GetNormalized(Reject(v.Tangent, v.Normal));
			//std::cout << v.tangent.x << "  " << v.tangent.y << "  " << v.tangent.z << std::endl;
		}

	}
}