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
	std::vector<Vesseltree::SegmentPoint *> newPointsFront;
	std::vector<Vesseltree::SegmentPoint *> newPointsBack;

	for (unsigned int i = 0, j = segment->points.size() - 1; i >= j; i++, j--) {
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
		if (fd > fg) {
			for (i++; i < j; i++) {
				fd = Sampling::calculateDistance(segment->points[i + 1], segment->points[i]);
				fg = Sampling::calculateG(segment->points[i + 1], alpha, beta) + Sampling::calculateG(segment->points[i], alpha, beta);

				if (fd <= fg) {
					newPointsFront.push_back(segment->points[i - 1]->combine(segment->points[i]));
					break;
				}
			}
		} else {
			newPointsFront.push_back(segment->points[i]);
		}

		// Sample from back
		if (bd > bg) {
			for (j--; i < j; j--) {

				bd = Sampling::calculateDistance(segment->points[j - 1], segment->points[j]);
				bg = Sampling::calculateG(segment->points[j], alpha, beta) + Sampling::calculateG(segment->points[j + 1], alpha, beta);

				if (bd <= bg) {
					newPointsFront.push_back(segment->points[j]->combine(segment->points[j + 1]));
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

	// TODO: Merge the two vectors to the new point set
}

float Sampling::calculateG(Vesseltree::Node *point, float alpha, float beta)
{
	float kappa = 1.0;
	// TODO: Calculate real curvature
	return (alpha * point->radius) / (1 + beta * kappa);
}

float Sampling::calculateDistance(Vesseltree::Node *point1, Vesseltree::Node *point2)
{
	D3DXVECTOR3 d = point2->position - point1->position;
	return D3DXVec3Length(&d);
}