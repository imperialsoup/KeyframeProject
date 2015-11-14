#include "cinder/app/AppBase.h"
#include "cinder/cinder.h"
#include "cinder/Function.h"
#include "glm/glm.hpp"
#include "Skeleton.h"
#include <fstream>
#include "Skin.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace glm;
using namespace ci::app;
using namespace std;
using namespace gl;


Skin::Skin(){}

/** init functions
all init functions initalize their respective member variable
*/
void Skin::init(const std::string& filename, Skeleton * skeletonptr){

	skeleton = skeletonptr;	//set the associated skeleton
	vector<vec4> vertices;
	vector<vec4> normals;
	vector<map<int, float>> skinweights;
	int num;

	console() << "Initializing Skin..." << endl;
	file.open(filename);

	if (!fs::exists(filename)){
		console() << "Skin.init() : Error, file not found." << endl;
		return;
	}

	if (file.is_open()){
		console() << "Skin.init() : File found, parsing now..." << endl;
		string input, trash;

		while (!file.eof()){

			file >> input;	//the identifier
			file >> num;
			file >> trash;	//the bracket

			if (input == "positions"){
				vertices = initVertices(num);
			}
			if (input == "normals"){
				normals = initNormals(num);
			}
			if (input == "skinweights"){
				skinweights = initSkinWeight(num);
			}
			if (input == "triangles"){
				initTriangles(num);
			}
			if (input == "bindings"){
				initBindings(num);
			}
			if (input == "}"){
				continue;
			}
		}
		console() << "Skin.init() : Done!" << endl;
	}

	//init vertexes
	for (int i = 0; i < vertices.size(); i++){
		SkinVertex skinvertex;
		skinvertex.position = vertices[i];
		skinvertex.normal = normals[i];
		skinvertex.weights = skinweights[i];
		skinvertexes.push_back(skinvertex);
	}

	//init skinning matrices
	for (int i = 0; i < (*skeleton).joints.size(); i++){
		mat4x4 skinningMatrix = mat4x4(1.0);

		skinMatrices.push_back(skinningMatrix);
	}
}

vector<ci::vec4> Skin::initVertices(int numVertices){
	vector<ci::vec4> vertices; //a vector of vec3s to return

	console() << "initVertices : initializing vertices..." << endl;
	string trash;
	vec4 vertex = vec4(0.0);
	for (int i = 0; i < numVertices; i++){
		file >> vertex.x;
		file >> vertex.y;
		file >> vertex.z;
		vertex.w = 0.0;
		vertices.push_back(vertex);
	}
	file >> trash; //bracket
	console() << "initVertices : Done!" << endl;
	return vertices;
}

vector<map<int, float>> Skin::initSkinWeight(int numVertices){
	vector<map<int, float>> skinweights;

	console() << "initskinWeights : initializing skinweights..." << endl;
	string trash;
	int numBones, boneIndex;
	float weight;

	map<int, float> boneweights;

	for (int i = 0; i < numVertices; i++){
		file >> numBones;
		for (int j = 0; j < numBones; j++){
			file >> boneIndex;
			file >> weight;
			boneweights.insert(pair<int, float>(boneIndex, weight));
		}
		skinweights.push_back(boneweights);
		boneweights.clear();
	}
	file >> trash; //bracket
	console() << "initSkinWeights : Done!" << endl;
	return skinweights;
}


void Skin::initTriangles(int numTriangles){
	console() << "initTriangles : initializing triangles..." << endl;
	string trash;
	vec3 vertex = vec3();
	for (int i = 0; i < numTriangles; i++){
		file >> vertex.x;
		file >> vertex.y;
		file >> vertex.z;
		triangles.push_back(vertex);
	}

	file >> trash; //bracket
	console() << "initTriangles : Done!" << endl;
	return;
}

void Skin::initBindings(int numBindings){
	console() << "initBindings : initializing vertices..." << endl;
	string trash;
	mat4x4 binding;
	for (int i = 0; i < numBindings; i++){
		file >> trash; file >> trash;	// 'matrix', '}'		//add extra column for homo coords
		file >> binding[0][0]; file >> binding[0][1]; file >> binding[0][2];  binding[0][3] = 0;
		file >> binding[1][0]; file >> binding[1][1]; file >> binding[1][2];  binding[1][3] = 0;
		file >> binding[2][0]; file >> binding[2][1]; file >> binding[2][2];  binding[2][3] = 0;
		file >> binding[3][0]; file >> binding[3][1]; file >> binding[3][2];  binding[3][3] = 1;
		/*file >> binding[0][0]; file >> binding[1][0]; file >> binding[2][0];  binding[3][0] = 0;
		file >> binding[0][1]; file >> binding[1][1]; file >> binding[2][1];  binding[3][1] = 0;
		file >> binding[0][2]; file >> binding[1][2]; file >> binding[2][2];  binding[3][2] = 0;
		file >> binding[0][3]; file >> binding[1][3]; file >> binding[2][3];  binding[3][3] = 1;*/
		file >> trash; // bracket

		mat4x4 invBinding = inverse(binding);

		console() << "binding" << i << " : " << binding << endl;
		console() << "inverse binding" << i << " : " << invBinding << endl;
		bindings.push_back(invBinding);
	}
	console() << "initBindings : Done!" << endl;
}

vector<vec4> Skin::initNormals(int numVertices){
	vector<vec4> normals;
	console() << "initNormals: initializing normals..." << endl;
	string trash;
	vec4 normal = vec4(0.0);
	for (int i = 0; i < numVertices; i++){
		file >> normal.x;
		file >> normal.y;
		file >> normal.z;
		normal.w = 0.0;
		normals.push_back(normal);
	}
	file >> trash; //bracket
	console() << "initNormals: Done!" << endl;
	return normals;
}

/*draw the Skin*/
void Skin::draw(){
	//set matrix state to world identity
	//loop through triangles and draw using world space positions and normals
	pushMatrices();
	begin(GL_TRIANGLES);
	color(212, 144, 195);
	for (int i = 0; i < triangles.size(); i++){
		/*	vertex(skinvertexes[triangles[i].x].trans_position.x, skinvertexes[triangles[i].x].trans_position.y, skinvertexes[triangles[i].x].trans_position.z);
		vertex(skinvertexes[triangles[i].y].trans_position.x, skinvertexes[triangles[i].y].trans_position.y, skinvertexes[triangles[i].y].trans_position.z);
		vertex(skinvertexes[triangles[i].z].trans_position.x, skinvertexes[triangles[i].z].trans_position.y, skinvertexes[triangles[i].z].trans_position.z);*/

		vertex(skinvertexes[triangles[i].x].trans_position.x, skinvertexes[triangles[i].x].trans_position.y, skinvertexes[triangles[i].x].trans_position.z);
		vertex(skinvertexes[triangles[i].y].trans_position.x, skinvertexes[triangles[i].y].trans_position.y, skinvertexes[triangles[i].y].trans_position.z);
		vertex(skinvertexes[triangles[i].z].trans_position.x, skinvertexes[triangles[i].z].trans_position.y, skinvertexes[triangles[i].z].trans_position.z);
	}
	end();
	console() << "Skin::draW() : done " << endl;
	popMatrices();
}

void Skin::update(){
	//compute skinning matrices for each joint
	for (int i = 0; i < (*skeleton).joints.size(); i++){
		skinMatrices[i] = (*(*skeleton).joints[i]).world * bindings[i];
		/*console()<< i << endl;
		console() << "joint world " << (*(*skeleton).joints[i]).world << endl;;
		console() << "invbinding: " <<  bindings[i] << endl;;
		console() << "skinMatrice : " << skinMatrices[i] << endl;*/
	}

	//Loop through vertices and compute blended position & normal
	for (int i = 0; i < skinvertexes.size(); i++){
		vec4 transformedVertex = vec4(0.0);
		vec4 transformedNormal = vec4(0.0);
		//Compute blended positions and normals 

		/**	iterates through the vertex's weight dictionary and accumulate the average
		it->first is index of bone
		it->second is weight regarding that bone*/
		for (map<int, float>::iterator it = skinvertexes[i].weights.begin(); it != skinvertexes[i].weights.end(); it++){
			std::cout << it->first << " => " << it->second << '\n';
			transformedVertex += it->second * (skinMatrices[it->first] * skinvertexes[i].position);
			transformedNormal += it->second * (skinMatrices[it->first] * skinvertexes[i].normal);
		}
		skinvertexes[i].trans_position = transformedVertex;
		skinvertexes[i].trans_normal = transformedNormal;
	}
}