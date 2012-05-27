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
		calculateUpVectors(seg);

	
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

//Calculate the average Normals at the branchings -- Wie wird das berechnet wenn ein Segment mehrere Parents hat??
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

void Mesh::calculateUpVectors(Segment* seg)
{
	D3DXVECTOR3 up_vector;
	if(seg->points.size()!=0)
	{
		//Über Segment iterieren
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