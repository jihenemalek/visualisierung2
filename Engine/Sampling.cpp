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

	if (segment->points.size() > 0) {
		for (int i = 0, j = segment->points.size() - 1; i <= j; i++, j--) {
			float fd, bd, fg, bg;
			if (i == 0) {
				fd = Sampling::calculateDistance(segment->startNode, segment->points[i]);
				bd = Sampling::calculateDistance(segment->endNode, segment->points[j]);
				fg = Sampling::calculateG(segment->points[i], alpha, beta) + Sampling::calculateG(segment->startNode, alpha, beta);
				bg = Sampling::calculateG(segment->points[j], alpha, beta) + Sampling::calculateG(segment->endNode, alpha, beta);
			} else {
				fd = Sampling::calculateDistance(segment->points[i + 1], segment->points[i]);
				bd = Sampling::calculateDistance(segment->points[j - 1], segment->points[j]);
				fg = Sampling::calculateG(segment->points[i + 1], alpha, beta) + Sampling::calculateG(segment->points[i], alpha, beta);
				bg = Sampling::calculateG(segment->points[j - 1], alpha, beta) + Sampling::calculateG(segment->points[j], alpha, beta);
			}

			// Sample from front
			if (fd < fg) {
				for (i++; i < j; i++) {
					fd = Sampling::calculateDistance(segment->points[i + 1], segment->points[i]);
					fg = Sampling::calculateG(segment->points[i + 1], alpha, beta) + Sampling::calculateG(segment->points[i], alpha, beta);

					if (fd >= fg) {
						newPointsFront.push_back(segment->points[i - 1]->combine(segment->points[i]));
						break;
					}
				}
			} else {
				newPointsFront.push_back(segment->points[i]);
			}

			// Sample from back
			if (bd < bg) {
				for (j--; i < j; j--) {

					bd = Sampling::calculateDistance(segment->points[j - 1], segment->points[j]);
					bg = Sampling::calculateG(segment->points[j], alpha, beta) + Sampling::calculateG(segment->points[j + 1], alpha, beta);

					if (bd >= bg) {
						newPointsBack.push_back(segment->points[j]->combine(segment->points[j + 1]));
						break;
					}
				}
			} else {
				newPointsBack.push_back(segment->points[j]);
			}

			if (Sampling::calculateDistance(segment->points[i], segment->points[j]) <= Sampling::calculateG(segment->points[i], alpha, beta) + Sampling::calculateG(segment->points[j], alpha, beta)) {
				break;
			}
		}

		// Merge the two vectors to the new point set
		for (std::vector<Vesseltree::SegmentPoint *>::iterator it = newPointsFront.begin(); it != newPointsFront.end(); it++) {
			newPoints.push_back(*it);
		}
		for (std::vector<Vesseltree::SegmentPoint *>::reverse_iterator rit = newPointsBack.rbegin(); rit != newPointsBack.rend(); rit++) {
			newPoints.push_back(*rit);
		}

		// Set new points
		segment->points = newPoints;
	}

	// Sample rest of the tree
	for(std::vector<Vesseltree::Segment *>::iterator it = segment->children.begin(); it != segment->children.end(); it++) {
		Sampling::downsampleSegmentRecursive((*it), alpha, beta);
	}
}

float Sampling::calculateG(Vesseltree::Node *point, float alpha, float beta)
{

	float kappa = 0;

	if (point->child != NULL && point->parent != NULL) 
	{
		D3DXVECTOR3 AB = point->position - point->parent->position;
		D3DXVECTOR3 BC = point->child->position - point->position;
		D3DXVECTOR3 CA = point->parent->position - point->child->position;
		float a = D3DXVec3Length(&AB);
		float b = D3DXVec3Length(&BC);
		float c = D3DXVec3Length(&CA);
		float s = 0.5f * (a + b + c);

		kappa = 4.0f * sqrt(fabs(s * (s - a) * (s - b) * (s - c))) / (a * b * c);
	}


	return (alpha * point->radius) / (1 + beta * kappa);
}

float Sampling::calculateDistance(Vesseltree::Node *point1, Vesseltree::Node *point2)
{
	D3DXVECTOR3 d = point2->position - point1->position;
	return D3DXVec3Length(&d);
}