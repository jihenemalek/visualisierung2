#include "Mesh.h"
#include <assert.h>

Mesh::Mesh(void)
{
}


Mesh::~Mesh(void)
{
}

void Mesh::calculateMesh(Root* root)
{
	Segment* seg = root->child;

	if(root->child != NULL)
	{


		//Calculate Normal + Direction for the first cross-section (control point)
		//Calculate Normals + Directions for all cross-sections (segment points)
		calculateDirectionsNormals(seg);
		//calculate Normal + Direction for last cross-section (control point)

		
		//Calculate average normals at the branchings

		averageNormals(seg);

		//Funktion
		
		writeSegments(seg);

		// Propagate up-vector along the whole tree
		//first choose up vector perpendicular to direction of first cross section of the first segment
		seg->startNode->upVector = D3DXVECTOR3(1, 1, 1);

		//Calculate remaining up-Vectors
		calculateUpVectors(seg);

		// Tile mesh for the first section of the root node
		this->tileTrivially(root->child->startNode, root->child->points.at(0));
		// Tile the rest of the tree
		this->tileTree(root->child);

		// Triangulate
		this->triangulate();
	}
	else
	{
		//HWND hwnd;
		//MessageBox(hwnd, L"no segments", L"Error", MB_OK);
	}
}


//Calculate Normals and Directions of the Segments
//This function is called for each segment
void Mesh::calculateDirectionsNormals(Segment* seg)
{
	if(seg->startNode != NULL)
	{
		//Calculate Normal + Direction for the first cross-section (control point)
		D3DXVECTOR3 direction;
		direction.x = seg->points.at(0)->position.x - seg->startNode->position.x;
		direction.y = seg->points.at(0)->position.y - seg ->startNode->position.y;
		direction.z = seg->points.at(0)->position.z - seg->startNode->position.z;

		D3DXVec3Normalize(&direction,&direction);

		seg->startNode->direction = direction;
		seg->startNode->normal = direction;

		if (seg->points.size() > 0) {
			SegmentPoint* node = seg->points.at(0);

			//Es muss erst mal die direction und Normale für die erste cross-section berechnet werden, weil sich die Normale mit dem Vorgänger zusammensetzt (control point!)
			D3DXVECTOR3 normal;
		
			if (seg->points.size() > 1) {
				direction = seg->points.at(1)->position - node->position;
			} else {
				direction = seg->endNode->position - node->position;
			}
		 
			D3DXVec3Normalize(&direction, &direction);
			seg->points.at(0)->direction = direction;

			normal = seg->startNode->direction + seg->points.at(0)->direction;
			D3DXVec3Normalize(&normal, &normal);
			seg->points.at(0)->normal = normal;

			//jetzt für die anderen:
			calculateDirNorm(node);
		}

		if(seg->endNode != NULL)
		{
			//calculate Normal + Direction for last cross-section (control point)
			if (seg->points.size() > 0) {
				seg->endNode->direction = seg->points.back()->direction;
			} else {
				seg->endNode->direction = seg->startNode->direction;
			}
			
			
			seg->endNode->normal = seg->endNode->direction;

		}
	}
	std::vector<Segment*> seg_list = seg->children;
	for (unsigned int i = 0; i < seg_list.size(); i++)
	{
		if(seg_list.at(i) != NULL)
		{
			calculateDirectionsNormals(seg_list.at(i));
		}
	}
}

//Here the normals and directions for each cross-section are calculated
void Mesh::calculateDirNorm(SegmentPoint* seg_point)
{
	D3DXVECTOR3 normal;
	D3DXVECTOR3 direction;
	SegmentPoint* seg_point1;
	SegmentPoint* seg_point2;
	
	
	if(seg_point->child != NULL)
	{
		seg_point2 = seg_point->child;
		direction.x = seg_point2->position.x - seg_point->position.x;
		direction.y = seg_point2->position.y - seg_point->position.y;
		direction.z = seg_point2->position.z - seg_point->position.z;
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

//Calculate the average Normals at the branchings -> nicht berücksichtigt dass ein Child mehrere Parents haben kann
void Mesh:: averageNormals(Segment* seg)
{
	
	if((seg->endNode != NULL) && (seg->children.size() != NULL))
	
	{	
		ControlPoint* end_node = seg->endNode;

		std::vector<Segment*> seg_list = seg->children;
		D3DXVECTOR3 average = end_node->normal;
		D3DXVECTOR3 normal1 = end_node->normal;
		
		float average_counter = 1;
		for (unsigned int i = 0; i < seg_list.size(); i++)

		{
			if(seg_list.at(i) != NULL)
			{
				//Calculate dot product 

				if(seg_list.at(i)->startNode != NULL)
				{
					ControlPoint* a = seg_list.at(i)->startNode;
					D3DXVECTOR3 normal2 = a->normal;
					FLOAT product = D3DXVec3Dot(&normal1,&normal2);
					if(product > 0) //take only positive results
					{

						average = average + normal2; 
						average_counter = average_counter + 1;

					}
				}
			}	
			average = average / average_counter;
		}

		D3DXVec3Normalize(&average,&average);
		end_node->normal = average;
		for (unsigned int i = 0; i < seg_list.size();i++)
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

	for (unsigned int i = 0; i < seg_list.size(); i++)
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
		//Über Segment iterieren
		D3DXVECTOR3 pu, position;
		pu = seg->startNode->position + seg->startNode->upVector;
		position = seg->points.at(0)->position;

		up_vector = calculateUp(pu,seg->startNode->direction,position,seg->points.at(0)->normal);
		seg->points.at(0)->upVector = up_vector;


		for (unsigned int i = 0; i < (seg->points.size()-1); i++)
		{
			pu = seg->points.at(i)->position + seg->points.at(i)->upVector;
			position = seg->points.at(i+1)->position;
		
			up_vector = calculateUp(pu,seg->points.at(i)->direction,position,seg->points.at(i+1)->normal);
			seg->points.at(i+1)->upVector = up_vector;
		}

		//letzer Control Point
		pu = seg->points.back()->position + seg->points.back()->upVector;
		position = seg->endNode->position;

		up_vector = calculateUp(pu,seg->points.back()->direction,position,seg->endNode->normal);
		seg->endNode->upVector = up_vector;

		//An jeden StartNode der Children weitergeben und die Funktion nochmals aufrufen
		if(seg->children.size() != 0)
		{
			for (unsigned int i = 0; i < seg->children.size(); i++)
			{
				Segment* seg2 = seg->children.at(i);
				pu = seg->endNode->position + seg->endNode->upVector;
				position = seg2->startNode->position;

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
	if (seg->processed) return;


	if (seg->points.size() > 0) {
		// For all non-branching sections (all points between startNode and endNode)
		for (unsigned int i = 0; i < seg->points.size() - 1; i++) {
			this->tileTrivially(seg->points.at(i), seg->points.at(i + 1));
		}

	}

	seg->processed = true;

	// Classify branching segments into forward and backward
	std::set<Segment *> forward;
	std::set<Segment *> backward;

	
	for (std::vector<Segment *>::iterator it = seg->children.begin(); it != seg->children.end(); it++) 
	{
		
		D3DXVec3Normalize(&seg->endNode->direction, &seg->endNode->direction);
		D3DXVec3Normalize(&(*it)->startNode->direction, &(*it)->startNode->direction);
		

		float dotProduct = D3DXVec3Dot(&seg->endNode->direction, &(*it)->startNode->direction);

		if (dotProduct > 0) 
		{
			backward.insert(*it);
		} 
		else
		{
			forward.insert(*it);
		}
	}


	if (seg->children.size() > 1) {
		seg = seg;
	}

	// Assert that the number all forward or backward branches must be equal to the children of the segment
	assert((forward.size() + backward.size()) == seg->children.size());
	
	// If no backward branches exist, tile trivially
	if (backward.size() == 0) {
		if (seg->points.size() == 0) {
			this->tileTrivially(seg->startNode, seg->endNode);
		} else {
			this->tileTrivially(seg->points.back(), seg->endNode);
		}
	} else {

		// Process backward pointing branches
		
		// Classify backward set into quadrants relative to last section
		std::set<Segment *> quadrants[4];
		D3DXVECTOR3 upVector[4];
		D3DXVECTOR3 avgUpVector[4];

		if (seg->points.size() == 0) {
			upVector[0] = seg->startNode->upVector;
			upVector[1] = this->rotateVector(upVector[0], seg->startNode->direction);
			upVector[2] = this->rotateVector(upVector[1], seg->startNode->direction);
			upVector[3] = this->rotateVector(upVector[2], seg->startNode->direction);
		} else {
			upVector[0] = seg->points.back()->upVector;
			upVector[1] = this->rotateVector(upVector[0], seg->points.back()->direction);
			upVector[2] = this->rotateVector(upVector[1], seg->points.back()->direction);
			upVector[3] = this->rotateVector(upVector[2], seg->points.back()->direction);
		}


		avgUpVector[0] = (upVector[0] + upVector[1]) / 2.0f;
		avgUpVector[1] = (upVector[1] + upVector[2]) / 2.0f;
		avgUpVector[2] = (upVector[2] + upVector[3]) / 2.0f;
		avgUpVector[3] = (upVector[3] + upVector[0]) / 2.0f;

		for (std::set<Vesseltree::Segment *>::iterator it = backward.begin(); it != backward.end(); it++) {
			for (unsigned int i = 0; i < 4; i++) {
				if (D3DXVec3Dot(&avgUpVector[i], &((*it)->startNode->direction)) < 1) {

					quadrants[i].insert(*it);
					break;
				}
			}
		}

		// Join backward segments to last section for each quadrant

		for (unsigned int i = 0; i < 4; i++) {
			this->tileJoint(quadrants[i], seg->endNode->direction, seg, avgUpVector[i]);

		}

		// Recursively generate subtrees of all backward segments
		for (std::set<Segment *>::iterator it = backward.begin(); it != backward.end(); it++) 
		{
			this->tileTree(*it);
		}
	}

	if (forward.size() > 0)
	{
		// Select straightest segment S in the forward set
		Segment *S;
		float minAngle = FLT_MAX;
		for (std::set<Segment *>::iterator it = forward.begin(); it != forward.end(); it++) 
		{
			float angle = D3DXVec3Dot(&((*it)->startNode->direction), &(seg->endNode->direction));
			if (angle < minAngle) 
			{
				minAngle = angle;
				S = *it;
			}
		}

		// Remove straightest segment S from the set
		forward.erase(S);

		// Classify remaining set into quadrants relative to S	
		std::set<Segment *> quadrants[4];
		D3DXVECTOR3 upVector[4];
		D3DXVECTOR3 avgUpVector[4];

		upVector[0] = S->startNode->upVector;
		upVector[1] = this->rotateVector(upVector[0], S->startNode->direction);
		upVector[2] = this->rotateVector(upVector[1], S->startNode->direction);
		upVector[3] = this->rotateVector(upVector[2], S->startNode->direction);
		

		avgUpVector[0] = (upVector[0] + upVector[1]) / 2.0f;
		avgUpVector[1] = (upVector[1] + upVector[2]) / 2.0f;
		avgUpVector[2] = (upVector[2] + upVector[3]) / 2.0f;
		avgUpVector[3] = (upVector[3] + upVector[0]) / 2.0f;

		for (std::set<Segment *>::iterator it = forward.begin(); it != forward.end(); it++) {
			for (unsigned int i = 0; i < 4; i++) {
				if (D3DXVec3Dot(&avgUpVector[i], &((*it)->startNode->direction)) < 1) {

					quadrants[i].insert(*it);
					break;
				}
			}
		}
		
		// Join all segments for each quadrant together

		for (unsigned int i = 0; i < 4; i++) {
			this->tileJoint(quadrants[i], S->startNode->direction, S, avgUpVector[i]);

		}
		
		if (forward.size() > 1) {
			S = S;
		}

		// Recursively generate subtrees for all forward segments

		//this->tileTree(S);		// S was removed from set and must be processed separately
		for (std::set<Segment *>::iterator it = forward.begin(); it != forward.end(); it++) {

			this->tileTree(*it);
		}
	}
}

void Mesh::tileTrivially(SegmentPoint *p1, SegmentPoint *p2)
{
	D3DXVECTOR3 upVector1 = p1->upVector;
	D3DXVECTOR3 upVector2 = p2->upVector;

	Patch patch[4];

	for (unsigned int i = 0; i < 3; i++) {
		patch[i].vertex0 = p1->position + (p1->radius * upVector1);

		upVector1 = rotateVector(upVector1, p1->direction);
		patch[i].vertex1 = p1->position + (p1->radius * upVector1);

		patch[i].vertex3 = p2->position + (p2->radius * upVector2);
		upVector2 = rotateVector(upVector2, p2->direction);
		patch[i].vertex2 = p2->position + (p2->radius * upVector2);
	}

	// Last section
	patch[3].vertex0 = p1->position + (p1->radius * upVector1);
	patch[3].vertex1 = p1->position + (p1->radius * p1->upVector);
	patch[3].vertex3 = p2->position + (p2->radius * upVector2);
	patch[3].vertex2 = p2->position + (p2->radius * p2->upVector);

	// Any special handling?

	// Add the patches to the list
	for (unsigned int i = 0; i < 4; i++) 
	{
		this->patches.push_back(patch[i]);
	}
}

//tile Trivially
void Mesh::tileTrivially(Segment* seg)
{
	D3DXVECTOR3 position, up_vector;
	float radius;
	std::vector<D3DXVECTOR3> vertex_list;

	//für segmente ausserhalb der Branchings berechnen
	if(seg->points.size() != 0)
	{
		for (unsigned int i = 0; i < seg->points.size(); i++)
		{
			position = seg->points.at(i)->position;

			radius = seg->points.at(i)->radius;

			up_vector = seg->points.at(i)->upVector;
			
			vertex_list = tileTrivial(position, radius, up_vector, seg->points.at(i)->direction);
			seg->points.at(i)->vertices = vertex_list;
		}

	}
	if(seg->children.size() != 0)
	{
		for (unsigned int i = 0; i < seg->children.size(); i++)
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

void Mesh::tileJoint(std::set<Segment *> segments, D3DXVECTOR3 direction, Segment *caller, D3DXVECTOR3 quadDirection)
{
	if (segments.size() == 0) {
		// Close this side of the quadrant with a simple patch

		D3DXVECTOR3 upVector[4];
		D3DXVECTOR3 pUpVector[4];
		upVector[0] = caller->startNode->upVector;
		upVector[1] = this->rotateVector(upVector[0], caller->startNode->direction);
		upVector[2] = this->rotateVector(upVector[1], caller->startNode->direction);
		upVector[3] = this->rotateVector(upVector[2], caller->startNode->direction);

		if (caller->type == Vesseltree::kSegmentTypeInterpolated) {
			pUpVector[0] = caller->endNode->upVector;
			pUpVector[1] = this->rotateVector(upVector[0], caller->endNode->direction);
			pUpVector[2] = this->rotateVector(upVector[1], caller->endNode->direction);
			pUpVector[3] = this->rotateVector(upVector[2], caller->endNode->direction);
		} else {
			pUpVector[0] = caller->points.front()->upVector;
			pUpVector[1] = this->rotateVector(pUpVector[0], caller->points.front()->direction);
			pUpVector[2] = this->rotateVector(pUpVector[1], caller->points.front()->direction);
			pUpVector[3] = this->rotateVector(pUpVector[2], caller->points.front()->direction);
		}

		for (unsigned int i = 0; i < 4; i++) {
			if (D3DXVec3Dot(&quadDirection, &upVector[i]) < 1 && D3DXVec3Dot(&quadDirection, &upVector[(i + 1) % 4])) {
				Patch p;

				p.vertex0 = caller->startNode->position + caller->startNode->radius * upVector[i];
				p.vertex1 = caller->startNode->position + caller->startNode->radius * upVector[(i + 1) % 4];
				
				if (caller->type == Vesseltree::kSegmentTypeInterpolated) {
					p.vertex2 = caller->endNode->position + caller->endNode->radius * pUpVector[(i + 1) % 4];
					p.vertex3 = caller->endNode->position + caller->endNode->radius * pUpVector[i];
				} else {
					p.vertex2 = caller->points.front()->position + caller->points.front()->radius * pUpVector[(i + 1) % 4];
					p.vertex3 = caller->points.front()->position + caller->points.front()->radius * pUpVector[i];
				}

				patches.push_back(p);
				break;
			}
		}
	} else {
		// Find the closest segment N in the set (smallest angle to direction)
		Segment *N;
		float minAngle = FLT_MAX;
		for (std::set<Segment *>::iterator it = segments.begin(); it != segments.end(); it++) {
			float angle = D3DXVec3Dot(&((*it)->startNode->direction), &direction);
			if (angle < minAngle) {
				minAngle = angle;
				N = *it;
			}
		}

		segments.erase(N);

		// Classify the remaining segments into quadrants relative to direction N
		std::set<Segment *> quadrants[3];
		D3DXVECTOR3 upVector[4];

		upVector[0] = N->startNode->upVector;
		upVector[1] = this->rotateVector(upVector[0], N->startNode->direction);
		upVector[2] = this->rotateVector(upVector[1], N->startNode->direction);
		upVector[3] = this->rotateVector(upVector[2], N->startNode->direction);

		D3DXVECTOR3 avgUpVector[3];
		avgUpVector[0] = (upVector[0] + upVector[1]) / 2.0f;
		avgUpVector[1] = (upVector[1] + upVector[2]) / 2.0f;
		avgUpVector[2] = (upVector[2] + upVector[3]) / 2.0f;

		for (std::set<Segment *>::iterator it = segments.begin(); it != segments.end(); it++) {
			for (unsigned int i = 0; i < 3; i++) {
				if (D3DXVec3Dot(&avgUpVector[i], &((*it)->startNode->direction)) < 1) {
					quadrants[i].insert(*it);
					break;
				}
			}
		}
		
		// Create a transition quadrilateral patch between S and N
		// TODO: Create patch

		for (unsigned int i = 0; i < 3; i++) {
			this->tileJoint(quadrants[i], N->startNode->direction, N, avgUpVector[i]);
		}
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
	for (unsigned int i = 0; i< 3; i++)
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
			for (unsigned int i = 1; i < seg->points.size(); i++)
			{
				/*//create Patch
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
				patches.push_back(temp_patch);*/
			}
		}
	}

	if(seg->children.size() != 0)
	{
		for (unsigned int i = 0; i < seg->children.size(); i++)
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
		for (unsigned int i = 0; i < seg->children.size(); i++)
		{
			if(seg->children.at(i)->classification = 0) //backward
			{
				backward_index.push_back(i);
			}
		}

		if(backward_index.size() == 0) //if no backward branches exist -> process tile trivial and create the patch of the endNode and the patches between the endNode and the last segment point
		{
			std::vector<D3DXVECTOR3> vertex_list;
			Patch temp_patch;
		
			vertex_list = tileTrivial(seg->endNode->position, seg->endNode->radius,seg->endNode->upVector, seg->endNode->direction);
			seg->endNode->vertices = vertex_list;

			/*//create Patches
			temp_patch.vertex0 = seg->endNode->vertices.at(0);
			temp_patch.vertex1 = seg->endNode->vertices.at(1);
			temp_patch.vertex2 = seg->endNode->vertices.at(2);
			temp_patch.vertex3 = seg->endNode->vertices.at(3);
			patches.push_back(temp_patch);*/

			if(seg->points.size() != 0)
			{
				/*//back side
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
				patches.push_back(temp_patch);*/
			}


		}
		else //here the backward segments have to be processed
		{
			//for each quadrant call function TielJoint

		}
	}

	if(seg->children.size() != 0)
	{
		for (unsigned int i = 0; i < seg->children.size(); i++)
		{
			processLastSections(seg->children.at(i));
		}
	}

}

//void Mesh:: TileJoint(std::vector<Segment*> seg_list, D3DXVECTOR3 direction)
//{
//	if(seg_list.size() == 0)
//	{
//		//closing patch
//	}
//	else
//	{
//		FLOAT product;
//		//take segment with minimal angle to direction
//		float angle = 400, temp_angle;
//		FLOAT a, b;
//		int index = 0;
//
//		//calculate segment with minimal angle -> index is calculated
//		for (unsigned int i = 0; i < seg_list.size(); i++)
//		{
//			product = D3DXVec3Dot(&direction,&seg_list.at(i)->startNode->direction);
//			a = D3DXVec3Length(&direction);
//			b = D3DXVec3Length(&(seg_list.at(i)->startNode->direction));
//			temp_angle = product/(a*b);
//			temp_angle = acos(temp_angle);
//
//			if(temp_angle < angle)
//			{
//				index = i;
//			}
//		}
//
//		//classify remaining segments into quadrants of N
//
//		//space between C and N is tiled by transition patch
//
//		//for the remaining 3 quadrants TileJoint is called
//		
//	}
//}

void Mesh::triangulate()
{
	Triangle temp_triangle;
	for (unsigned int i = 0; i < patches.size(); i++)
	{
		temp_triangle.vertex0 = patches.at(i).vertex0;
		temp_triangle.vertex1 = patches.at(i).vertex1;
		temp_triangle.vertex2 = patches.at(i).vertex2;
		triangles.push_back(temp_triangle);

		temp_triangle.vertex0 = patches.at(i).vertex2;
		temp_triangle.vertex1 = patches.at(i).vertex3;
		temp_triangle.vertex2 = patches.at(i).vertex0; 
		triangles.push_back(temp_triangle);
	}
}


//Funktion zum Ausgeben der Anzahl an cross-sections
void Mesh::writeSegments(Segment* seg)
{
	std::ofstream outfile;
	outfile.open("Ausgabe.txt",std::ios::app );
	
	if(seg->children.size() > 0)
	{
		outfile << "An diesem Braching sind " << seg->children.size() << " " << "Segmente, die weggehen"<< "\n";
		for(int i = 0; i < seg->children.size(); i++)
		{
			if(seg->children.at(i)->points.size() != 0)
			{
				outfile << "Anzahl Punkte: " << seg->points.size() << "\n";
			}
			else
			{
				outfile << "Segment ohne segmentpoints" << "\n";
				outfile << seg->startNode->position.x << " " << seg->startNode->position.y << " " << seg->startNode->position.z << "\n";
				outfile << seg->endNode->position.x << " " << seg->endNode->position.y << " " << seg->endNode->position.z << "\n";
			}
		}
		outfile.close();
		if(seg->children.size() != 0)
		{
			for(int i = 0; i < seg->children.size(); i++)
			{
				writeSegments(seg->children.at(i));
			}
		}
	}
}



 
