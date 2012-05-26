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
		calculateDirectionsNormals(seg);
		averageNormals(seg);
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
		direction.x = seg_point2->direction.x - seg_point2->direction.x;
		direction.y = seg_point2->direction.y - seg_point2->direction.y;
		direction.z = seg_point2->direction.z - seg_point2->direction.z;
		D3DXVec3Normalize(&direction,&direction);
	}
	else
	{
		seg_point1 = seg_point->parent;
		direction = seg_point1->direction;
	}
	seg_point->direction = direction;

	if(seg_point->parent != NULL)
	{
		seg_point1 = seg_point->parent;
		normal = seg_point1->direction + seg_point->direction;
		D3DXVec3Normalize(&normal,&normal);

	}
	else
	{
		normal = seg_point->direction; 
	}
	seg_point->normal = normal;

	if(seg_point->child != NULL)
	{
		calculateDirNorm(seg_point2);
	}
}

//Calculate the average Normals at the branchings -- Wie wird das berechnet wenn ein Segment mehrere Parents hat??
void Mesh:: averageNormals(Segment* seg)
{
	if((seg->points.size() != 0) && seg->children.size() != 0)
	{	
		SegmentPoint* node = seg->points.back();

		std::vector<Segment*> seg_list = seg->children;
		D3DXVECTOR3 average = node->normal;
		int average_counter = 1;
		for (int i = 0; i < seg_list.size(); i++)
		{
			if(seg_list.at(i) != NULL)
			{
				//Calculate dot product 

				if(seg_list.at(i)->points.size() != 0)
				{
					SegmentPoint* a = seg_list.at(i)->points.front();
					D3DXVECTOR3 normal = a->normal;
					D3DXVECTOR3 average2 = seg_list.at(i)->points.front()->normal;
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

		node->normal = average;
		for(int i = 0; i < seg_list.size();i++)
		{
			if(seg_list.at(i) != NULL)
			{
				//set all normals to the average normals and than call function again for all childs
				if(seg_list.at(i)->points.size() != 0)
				{
					seg_list.at(i)->points.front()->normal = average;
				}
				averageNormals(seg_list.at(i));
			}
		}
	}

	

}

//classify the segments in forward or backward -> Was passiert hier mit dem ersten Segement?
void Mesh:: classifySegments(Segment* seg)
{
	if(seg->points.size() != 0)
	{
		SegmentPoint* node = seg->points.front();

		
		D3DXVECTOR3 average = node->normal;
		D3DXVECTOR3 direction = node->direction;
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