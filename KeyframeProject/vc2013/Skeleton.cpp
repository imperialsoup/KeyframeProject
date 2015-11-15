#include "cinder/app/AppBase.h"
#include "cinder/cinder.h"
#include "cinder/Function.h"
#include "cinder/gl/gl.h"
#include <stdio.h>

#include "Skeleton.h"
#include <fstream>
#include "cinder/Json.h"

using namespace ci;
using namespace ci::app;
using namespace gl;
using namespace std;


/*Skeleton class*/
Skeleton::Skeleton(){
}

Skeleton::Skeleton(const std::string& filename){
	init(filename);
}

void Skeleton::init(const std::string& filename)
{
	console() << "Initializing Skeleton..." << std::endl;
	file.open(filename);

	if (!fs::exists(filename)){
		console() << "Skeleton.init() : Error, file not found." << std::endl;
		return;
	}
	if (file.is_open()){

		console() << "Skeleton.init() : File found, parsing now..." << std::endl;

		string firstjoint;
		file >> firstjoint;
		root = createJoint();

		world_joint = Joint(); world_joint.world = glm::mat4x4(1.0f);
		root.parent = &world_joint; //root is its own parent;

		joints.push_back(&root);
		assignParent(&root);
	}

	console() << "Skeleton.init() : Finished initializing skeleton! " << std::endl;
}

void Skeleton::assignParent(Joint * parent){

	for (int i = 0; i < (*parent).children.size(); i++){
		(*parent).children[i].parent = parent;
		console() << (*parent).children[i].name << "'s parent is " << (*(*parent).children[i].parent).name;

		joints.push_back(&(*parent).children[i]);

		//recursively assign parent
		if ((*parent).children[i].children.size() != 0){
			assignParent(&(*parent).children[i]);
		}
	}
}

Joint Skeleton::createJoint(){

	Joint joint = Joint();
	string input, name, trash;

	file >> joint.name;
	file >> trash;	//The start bracket
	file >> input;

	while (!file.eof()){
		if (input == "offset"){
			file >> joint.offset.x;
			file >> joint.offset.y;
			file >> joint.offset.z;
			//joint.offset.w = 1;
		}
		if (input == "boxmin"){
			file >> joint.boxmin.x;
			file >> joint.boxmin.y;
			file >> joint.boxmin.z;
			joint.boxmin.w = 1;
		}
		if (input == "boxmax"){
			file >> joint.boxmax.x;
			file >> joint.boxmax.y;
			file >> joint.boxmax.z;
			joint.boxmax.w = 1;
		}
		if (input == "rotxlimit"){
			file >> joint.rotxlimit.x;
			file >> joint.rotxlimit.y;
		}
		if (input == "rotylimit"){
			file >> joint.rotylimit.x;
			file >> joint.rotylimit.y;
		}
		if (input == "rotzlimit"){
			file >> joint.rotzlimit.x;
			file >> joint.rotzlimit.y;
		}
		if (input == "pose"){
			file >> joint.pose.x;
			file >> joint.pose.y;
			file >> joint.pose.z;
			//joint.pose.w = 1;
		}

		if (input == "balljoint"){
			Joint childJoint = createJoint();
			joint.children.push_back(childJoint);	//add the child joint to the parent joint's list of
		}

		/*HERE IS WHERE ITS FUCKING UP*/
		if (input == "}"){
			console() << "Created joint " + joint.name << std::endl;
			return joint;
		}
		file >> input;
	}
}

/*draw the dang thing*/
void Skeleton::draw(){
	drawJoint(&root);
}

void Skeleton::drawJoint(Joint * joint){
	multModelMatrix((*joint).local);
	pushModelMatrix();


	//console() << "Drawing " + (*joint).name + "..." << endl;

	float x = clamp((*joint).pose.x, (*joint).rotxlimit.x, (*joint).rotxlimit.y);
	float y = clamp((*joint).pose.y, (*joint).rotylimit.x, (*joint).rotylimit.y);
	float z = clamp((*joint).pose.z, (*joint).rotzlimit.x, (*joint).rotzlimit.y);

	begin(GL_LINES);
	color(255, 255, 255);

	//front of cube
	vertex((*joint).boxmin.x, (*joint).boxmin.y, (*joint).boxmin.z);	// A
	vertex((*joint).boxmax.x, (*joint).boxmin.y, (*joint).boxmin.z);	// B

	vertex((*joint).boxmax.x, (*joint).boxmin.y, (*joint).boxmin.z);	// B
	vertex((*joint).boxmax.x, (*joint).boxmax.y, (*joint).boxmin.z);	// G

	vertex((*joint).boxmax.x, (*joint).boxmax.y, (*joint).boxmin.z);	// G
	vertex((*joint).boxmin.x, (*joint).boxmax.y, (*joint).boxmin.z);	// E

	vertex((*joint).boxmin.x, (*joint).boxmax.y, (*joint).boxmin.z);	// E
	vertex((*joint).boxmin.x, (*joint).boxmin.y, (*joint).boxmin.z); // A

	//back of cube
	vertex((*joint).boxmax.x, (*joint).boxmax.y, (*joint).boxmax.z);	// H
	vertex((*joint).boxmin.x, (*joint).boxmax.y, (*joint).boxmax.z);	// F

	vertex((*joint).boxmin.x, (*joint).boxmax.y, (*joint).boxmax.z);	// F
	vertex((*joint).boxmin.x, (*joint).boxmin.y, (*joint).boxmax.z);	// C

	vertex((*joint).boxmin.x, (*joint).boxmin.y, (*joint).boxmax.z);	// C
	vertex((*joint).boxmax.x, (*joint).boxmin.y, (*joint).boxmax.z);	// D

	vertex((*joint).boxmax.x, (*joint).boxmin.y, (*joint).boxmax.z);	// D
	vertex((*joint).boxmax.x, (*joint).boxmax.y, (*joint).boxmax.z);	// H

	//connect front/back of cube
	vertex((*joint).boxmax.x, (*joint).boxmax.y, (*joint).boxmin.z);	// G
	vertex((*joint).boxmax.x, (*joint).boxmax.y, (*joint).boxmax.z);	// H

	vertex((*joint).boxmin.x, (*joint).boxmax.y, (*joint).boxmin.z);	// E
	vertex((*joint).boxmin.x, (*joint).boxmax.y, (*joint).boxmax.z);	// F

	vertex((*joint).boxmin.x, (*joint).boxmin.y, (*joint).boxmin.z); // A
	vertex((*joint).boxmin.x, (*joint).boxmin.y, (*joint).boxmax.z);	// C

	vertex((*joint).boxmax.x, (*joint).boxmin.y, (*joint).boxmin.z);	// B
	vertex((*joint).boxmax.x, (*joint).boxmin.y, (*joint).boxmax.z);	// D

	//console() << "Finished drawing " + (*joint).name + "." << endl;
	end();


	if ((*joint).children.size() != 0){
		for (int i = 0; i < (*joint).children.size(); i++){
			drawJoint(&(*joint).children[i]);
		}
	}
	popModelMatrix();
}

float Skeleton::clamp(float input, float min, float max){
	float result = 0;
	if (input < min)
		result = toRadians(min);
	else if (input > max)
		result = toRadians(max);
	else
		result = toRadians(input);

	return result;
}



/*need some way to do depth first traversal of joints*/
void Skeleton::depthSearch(Joint * node){
	console() << (*node).name << endl;
	for each(Joint child in (*node).children){
		depthSearch(&child);
	}
	return;
}