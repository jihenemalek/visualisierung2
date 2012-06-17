#ifndef _MESH_H_
#define _MESH_H_

#include <libxml/tree.h>
#include "Vesseltree\Tree.hpp"

#include <set>


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
	
	void Mesh::tileTree(Segment*);
	void Mesh::tileTrivially(Node *firstPoint, Node *secondPoint);
	void Mesh::tileJoint(std::set<Segment*> segments, D3DXVECTOR3 direction, Segment *caller, D3DXVECTOR3 quadrantDirection);

	void Mesh::tileTrivially(Segment*);
	std::vector<D3DXVECTOR3> Mesh:: tileTrivial(D3DXVECTOR3, float, D3DXVECTOR3, D3DXVECTOR3);
	D3DXVECTOR3 Mesh:: rotateVector(D3DXVECTOR3, D3DXVECTOR3);
	void Mesh::generatePatches(Segment*);
	void Mesh::generatePatchesNonBranching(Segment*);
	void Mesh:: processLastSections(Segment*);

	void Mesh::triangulate(void);
	typedef struct Patch
	{
		D3DXVECTOR3 vertex0;
		D3DXVECTOR3 vertex1;
		D3DXVECTOR3 vertex2;
		D3DXVECTOR3 vertex3;

		D3DXVECTOR3 normal;

		D3DXVECTOR3 normal0;
		D3DXVECTOR3 normal1;
		D3DXVECTOR3 normal2;
		D3DXVECTOR3 normal3;

		D3DXVECTOR3 vertex0_mittelpunkt;
		D3DXVECTOR3 vertex1_mittelpunkt;
		D3DXVECTOR3 vertex2_mittelpunkt;
		D3DXVECTOR3 vertex3_mittelpunkt;

		float vertex0_radius;
		float vertex1_radius;
		float vertex2_radius;
		float vertex3_radius;

	} Patch;
	std::vector<Patch> patches;
	
	typedef struct Triangle
	{
		D3DXVECTOR3 vertex0;
		D3DXVECTOR3 vertex1;
		D3DXVECTOR3 vertex2;

		D3DXVECTOR3 normal;

		D3DXVECTOR3 vertex0_mittelpunkt;
		D3DXVECTOR3 vertex1_mittelpunkt;
		D3DXVECTOR3 vertex2_mittelpunkt;
		D3DXVECTOR3 vertex3_mittelpunkt;

		float vertex0_radius;
		float vertex1_radius;
		float vertex2_radius;
		
		

	} Trianlge;
	std::vector<Triangle> triangles;
	

		
	
};

#endif
