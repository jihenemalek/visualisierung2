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