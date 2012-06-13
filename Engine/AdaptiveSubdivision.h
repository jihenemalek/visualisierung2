#ifndef _ADAPTIVESUBDIVISION_H_
#define _ADAPTIVESUBDIVISION_H_

#include <libxml/tree.h>
#include "Vesseltree\Tree.hpp"
#include "Mesh.h"

#include <set>


using namespace Vesseltree;
class AdaptiveSubdivision
{

	
public:
	AdaptiveSubdivision(void);
	~AdaptiveSubdivision(void);
	std::vector<Mesh::Triangle>  AdaptiveSubdivision::Subdivide(std::vector<Mesh::Triangle>, float);
	

	typedef struct Tagged
	{
		Mesh::Triangle* triangle;

		std::vector<D3DXVECTOR3> vertices;
		std::vector<int> number;
		int geteilt;

	} Tagged;
	std:: vector<Tagged> tagged;

	typedef struct Subdivided
	{
		Mesh::Triangle* triangle;

		D3DXVECTOR3 new_vertex;

	} Subdivided;
	std::vector<Subdivided> subdivided;

};
#endif