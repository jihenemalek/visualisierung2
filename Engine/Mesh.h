#include <libxml/tree.h>
#include "Vesseltree\Tree.hpp"
using namespace Vesseltree;
class Mesh
{
	
public:
	Mesh(void);
	~Mesh(void);
	void Mesh::calculateDirectionsNormals(Segment*);
	void Mesh::calculateDirNorm(SegmentPoint*);
	void Mesh::calculateMesh(Root*);
	void Mesh::averageNormals(Segment*);
	void Mesh::classifySegments(Segment*);
};

