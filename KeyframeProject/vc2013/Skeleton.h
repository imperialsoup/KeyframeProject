#ifndef KeyframeProject_Skeleton_h
#define KeyframeProject_Skeleton_h

/*Joint class*/
class Joint{
public:
	int		id = 0;
	std::string	name = "";
	ci::vec3	offset = ci::vec3(0.0, 0.0, 0.0);
	ci::vec4	boxmin = ci::vec4(0.0, 0.0, 0.0, 0.0);
	ci::vec4	boxmax = ci::vec4(0.0, 0.0, 0.0, 0.0);
	ci::vec3	pose = ci::vec3(0.0, 0.0, 0.0);

	ci::vec2	rotxlimit = ci::vec2(-100000, 100000);
	ci::vec2	rotylimit = ci::vec2(-100000, 100000);
	ci::vec2	rotzlimit = ci::vec2(-100000, 100000);
	std::vector<Joint>	children;
	Joint * parent;

	glm::mat4x4 world;
	glm::mat4x4 world_buffer;
	glm::mat4x4 local;

	inline void computeLocal(){
		glm::mat4x4 transMatrix = glm::translate(offset);
		glm::mat4x4 rotxMatrix = glm::rotate(pose.x, ci::vec3(1.0, 0, 0));
		glm::mat4x4 rotyMatrix = glm::rotate(pose.y, ci::vec3(0, 1.0, 0));
		glm::mat4x4 rotzMatrix = glm::rotate(pose.z, ci::vec3(0, 0, 1.0));
		local = transMatrix *  rotzMatrix * rotyMatrix * rotxMatrix;
		//return local;
	}

	inline void computeWorld(){
		cinder::app::console() << "world : " << world << std::endl;
		cinder::app::console() << "parent world : " << (*parent).world << std::endl;
		cinder::app::console() << "local : " << local << std::endl;
		world = (*parent).world * local;
		cinder::app::console() << "new world : " << world << std::endl;
		//world = local;
		//return world;
	}

	inline void update(){
		computeLocal();
		computeWorld();
	}
};


class Skeleton{
public:
	Skeleton();
	Skeleton(const std::string& filename);
	void init(const std::string& filename);
	Joint createJoint();
	void draw();
	void assignParent(Joint * parent);
	float clamp(float input, float min, float max);
	void drawJoint(Joint * joint);
	void depthSearch(Joint * node);
	void doThing();

	std::vector<Joint * > joints;
	Joint root;
	Joint world_joint;
	std::ifstream file;

	inline void update(){
		for (int i = 0; i < joints.size(); i++){
			(*joints[i]).update();
		}
	}
};

#endif
