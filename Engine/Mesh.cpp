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

		

		// Propagate up-vector along the whole tree
		//first choose up vector perpendicular to direction of first cross section of the first segment

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
		D3DXVECTOR3 direction = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		int i = 0;

		while(D3DXVec3Length(&direction) == 0)
		{
			if (i < seg->points.size())
			{
				direction = seg->points.at(i)->position - seg->startNode->position;
			} else 
			{
				direction = seg->endNode->position - seg->startNode->position;
			}
			i++;
		}

		D3DXVec3Normalize(&direction, &direction);

		seg->startNode->direction = direction;
		seg->startNode->normal = direction;
		
		if (seg->points.size() > 0) 
		{
			SegmentPoint* node = seg->points.at(0);

			//Es muss erst mal die direction und Normale für die erste cross-section berechnet werden, weil sich die Normale mit dem Vorgänger zusammensetzt (control point!)
			D3DXVECTOR3 normal;
		
			if (seg->points.size() > 1)
			{
				direction = seg->points.at(1)->position - node->position;
			} else 
			{
				direction = seg->endNode->position - node->position;
			}
		 
			D3DXVec3Normalize(&direction, &direction);
			seg->points.front()->direction = direction;

			normal = seg->startNode->direction + seg->points.front()->direction;
			D3DXVec3Normalize(&normal, &normal);
			seg->points.front()->normal = normal;

			//jetzt für die anderen:
			calculateDirNorm(node);
		}

		if(seg->endNode != NULL) 
		{
			//calculate Normal + Direction for last cross-section (control point)
			if (seg->points.size() > 0) 
			{
				seg->endNode->direction = seg->points.back()->direction;
			} 
			else 
			{
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
		direction = seg_point2->position - seg_point->position;
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
	D3DXVECTOR3 anorm = D3DXVECTOR3(0, 0, 1);
	if (fabs(seg->startNode->direction.x) < fabs(seg->startNode->direction.y)) 
	{
		anorm = D3DXVECTOR3(1, 0, 0);
	} 
	else if (fabs(seg->startNode->direction.y) < fabs(seg->startNode->direction.z)) 
	{
		anorm = D3DXVECTOR3(0, 1, 0);
	}

	D3DXVec3Cross(&seg->startNode->upVector, &anorm, &seg->startNode->direction);
	D3DXVec3Normalize(&seg->startNode->upVector, &seg->startNode->upVector);
	
	D3DXVECTOR3 up_vector;

	if(seg->points.size() != 0) 
	{
		//Über Segment iterieren
		D3DXVECTOR3 pu, position;
		pu = seg->startNode->position + seg->startNode->upVector;
		position = seg->points.front()->position;

		up_vector = calculateUp(pu, seg->startNode->direction, position, seg->points.front()->normal);
		seg->points.front()->upVector = up_vector;

		for (unsigned int i = 0; i < (seg->points.size() - 1); i++)
		{
			pu = seg->points.at(i)->position + seg->points.at(i)->upVector;
			position = seg->points.at(i+1)->position;
		
			up_vector = calculateUp(pu,seg->points.at(i)->direction, position, seg->points.at(i+1)->normal);
			seg->points.at(i+1)->upVector = up_vector;
		}

		//letzer Control Point
		pu = seg->points.back()->position + seg->points.back()->upVector;
		position = seg->endNode->position;

		up_vector = calculateUp(pu,seg->points.back()->direction, position,seg->endNode->normal);
		seg->endNode->upVector = up_vector;

		//An jeden StartNode der Children weitergeben und die Funktion nochmals aufrufen
		if(seg->children.size() != 0)
		{
			for (unsigned int i = 0; i < seg->children.size(); i++)
			{
				Segment* seg2 = seg->children.at(i);
				pu = seg->endNode->position + seg->endNode->upVector;
				position = seg2->startNode->position;

				up_vector = calculateUp(pu,seg->endNode->direction, position, seg2->startNode->normal);
				seg2->startNode->upVector = up_vector;
				calculateUpVectors(seg2);
			}
		}
	} 
	else 
	{
		// Calculate up-vector for interpolated segments
	}

}

//here the up-vector is calculated
D3DXVECTOR3 Mesh::calculateUp(D3DXVECTOR3 pu, D3DXVECTOR3 direction, D3DXVECTOR3 position, D3DXVECTOR3 normal)
{
	D3DXVECTOR3 temp = (position - pu);

	float d = D3DXVec3Dot(&temp, &normal) / D3DXVec3Dot(&direction, &normal);
	D3DXVECTOR3 pn = pu + (d * direction);
	D3DXVECTOR3 up_vector = pn - position;

	D3DXVec3Normalize(&up_vector,&up_vector);
	return up_vector;
}

//Tile tree algorithm
void Mesh::tileTree(Segment* seg)
{
	if (seg->processed) return;


	if (seg->points.size() > 0) 
	{
		// For all non-branching sections (all points between startNode and endNode)
		for (unsigned int i = 0; i < seg->points.size() - 1; i++) 
		{
			this->tileTrivially(seg->points.at(i), seg->points.at(i + 1));
		}

	}

	seg->processed = true;

	// Classify branching segments into forward and backward
	std::set<Segment *> forward;
	std::set<Segment *> backward;

	
	for (std::vector<Segment *>::iterator it = seg->children.begin(); it != seg->children.end(); it++) 
	{
		float dotProduct = D3DXVec3Dot(&seg->endNode->direction, &(*it)->startNode->direction);

		if (dotProduct < 0) 
		{
			backward.insert(*it);
		} 
		else
		{
			forward.insert(*it);
		}
	}


	if (seg->children.size() > 1) 
	{
		seg = seg;
	}

	// Assert that the number all forward or backward branches must be equal to the children of the segment
	assert((forward.size() + backward.size()) == seg->children.size());
	
	// If no backward branches exist, tile trivially
	if (backward.size() == 0)
	{
		if (seg->points.size() == 0) 
		{
			this->tileTrivially(seg->startNode, seg->endNode);
		} else 
		{
			this->tileTrivially(seg->points.back(), seg->endNode);
		}
	} 
	else 
	{

		// Process backward pointing branches
		
		// Classify backward set into quadrants relative to last section
		std::set<Segment *> quadrants[4];
		D3DXVECTOR3 upVector[4];
		D3DXVECTOR3 avgUpVector[4];

		if (seg->points.size() == 0)
		{
			upVector[0] = seg->startNode->upVector;
			upVector[1] = this->rotateVector(upVector[0], seg->startNode->direction);
			upVector[2] = this->rotateVector(upVector[1], seg->startNode->direction);
			upVector[3] = this->rotateVector(upVector[2], seg->startNode->direction);
		} 
		else 
		{
			upVector[0] = seg->points.back()->upVector;
			upVector[1] = this->rotateVector(upVector[0], seg->points.back()->direction);
			upVector[2] = this->rotateVector(upVector[1], seg->points.back()->direction);
			upVector[3] = this->rotateVector(upVector[2], seg->points.back()->direction);
		}


		avgUpVector[0] = (upVector[0] + upVector[1]) / 2.0f;
		avgUpVector[1] = (upVector[1] + upVector[2]) / 2.0f;
		avgUpVector[2] = (upVector[2] + upVector[3]) / 2.0f;
		avgUpVector[3] = (upVector[3] + upVector[0]) / 2.0f;

		for (std::set<Vesseltree::Segment *>::iterator it = backward.begin(); it != backward.end(); it++)
		{
			for (unsigned int i = 0; i < 4; i++) 
			{
				if (D3DXVec3Dot(&avgUpVector[i], &((*it)->startNode->direction)) < 1) 
				{

					quadrants[i].insert(*it);
					break;
				}
			}
		}

		// Join backward segments to last section for each quadrant

		for (unsigned int i = 0; i < 4; i++) 
		{
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
			if (fabs(1 - angle) < minAngle) 
			{
				minAngle = fabs(1 - angle);
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
		upVector[1] = this->rotateVector(upVector[0], S->startNode->normal);
		upVector[2] = this->rotateVector(upVector[1], S->startNode->normal);
		upVector[3] = this->rotateVector(upVector[2], S->startNode->normal);
		

		avgUpVector[0] = (upVector[0] + upVector[1]) / 2.0f;
		avgUpVector[1] = (upVector[1] + upVector[2]) / 2.0f;
		avgUpVector[2] = (upVector[2] + upVector[3]) / 2.0f;
		avgUpVector[3] = (upVector[3] + upVector[0]) / 2.0f;

		for (std::set<Segment *>::iterator it = forward.begin(); it != forward.end(); it++) 
		{
			for (unsigned int i = 0; i < 4; i++) 
			{
				if (D3DXVec3Dot(&avgUpVector[i], &((*it)->startNode->normal)) < 1) 
				{

					quadrants[i].insert(*it);
					break;
				}
			}
		}
		
		// Join all segments for each quadrant together

		for (unsigned int i = 0; i < 4; i++) 
		{
			this->tileJoint(quadrants[i], S->startNode->normal, S, avgUpVector[i]);
		}

		// Recursively generate subtrees for all forward segments

		this->tileTree(S);		// S was removed from set and must be processed separately
		for (std::set<Segment *>::iterator it = forward.begin(); it != forward.end(); it++) 
		{
			this->tileTree(*it);
		}
	}
}

void Mesh::tileTrivially(SegmentPoint *p1, SegmentPoint *p2)
{
	D3DXVECTOR3 upVector1 = p1->upVector;
	D3DXVECTOR3 upVector2 = p2->upVector;

	Patch patch[4];

	for (unsigned int i = 0; i < 3; i++) 
	{
		D3DXVECTOR3 normal = upVector1 + upVector2;
		
		patch[i].vertex0 = p1->position + (p1->radius * upVector1);
		patch[i].vertex0_mittelpunkt = p1->position;
		patch[i].vertex0_radius = p1->radius;

		upVector1 = rotateVector(upVector1, p1->normal);
		patch[i].vertex1 = p1->position + (p1->radius * upVector1);
		patch[i].vertex1_mittelpunkt = p1->position;
		patch[i].vertex1_radius = p1->radius;

		patch[i].vertex3 = p2->position + (p2->radius * upVector2);
		patch[i].vertex3_mittelpunkt = p2->position;
		patch[i].vertex3_radius = p2->radius;

		upVector2 = rotateVector(upVector2, p2->normal);
		patch[i].vertex2 = p2->position + (p2->radius * upVector2);
		patch[i].vertex2_mittelpunkt = p2->position;
		patch[i].vertex2_radius = p2->radius;

		patch[i].normal = (normal + upVector1 + upVector2) / 4.0;
	}

	// Last section
	patch[3].vertex0 = p1->position + (p1->radius * upVector1);
	patch[3].vertex0_mittelpunkt = p1->position;
	patch[3].vertex0_radius = p1->radius;
	patch[3].vertex1 = p1->position + (p1->radius * p1->upVector);
	patch[3].vertex1_mittelpunkt = p1->position;
	patch[3].vertex1_radius = p1->radius;
	patch[3].vertex3 = p2->position + (p2->radius * upVector2);
	patch[3].vertex3_mittelpunkt = p2->position;
	patch[3].vertex3_radius = p2->radius;
	patch[3].vertex2 = p2->position + (p2->radius * p2->upVector);
	patch[3].vertex2_mittelpunkt = p2->position;
	patch[3].vertex2_radius = p2->radius;
	patch[3].normal = (upVector1 + p1->upVector + upVector2 + p2->upVector) / 4.0f;
	// Any special handling?

	// Add the patches to the list
	for (unsigned int i = 0; i < 4; i++) 
	{
		this->patches.push_back(patch[i]);
	}
}

void Mesh::tileJoint(std::set<Segment *> segments, D3DXVECTOR3 direction, Segment *caller, D3DXVECTOR3 quadDirection)
{

	if (segments.size() == 0) 
	{
		// Close this side of the quadrant with a simple patch
		D3DXVECTOR3 pUpVector[4];	
		D3DXVECTOR3 upVector[4];
		upVector[0] = caller->startNode->upVector;
		upVector[1] = this->rotateVector(upVector[0], caller->startNode->normal);
		upVector[2] = this->rotateVector(upVector[1], caller->startNode->normal);
		upVector[3] = this->rotateVector(upVector[2], caller->startNode->normal);

		if (caller->type == Vesseltree::kSegmentTypeInterpolated || caller->points.size() == 0) 
		{
			pUpVector[0] = caller->endNode->upVector;
			pUpVector[1] = this->rotateVector(upVector[0], caller->endNode->normal);
			pUpVector[2] = this->rotateVector(upVector[1], caller->endNode->normal);
			pUpVector[3] = this->rotateVector(upVector[2], caller->endNode->normal);
		} 
		else 
		{
			pUpVector[0] = caller->points.front()->upVector;
			pUpVector[1] = this->rotateVector(pUpVector[0], caller->points.front()->normal);
			pUpVector[2] = this->rotateVector(pUpVector[1], caller->points.front()->normal);
			pUpVector[3] = this->rotateVector(pUpVector[2], caller->points.front()->normal);
		}

		for (unsigned int i = 0; i < 4; i++) 
		{
			float d1 = D3DXVec3Dot(&quadDirection, &upVector[i]);
			float d2 = D3DXVec3Dot(&quadDirection, &upVector[(i + 1) % 4]);

			if (d1 > 0 && d2 > 0) 
			{
				Patch p;

				p.vertex0 = caller->startNode->position + caller->startNode->radius * upVector[i];
				p.vertex0_mittelpunkt = caller->startNode->position;
				p.vertex0_radius = caller->startNode->radius;
				p.vertex1 = caller->startNode->position + caller->startNode->radius * upVector[(i + 1) % 4];
				p.vertex1_mittelpunkt = caller->startNode->position;
				p.vertex1_radius = caller->startNode->radius;
				
				if (caller->type == Vesseltree::kSegmentTypeInterpolated || caller->points.size() == 0) 
				{
					p.vertex2 = caller->endNode->position + caller->endNode->radius * pUpVector[(i + 1) % 4];
					p.vertex2_mittelpunkt = caller->endNode->position;
					p.vertex2_radius = caller->endNode->radius;
					p.vertex3 = caller->endNode->position + caller->endNode->radius * pUpVector[i];
					p.vertex3_mittelpunkt = caller->endNode->position;
					p.vertex3_radius = caller->endNode->radius;
				} 
				else 
				{
					p.vertex2 = caller->points.front()->position + caller->points.front()->radius * pUpVector[(i + 1) % 4];
					p.vertex2_mittelpunkt = caller->points.front()->position;
					p.vertex2_radius = caller->points.front()->radius;
					p.vertex3 = caller->points.front()->position + caller->points.front()->radius * pUpVector[i];
					p.vertex3_mittelpunkt = caller->points.front()->position;
					p.vertex3_radius = caller->points.front()->radius;
				}

				p.normal = (upVector[i] + upVector[(i + 1) % 4] + pUpVector[i] + pUpVector[(i + 1) % 4]) / 4.0f;

				patches.push_back(p);
				break;
			}
		}
	} 
	else 
	{
		// Find the closest segment N in the set (smallest angle to direction)
		Segment *N;
		float minAngle = FLT_MAX;
		for (std::set<Segment *>::iterator it = segments.begin(); it != segments.end(); it++) 
		{
			float angle = D3DXVec3Dot(&((*it)->startNode->direction), &direction);
			if (angle < minAngle) 
			{
				minAngle = angle;
				N = *it;
			}
		}

		segments.erase(N);

		// Classify the remaining segments into quadrants relative to direction N
		std::set<Segment *> quadrants[3];
		D3DXVECTOR3 NUpVector[4];

		NUpVector[0] = N->startNode->upVector;
		NUpVector[1] = this->rotateVector(NUpVector[0], N->startNode->normal);
		NUpVector[2] = this->rotateVector(NUpVector[1], N->startNode->normal);
		NUpVector[3] = this->rotateVector(NUpVector[2], N->startNode->normal);

		D3DXVECTOR3 avgUpVector[3];
		avgUpVector[0] = (NUpVector[0] + NUpVector[1]) / 2.0f;
		avgUpVector[1] = (NUpVector[1] + NUpVector[2]) / 2.0f;
		avgUpVector[2] = (NUpVector[2] + NUpVector[3]) / 2.0f;

		for (std::set<Segment *>::iterator it = segments.begin(); it != segments.end(); it++) 
		{
			for (unsigned int i = 0; i < 3; i++) 
			{
				if (D3DXVec3Dot(&avgUpVector[i], &((*it)->startNode->direction)) < 1) 
				{
					quadrants[i].insert(*it);
					break;
				}
			}
		}
		
		// Create a transition quadrilateral patch between S and N
		D3DXVECTOR3 v0, v1, v2, v3;
		if (N->points.size() > 0) {
			D3DXVECTOR3 aUpVector[4];
			aUpVector[0] = caller->points.front()->upVector;
			aUpVector[1] = this->rotateVector(aUpVector[0], caller->points.front()->normal);
			aUpVector[2] = this->rotateVector(aUpVector[1], caller->points.front()->normal);
			aUpVector[3] = this->rotateVector(aUpVector[2], caller->points.front()->normal);

			for (int i = 0; i < 4; i++) {
				float d1 = D3DXVec3Dot(&quadDirection, &aUpVector[i]);
				float d2 = D3DXVec3Dot(&quadDirection, &aUpVector[(i + 1) % 4]);

				if (d1 >= 0 && d2 >= 0) {
					v0 = caller->points.front()->position + (caller->points.front()->radius * aUpVector[i]);
					v1 = caller->points.front()->position + (caller->points.front()->radius * aUpVector[(i + 1) % 4]);
				}
			}

			D3DXVECTOR3 aNUpVector[4];
			aNUpVector[0] = N->points.front()->upVector;
			aNUpVector[1] = this->rotateVector(aNUpVector[0], N->points.front()->normal);
			aNUpVector[2] = this->rotateVector(aNUpVector[1], N->points.front()->normal);
			aNUpVector[3] = this->rotateVector(aNUpVector[2], N->points.front()->normal);
		
			for (int i = 0; i < 4; i++) {
				float d1 = D3DXVec3Dot(&quadDirection, &aNUpVector[i]);
				float d2 = D3DXVec3Dot(&quadDirection, &aNUpVector[(i + 1) % 4]);

				if (d1 >= 0 && d2 >= 0) {
					v3 = N->points.front()->position + (N->points.front()->radius * aNUpVector[i]);
					v2 = N->points.front()->position + (N->points.front()->radius * aNUpVector[(i + 1) % 4]);
				}
			}
	
			Patch p;
			p.vertex0 = v0;
			p.vertex0_mittelpunkt = caller->points.front()->position;
			p.vertex0_radius = caller->points.front()->radius;
			p.vertex1 = v1;
			p.vertex1_mittelpunkt = caller->points.front()->position;
			p.vertex1_radius = caller->points.front()->radius;
			p.vertex2 = v2;
			p.vertex2_mittelpunkt = N->points.front()->position;
			p.vertex2_radius = N->points.front()->radius;
			p.vertex3 = v3;
			p.vertex3_mittelpunkt = N->points.front()->position;
			p.vertex3_radius = N->points.front()->radius;
			patches.push_back(p);
		}

		// Tile all other branches
		for (unsigned int i = 0; i < 3; i++) 
		{
			this->tileJoint(quadrants[i], N->startNode->direction, N, avgUpVector[i]);
		}
	}
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
		temp_triangle.normal = patches.at(i).normal;

		temp_triangle.vertex0_mittelpunkt = patches.at(i).vertex0_mittelpunkt;
		temp_triangle.vertex1_mittelpunkt = patches.at(i).vertex1_mittelpunkt;
		temp_triangle.vertex2_mittelpunkt = patches.at(i).vertex2_mittelpunkt;

		temp_triangle.vertex0_radius = patches.at(i).vertex0_radius;
		temp_triangle.vertex1_radius = patches.at(i).vertex1_radius;
		temp_triangle.vertex2_radius = patches.at(i).vertex2_radius;
		
		triangles.push_back(temp_triangle);

		temp_triangle.vertex0 = patches.at(i).vertex2;
		temp_triangle.vertex1 = patches.at(i).vertex3;
		temp_triangle.vertex2 = patches.at(i).vertex0; 
		temp_triangle.normal = patches.at(i).normal;

		temp_triangle.vertex0_mittelpunkt = patches.at(i).vertex2_mittelpunkt;
		temp_triangle.vertex1_mittelpunkt = patches.at(i).vertex3_mittelpunkt;
		temp_triangle.vertex2_mittelpunkt = patches.at(i).vertex0_mittelpunkt;

		temp_triangle.vertex0_radius = patches.at(i).vertex2_radius;
		temp_triangle.vertex1_radius = patches.at(i).vertex3_radius;
		temp_triangle.vertex2_radius = patches.at(i).vertex0_radius;


	
		triangles.push_back(temp_triangle);
	}
}






 
