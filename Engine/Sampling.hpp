//
// Sampling.hpp
// Visualisierung2
// 
// Created by Markus Mühlberger on 10.06.12
// Copyright (c) 2012 Markus Mühlberger. All rights reserved.
//

#ifndef Visualisierung2_Sampling_hpp
#define Visualisierung2_Sampling_hpp

#include "Vesseltree/Tree.hpp"

class Sampling
{

public:

	static void downsample(Vesseltree::Root *tree, float alpha, float beta);

private:

	static void downsampleSegmentRecursive(Vesseltree::Segment *segment, float alpha, float beta);
	static float calculateG(Vesseltree::Node *point, float alpha, float beta);
	static float calculateDistance(Vesseltree::Node *point1, Vesseltree::Node *point2);
};

#endif