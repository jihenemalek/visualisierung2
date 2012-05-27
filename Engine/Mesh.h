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
	void Mesh::calculateUpVectors(Segment*);
	D3DXVECTOR3 Mesh::calculateUp(D3DXVECTOR3 pu, D3DXVECTOR3 direction, D3DXVECTOR3 position, D3DXVECTOR3 normal);
};

