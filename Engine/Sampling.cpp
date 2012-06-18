//
// Sampling.cpp
// Visualisierung2
// 
// Created by Markus Mühlberger on 10.06.12
// Copyright (c) 2012 Markus Mühlberger. All rights reserved.
//

#include "Sampling.hpp"

#include <math.h>

void Sampling::downsample(Vesseltree::Root *tree, float alpha, float beta) 
{
	Sampling::downsampleSegmentRecursive(tree->child, alpha, beta);
}

void Sampling::downsampleSegmentRecursive(Vesseltree::Segment *segment, float alpha, float beta) 
{
	std::vector<Vesseltree::SegmentPoint *> newPoints, oldPoints;
	std::vector<Vesseltree::SegmentPoint *> newPointsFront;
	std::vector<Vesseltree::SegmentPoint *> newPointsBack;

	oldPoints = segment->points;

	if (segment->points.size() > 0) 
	{
		int i = 0; 
		int j = segment->points.size() - 1;
		while(i < j) 
		{
			float fDistance, fWeight;
			float bDistance, bWeight;
			
			if (i == 0) 
			{
				fDistance = Sampling::calculateDistance(segment->startNode, segment->points.front());
				fWeight = Sampling::calculateG(segment->points.front(), alpha, beta) + Sampling::calculateG(segment->startNode, alpha, beta);
				bDistance = Sampling::calculateDistance(segment->points.back(), segment->endNode);
				bWeight = Sampling::calculateG(segment->points.back(), alpha, beta) + Sampling::calculateG(segment->endNode, alpha, beta);
			} 
			else 
			{
				fDistance = Sampling::calculateDistance(segment->points.at(i), segment->points.at(i + 1));
				fWeight = Sampling::calculateG(segment->points.at(i + 1), alpha, beta) + Sampling::calculateG(segment->points.at(i), alpha, beta);
				bDistance = Sampling::calculateDistance(segment->points.at(j), segment->points.at(j - 1));
				bWeight = Sampling::calculateG(segment->points.at(j - 1), alpha, beta) + Sampling::calculateG(segment->points.at(j), alpha, beta);
			}
			
			if (fDistance >= fWeight) 
			{
				newPointsFront.push_back(segment->points[i]);
				i++;
			} 
			else if(fWeight < 10000) 
			{
				int k = 2;

				while((i + k) < j) 
				{
					fDistance = Sampling::calculateDistance(segment->points.at(i + k), segment->points.at(i));
					fWeight = Sampling::calculateG(segment->points.at(i + k), alpha, beta) + Sampling::calculateG(segment->points.at(i), alpha, beta);

					if (fDistance >= fWeight) 
					{
						newPointsFront.push_back(segment->points.at(i + k - 1)->combine(segment->points.at(i + k)));
						break;
					} 
					else if(fWeight > 10000) 
					{
						break;
					}
					k++;
				}
				i += k;
			} 
			else 
			{
				i++;
			}
			
			if (bDistance >= bWeight) 
			{
				newPointsBack.push_back(segment->points[j]);
				j--;
			}
			else if (bWeight < 10000)
			{
				int k = 2;

				while((j - k) > i) 
				{
					bDistance = Sampling::calculateDistance(segment->points.at(j - k), segment->points.at(j));
					bWeight = Sampling::calculateG(segment->points.at(j - k), alpha, beta) + Sampling::calculateG(segment->points.at(j), alpha, beta);

					if (bDistance >= bWeight) 
					{
						newPointsBack.push_back(segment->points.at(j - k)->combine(segment->points.at(j - k + 1)));
						break;
					}
					k++;
				}
				j -= k;
			}
			else
			{
				j--;
			}
		}

		// Merge the two vectors to the new point set
		for (std::vector<Vesseltree::SegmentPoint *>::iterator it = newPointsFront.begin(); it != newPointsFront.end(); it++) 
		{
			if (newPoints.size() > 0) 
			{
				(*it)->parent = newPoints.back();
				newPoints.back()->child = (*it);
			}
			newPoints.push_back(*it);
		}

		for (std::vector<Vesseltree::SegmentPoint *>::reverse_iterator rit = newPointsBack.rbegin(); rit != newPointsBack.rend(); rit++) 
		{
			if (newPoints.size() > 0) 
			{
				(*rit)->parent = newPoints.back();
				newPoints.back()->child = (*rit);
			}
			newPoints.push_back(*rit);
		}

		// Set new points
		if (newPoints.size() > 0)
		{
			newPoints.front()->parent = segment->startNode;
			newPoints.back()->child = segment->endNode;
		}
		segment->points = newPoints;
	}

	// Sample rest of the tree
	for(std::vector<Vesseltree::Segment *>::iterator it = segment->children.begin(); it != segment->children.end(); it++)
	{
		Sampling::downsampleSegmentRecursive((*it), alpha, beta);
	}
}

float Sampling::calculateG(Vesseltree::Node *point, float alpha, float beta)
{

	float kappa = 0;

	if (point->child && point->parent) 
	{
		D3DXVECTOR3 AB = point->position - point->parent->position;
		D3DXVECTOR3 BC = point->child->position - point->position;
		D3DXVECTOR3 CA = point->parent->position - point->child->position;
		float a = D3DXVec3Length(&AB);
		float b = D3DXVec3Length(&BC);
		float c = D3DXVec3Length(&CA);
		float s = 0.5f * (a + b + c);

		if (a == 0 || b == 0 || c == 0) return 10000.0f;

		float x = fabs(s * (s - a) * (s - b) * (s - c));

		kappa = 4.0f * sqrt(x) / (a * b * c);
	}


	return (alpha * point->radius) / (1 + beta * kappa);
}

float Sampling::calculateDistance(Vesseltree::Node *point1, Vesseltree::Node *point2)
{
	D3DXVECTOR3 d = point2->position - point1->position;
	return D3DXVec3Length(&d);
}