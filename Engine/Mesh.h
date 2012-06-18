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

	D3DXVECTOR3 Mesh:: rotateVector(D3DXVECTOR3, D3DXVECTOR3);

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

		float vertex0_radius;
		float vertex1_radius;
		float vertex2_radius;
		
		D3DXVECTOR3 vertexAt(int i)
		{ 
			if ((i % 3) == 0) return vertex0;
			if ((i % 3) == 1) return vertex1;
			return vertex2;
		}
		
		D3DXVECTOR3 centerlineAt(int i)
		{ 
			if ((i % 3) == 0) return vertex0_mittelpunkt;
			if ((i % 3) == 1) return vertex1_mittelpunkt;
			return vertex2_mittelpunkt;
		}
		
		float radiusAt(int i)
		{ 
			if ((i % 3) == 0) return vertex0_radius;
			if ((i % 3) == 1) return vertex1_radius;
			return vertex2_radius;
		}

		void setVertexAt(int i, D3DXVECTOR3 v) {
			if ((i % 3) == 0) this->vertex0 = v;
			if ((i % 3) == 1) this->vertex1 = v;
			this->vertex2 = v;
		}

		void setCenterlineAt(int i, D3DXVECTOR3 c) {
			if ((i % 3) == 0) this->vertex0_mittelpunkt = c;
			if ((i % 3) == 1) this->vertex1_mittelpunkt = c;
			this->vertex2_mittelpunkt = c;
		}

		void setRadiusAt(int i, float r) {
			if ((i % 3) == 0) this->vertex0_radius = r;
			if ((i % 3) == 1) this->vertex1_radius = r;
			this->vertex2_radius = r;
		}
	} Triangle;

	std::vector<Triangle> triangles;

};

#endif
