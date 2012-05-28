#include "Mesh.h"


Mesh::Mesh(void)
{
}


Mesh::~Mesh(void)
{
}

void Mesh:: calculateMesh(Root* root)
{
	Segment* seg = root->child;
	if(root->child != NULL)
	{
		//Calculate Normal + Direction for the first cross-section (control point)
		D3DXVECTOR3 direction;
		direction.x = seg->points.back()->position.x - seg->startNode->position.x;
		direction.y = seg->points.back()->position.y - seg ->startNode->position.y;
		direction.z = seg->points.back()->position.z - seg->startNode->position.z;
		seg->startNode->direction = direction;
		seg->startNode->normal = direction;
		//Calculate Normals + Directions for all cross-sections (segment points)
		calculateDirectionsNormals(seg);
		//calculate Normal + Direction for last cross-section (control point)
		seg->endNode->direction = seg->points.back()->direction;
		seg->endNode->normal = seg->endNode->direction;

		//Calculate average normals at the branchings
		averageNormals(seg);
		//Classify the segments in forward and backward
		classifySegments(seg);

		//Calculate up-Vectors
		//first choose up vector perpendicular to direction of first cross section of the first segment
		direction = seg->startNode->direction;
		D3DXVECTOR3 up_vector;
		up_vector.z = 0;
		up_vector.x = 1;
		up_vector.y = -(direction.x*up_vector.x)/direction.y;
		seg->startNode->upVector = up_vector;
		//Calculate remaining up-Vectors
		calculateUpVectors(seg);

		//Tile Tree Algorithmus -> is implemented different to the paper
		tileTree(seg);

		//Triangulate quadrilateral mesh
		triangulate();
		
		

	
	}
	else
	{
		HWND hwnd;
		MessageBox(hwnd, L"no segments", L"Error", MB_OK);
	}
}


//Calculate Normals and Directions of the Segments
//This function is called for each segment
void Mesh:: calculateDirectionsNormals(Segment* seg)
{
	if(seg->points.size() != 0)
	{
		SegmentPoint* node = seg->points.at(0);
		calculateDirNorm(node);
	}
	std::vector<Segment*> seg_list = seg->children;
	for (int i = 0; i < seg_list.size(); i++)
	{
		if(seg_list.at(i) != NULL)
		{
			calculateDirectionsNormals(seg_list.at(i));
		}
	}
}
	
	

//Here the normals and directions for each cross-section are calculated
void Mesh:: calculateDirNorm(SegmentPoint* seg_point)
{
	D3DXVECTOR3 normal;
	D3DXVECTOR3 direction;
	SegmentPoint* seg_point1;
	SegmentPoint* seg_point2;
	
	
	if(seg_point->child != NULL)
	{
		seg_point2 = seg_point->child;
		direction.x = seg_point2->direction.x - seg_point->direction.x;
		direction.y = seg_point2->direction.y - seg_point->direction.y;
		direction.z = seg_point2->direction.z - seg_point->direction.z;
		D3DXVec3Normalize(&direction,&direction);
		seg_point->direction = direction;
	}

	
	if(seg_point->parent != NULL)
	{
		seg_point1 = seg_point->parent;
		normal = seg_point1->direction + seg_point->direction;
		D3DXVec3Normalize(&normal,&normal);
		seg_point->normal = normal;

	}
	
	
	if(seg_point->child != NULL)
	{
		calculateDirNorm(seg_point2);
	}
}

//Calculate the average Normals at the branchings -> nicht ber�cksichtigt dass ein Child mehrere Parents haben kann
void Mesh:: averageNormals(Segment* seg)
{
	
	if((seg->endNode != NULL) && (seg->children.size() != NULL))
	
	{	
		ControlPoint* end_node = seg->endNode;

		std::vector<Segment*> seg_list = seg->children;
		D3DXVECTOR3 average = end_node->normal;
		int average_counter = 1;
		for (int i = 0; i < seg_list.size(); i++)
		{
			if(seg_list.at(i) != NULL)
			{
				//Calculate dot product 

				if(seg_list.at(i)->startNode != NULL)
				{
					
					ControlPoint* a = seg_list.at(i)->startNode;
					D3DXVECTOR3 average2 = a->normal;
					FLOAT product = D3DXVec3Dot(&average,&average2);
					if(product > 0) //take only positive results
					{
						average = average + average2; 
						average_counter = average_counter + 1;
					}
				}
			}	
			average = average / average_counter;
		}

		end_node->normal = average;
		for(int i = 0; i < seg_list.size();i++)
		{
			if(seg_list.at(i) != NULL)
			{
				//set all normals to the average normals and than call function again for all childs
				if(seg_list.at(i)->startNode != NULL)
				{
					seg_list.at(i)->startNode->normal = average;
				}
				averageNormals(seg_list.at(i));
			}
		}
	}

	

}

//classify the segments in forward or backward -> Was passiert hier mit dem ersten Segment?
void Mesh:: classifySegments(Segment* seg)
{
	if(seg->startNode != NULL)
	{
		ControlPoint* start_node = seg->startNode;

		
		D3DXVECTOR3 average = start_node->normal;
		D3DXVECTOR3 direction = start_node->direction;
		FLOAT product = D3DXVec3Dot(&average,&direction);
		if(product > 0)
		{
			seg->classification = 0;
		}
		else
		{
			seg->classification = 1;
		}
	}
	std::vector<Segment*> seg_list = seg->children;

	for (int i = 0; i < seg_list.size(); i++)
	{
		if(seg_list.at(i) != NULL)
		{
			classifySegments(seg_list.at(i));
			
		}
	}
}

//Calculate up-Vector for each segment
void Mesh::calculateUpVectors(Segment* seg)
{
	D3DXVECTOR3 up_vector;
	if(seg->points.size()!=0)
	{
		//�ber Segment iterieren
		D3DXVECTOR3 pu, position;
		pu.x = seg->startNode->position.x + seg->startNode->upVector.x;
		pu.y = seg->startNode->position.y + seg->startNode->upVector.y;
		pu.z = seg->startNode->position.z + seg->startNode->upVector.z;

		
		position.x = seg->points.at(0)->position.x;
		position.y = seg->points.at(0)->position.y;
		position.z = seg->points.at(0)->position.z;

		up_vector = calculateUp(pu,seg->startNode->direction,position,seg->points.at(0)->normal);
		seg->points.at(0)->upVector = up_vector;


		for(int i = 0; i < (seg->points.size()-1); i++)
		{
			pu.x = seg->points.at(i)->position.x + seg->points.at(i)->upVector.x;
			pu.y = seg->points.at(i)->position.y + seg->points.at(i)->upVector.y;
			pu.z = seg->points.at(i)->position.z + seg->points.at(i)->upVector.z;

			position.x = seg->points.at(i+1)->position.x;
			position.y = seg->points.at(i+1)->position.y;
			position.z = seg->points.at(i+1)->position.z;
		
			up_vector = calculateUp(pu,seg->points.at(i)->direction,position,seg->points.at(i+1)->normal);
			seg->points.at(i+1)->upVector = up_vector;
		}

		//letzer Control Point
		pu.x = seg->points.back()->position.x + seg->points.back()->upVector.x;
		pu.y = seg->points.back()->position.y + seg->points.back()->upVector.y;
		pu.z = seg->points.back()->position.z + seg->points.back()->upVector.z;

		position.x = seg->endNode->position.x;
		position.y = seg->endNode->position.y;
		position.z = seg->endNode->position.z;

		up_vector = calculateUp(pu,seg->points.back()->direction,position,seg->endNode->normal);
		seg->endNode->upVector = up_vector;

		//An jeden StartNode der Children weitergeben und die Funktion nochmals aufrufen
		if(seg->children.size() != 0)
		{
			for(int i = 0; i < seg->children.size(); i++)
			{
				Segment* seg2 = seg->children.at(i);
				pu.x = seg->endNode->position.x + seg->endNode->upVector.x;
				pu.y = seg->endNode->position.y + seg->endNode->upVector.y;
				pu.z = seg->endNode->position.z + seg->endNode->upVector.z;

				position.x = seg2->startNode->position.x;
				position.y = seg2->startNode->position.y;
				position.z = seg2->startNode->position.z;

				up_vector = calculateUp(pu,seg->endNode->direction,position,seg2->startNode->normal);
				seg2->startNode->upVector = up_vector;
				calculateUpVectors(seg2);
			}
		}

	}

}

//here the up-vector is calculated
D3DXVECTOR3 Mesh::calculateUp(D3DXVECTOR3 pu, D3DXVECTOR3 direction, D3DXVECTOR3 position, D3DXVECTOR3 normal)
{
	D3DXVECTOR3 temp,up_vector;
	FLOAT product,product2,s;
	temp = position-(pu+direction);
	product = D3DXVec3Dot(&temp, &(normal));
	product2 = D3DXVec3Dot(&direction,&normal);
	s = product / product2;
	temp = (s*direction) + pu;
	up_vector = temp-position;
	D3DXVec3Normalize(&up_vector,&up_vector);
	return up_vector;

}

//Tile tree algorithm
void Mesh::tileTree(Segment* seg)
{
	
	//for root segment ->process tile trivial algorithm
	D3DXVECTOR3 position, up_vector;
	float radius;
	std::vector<D3DXVECTOR3> vertex_list;
	position.x = seg->startNode->position.x;
	position.y = seg->startNode->position.y;
	position.z = seg->startNode->position.z;

	vertex_list = tileTrivial(position, seg->startNode->radius, seg->startNode->upVector, seg->startNode->direction);
	seg->startNode->vertices = vertex_list;

	//for all non-branching elements->process tile Trivially
	tileTrivially(seg);

	//Generate patches for first section of the first segment and non-branching sections incl. transition patches between the non-branching section and between the first section of the first segment and the first non-branching section
	generatePatches(seg);
	//process last section of incoming segment
	processLastSections(seg);

}

//tile Trivially
void Mesh::tileTrivially(Segment* seg)
{
	
	D3DXVECTOR3 position, up_vector;
	float radius;
	std::vector<D3DXVECTOR3> vertex_list;

	//f�r segmente ausserhalb der Branchings berechnen
	if(seg->points.size() != 0)
	{
		for(int i = 0; i < seg->points.size(); i++)
		{
			position.x = seg->points.at(i)->position.x;
			position.y = seg->points.at(i)->position.y;
			position.z = seg->points.at(i)->position.z;

			radius = seg->points.at(i)->radius;

			up_vector = seg->points.at(i)->upVector;
			
			vertex_list = tileTrivial(position, radius, up_vector, seg->points.at(i)->direction);
			seg->points.at(i)->vertices = vertex_list;
		}

	}
	if(seg->children.size() != 0)
	{
		for(int i = 0; i < seg->children.size(); i++)
		{
			tileTrivially(seg->children.at(i));
		}
	}
	else
	{
		position.x = seg->endNode->position.x;
		position.y = seg->endNode->position.y;
		position.z = seg->endNode->position.z;
		
		radius = seg->endNode->radius;
		up_vector = seg->endNode->upVector;

		vertex_list = tileTrivial(position,radius,up_vector, seg->endNode->direction);
		seg->endNode->vertices = vertex_list;
	}

}


//tile trivial ->gives back the vertices of the cross-sections
std::vector<D3DXVECTOR3> Mesh:: tileTrivial(D3DXVECTOR3 position, float radius, D3DXVECTOR3 up_vector,  D3DXVECTOR3 direction)
{
	D3DXVECTOR3 temp_vertex;
	std::vector<D3DXVECTOR3> vertex_list;
	temp_vertex = position + (radius * up_vector);
	vertex_list.push_back(temp_vertex);

	//rotate up_vector to calculate the other 3 vectors
	for(int i = 0; i< 3; i++)
	{
		up_vector = rotateVector(up_vector,direction);
		temp_vertex = position + (radius * up_vector);
		vertex_list.push_back(temp_vertex);
	}
	

	return vertex_list;
}

//Function to rotate up_vector around direction_vector
D3DXVECTOR3 Mesh:: rotateVector(D3DXVECTOR3 up_vector, D3DXVECTOR3 direction_vector)
{
	
	FLOAT x = up_vector.x;
	FLOAT y = up_vector.y;
	FLOAT z = up_vector.z;
	FLOAT u = direction_vector.x;
	FLOAT v = direction_vector.y;
	FLOAT w = direction_vector.z;
	D3DXVECTOR3 rotated_vec;

	rotated_vec.x =u*(u*x+v*y+w*z)+(x*(v*v+w*w)-u*(v*y+w*z))*0+(-w*y+v*z)*1;
	rotated_vec.y =v*(u*x+v*y+w*z)+(y*(u*u+w*w)-v*(u*x+w*z))*0+(w*x-u*z)*1;
	rotated_vec.z =w*(u*x+v*y+w*z)+(z*(u*u+v*v)-w*(u*x+v*y))*0+(-v*x+u*y)*1;

	D3DXVec3Normalize(&rotated_vec,&rotated_vec);

	return rotated_vec;
}

//generate patches for root section and non-branching sections
void Mesh::generatePatches(Segment* seg)
{
	//Generate patches for root element
		Patch temp_patch;
		if(seg->startNode->vertices.size() != 0)
		{
			temp_patch.vertex0 = seg->startNode->vertices.at(0);
			temp_patch.vertex1 = seg->startNode->vertices.at(1);
			temp_patch.vertex2 = seg->startNode->vertices.at(2);
			temp_patch.vertex3 = seg->startNode->vertices.at(3);

			patches.push_back(temp_patch);

			//Generate patches for the first segment points
			if(seg->points.size() != 0)
			{
				if(seg->points.at(0)->vertices.size() != 0)
				{
					temp_patch.vertex0 = seg->points.at(0)->vertices.at(0);
					temp_patch.vertex1 = seg->points.at(0)->vertices.at(1);
					temp_patch.vertex2 = seg->points.at(0)->vertices.at(2);
					temp_patch.vertex3 = seg->points.at(0)->vertices.at(3);

					patches.push_back(temp_patch);
				}

				//create 4 sides of the cube between root segment and first segment point
				//back side
				temp_patch.vertex0 = seg->startNode->vertices.at(0);
				temp_patch.vertex1 = seg->startNode->vertices.at(1);
				temp_patch.vertex2 = seg->points.at(0)->vertices.at(1);
				temp_patch.vertex3 = seg->points.at(0)->vertices.at(0);
				patches.push_back(temp_patch);

				//left side
				temp_patch.vertex0 = seg->startNode->vertices.at(1);
				temp_patch.vertex1 = seg->startNode->vertices.at(2);
				temp_patch.vertex2 = seg->points.at(0)->vertices.at(2);
				temp_patch.vertex3 = seg->points.at(0)->vertices.at(1);
				patches.push_back(temp_patch);

				//front side
				temp_patch.vertex0 = seg->startNode->vertices.at(3);
				temp_patch.vertex1 = seg->startNode->vertices.at(2);
				temp_patch.vertex2 = seg->points.at(0)->vertices.at(2);
				temp_patch.vertex3 = seg->points.at(0)->vertices.at(3);
				patches.push_back(temp_patch);

				//right side
				temp_patch.vertex0 = seg->startNode->vertices.at(0);
				temp_patch.vertex1 = seg->startNode->vertices.at(3);
				temp_patch.vertex2 = seg->points.at(0)->vertices.at(3);
				temp_patch.vertex3 = seg->points.at(0)->vertices.at(0);
				patches.push_back(temp_patch);
			}
			
		}

		//Generate patches for all non-branching cross sections
		generatePatchesNonBranching(seg);
}

//generates patches of non-branching sections and the transition patches between them
void Mesh::generatePatchesNonBranching(Segment* seg)
{
	Patch temp_patch;
	if(seg->points.size() != 0)
	{
		temp_patch.vertex0 = seg->points.at(0)->vertices.at(0);
		temp_patch.vertex1 = seg->points.at(0)->vertices.at(1);
		temp_patch.vertex2 = seg->points.at(0)->vertices.at(2);
		temp_patch.vertex3 = seg->points.at(0)->vertices.at(3);
		patches.push_back(temp_patch);

		if(seg->points.size() > 1)
		{
			for(int i = 1; i < seg->points.size(); i++)
			{
				//create Patch
				temp_patch.vertex0 = seg->points.at(i)->vertices.at(0);
				temp_patch.vertex1 = seg->points.at(i)->vertices.at(1);
				temp_patch.vertex2 = seg->points.at(i)->vertices.at(2);
				temp_patch.vertex3 = seg->points.at(i)->vertices.at(3);
				patches.push_back(temp_patch);

				//create 4 sides of the cube between two sections
				//back side
				temp_patch.vertex0 = seg->points.at(i-1)->vertices.at(0);
				temp_patch.vertex1 = seg->points.at(i-1)->vertices.at(1);
				temp_patch.vertex2 = seg->points.at(i)->vertices.at(1);
				temp_patch.vertex3 = seg->points.at(i)->vertices.at(0);
				patches.push_back(temp_patch);

				//left side
				temp_patch.vertex0 = seg->points.at(i-1)->vertices.at(1);
				temp_patch.vertex1 = seg->points.at(i-1)->vertices.at(2);
				temp_patch.vertex2 = seg->points.at(i)->vertices.at(2);
				temp_patch.vertex3 = seg->points.at(i)->vertices.at(1);
				patches.push_back(temp_patch);

				//front side
				temp_patch.vertex0 = seg->points.at(i-1)->vertices.at(3);
				temp_patch.vertex1 = seg->points.at(i-1)->vertices.at(2);
				temp_patch.vertex2 = seg->points.at(i)->vertices.at(2);
				temp_patch.vertex3 = seg->points.at(i)->vertices.at(3);
				patches.push_back(temp_patch);

				//right side
				temp_patch.vertex0 = seg->points.at(i-1)->vertices.at(0);
				temp_patch.vertex1 = seg->points.at(i-1)->vertices.at(3);
				temp_patch.vertex2 = seg->points.at(i)->vertices.at(3);
				temp_patch.vertex3 = seg->points.at(i)->vertices.at(0);
				patches.push_back(temp_patch);
			}
		}
	}

	if(seg->children.size() != 0)
	{
		for(int i = 0; i < seg->children.size(); i++)
		{
			generatePatchesNonBranching(seg->children.at(i));
		}
	}
	
}

//process last section
void Mesh:: processLastSections(Segment* seg)
{
	std::vector<int> backward_index;
	if((seg->endNode != NULL) && (seg->children.size() != 0))
	{
		for(int i = 0; i < seg->children.size(); i++)
		{
			if(seg->children.at(i)->classification = 0) //backward
			{
				backward_index.push_back(i);
			}
		}

		if(backward_index.size() == 0) //if no backward branches exist -> process tile trivial and create the patch of the endNode and the patches between the endNode and the last segment point
		{
			D3DXVECTOR3 position;
			std::vector<D3DXVECTOR3> vertex_list;
			Patch temp_patch;

			position.x = seg->endNode->position.x;
			position.y = seg->endNode->position.y;
			position.z = seg->endNode->position.z;
		
			vertex_list = tileTrivial(position,seg->endNode->radius,seg->endNode->upVector, seg->endNode->direction);
			seg->endNode->vertices = vertex_list;

			//create Patches
			temp_patch.vertex0 = seg->endNode->vertices.at(0);
			temp_patch.vertex1 = seg->endNode->vertices.at(1);
			temp_patch.vertex2 = seg->endNode->vertices.at(2);
			temp_patch.vertex3 = seg->endNode->vertices.at(3);
			patches.push_back(temp_patch);

			if(seg->points.size() != 0)
			{
				//back side
				temp_patch.vertex0 = seg->points.back()->vertices.at(0);
				temp_patch.vertex1 = seg->points.back()->vertices.at(1);
				temp_patch.vertex2 = seg->endNode->vertices.at(1);
				temp_patch.vertex3 = seg->endNode->vertices.at(0);
				patches.push_back(temp_patch);

				//left side
				temp_patch.vertex0 = seg->points.back()->vertices.at(1);
				temp_patch.vertex1 = seg->points.back()->vertices.at(2);
				temp_patch.vertex2 = seg->endNode->vertices.at(2);
				temp_patch.vertex3 = seg->endNode->vertices.at(1);
				patches.push_back(temp_patch);

				//front side
				temp_patch.vertex0 = seg->points.back()->vertices.at(3);
				temp_patch.vertex1 = seg->points.back()->vertices.at(2);
				temp_patch.vertex2 = seg->endNode->vertices.at(2);
				temp_patch.vertex3 = seg->endNode->vertices.at(3);
				patches.push_back(temp_patch);

				//right side
				temp_patch.vertex0 = seg->points.back()->vertices.at(0);
				temp_patch.vertex1 = seg->points.back()->vertices.at(3);
				temp_patch.vertex2 = seg->endNode->vertices.at(3);
				temp_patch.vertex3 = seg->endNode->vertices.at(0);
				patches.push_back(temp_patch);
			}


		}
		else //here the backward segments have to be processed
		{
			//for each quadrant call function TielJoint

		}
	}

	if(seg->children.size() != 0)
	{
		for(int i = 0; i < seg->children.size(); i++)
		{
			processLastSections(seg->children.at(i));
		}
	}

}

void Mesh:: TileJoint(std::vector<Segment*> seg_list, D3DXVECTOR3 direction)
{
	if(seg_list.size() == 0)
	{
		//closing patch
	}
	else
	{
		FLOAT product;
		//take segment with minimal angle to direction
		float angle = 400, temp_angle;
		FLOAT a, b;
		int index = 0;

		//calculate segment with minimal angle -> index is calculated
		for(int i = 0; i < seg_list.size(); i++)
		{
			product = D3DXVec3Dot(&direction,&seg_list.at(i)->startNode->direction);
			a = D3DXVec3Length(&direction);
			b = D3DXVec3Length(&(seg_list.at(i)->startNode->direction));
			temp_angle = product/(a*b);
			temp_angle = acos(temp_angle);

			if(temp_angle < angle)
			{
				index = i;
			}
		}

		//classify remaining segments into quadrants of N

		//space between C and N is tiled by transition patch

		//for the remaining 3 quadrants TileJoint is called
		
	}
}

void Mesh::triangulate()
{
	Triangle temp_triangle;
	for(int i = 0; i < patches.size(); i++)
	{
		temp_triangle.vertex0 = patches.at(i).vertex0;
		temp_triangle.vertex1 = patches.at(i).vertex1;
		temp_triangle.vertex2 = patches.at(i).vertex2;
		triangles.push_back(temp_triangle);

		temp_triangle.vertex0 = patches.at(i).vertex2;
		temp_triangle.vertex1 = patches.at(i).vertex3;
		temp_triangle.vertex2 = patches.at(i).vertex0; 
	}
}