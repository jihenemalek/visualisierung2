//
//  Triangle.h
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Triangle : NSObject

@property (assign) BOOL tagged;

@property (nonatomic, strong) NSArray *subtriangles;

- (GLKVector3)vertexAtIndex:(NSUInteger)idx;
- (void)setVertex:(GLKVector3)vertex atIndex:(NSUInteger)idx;

- (GLKVector3)centerAtIndex:(NSUInteger)idx;
- (void)setCenter:(GLKVector3)center atIndex:(NSUInteger)idx;

- (float)curvatureAtIndex:(NSUInteger)idx;
- (void)setCurvature:(float)curvature atIndex:(NSUInteger)idx;

- (float)radiusAtIndex:(NSUInteger)idx;
- (void)setRadius:(float)radius atIndex:(NSUInteger)idx;

- (GLKVector3)normalAtIndex:(NSUInteger)idx;

- (__weak Triangle *)neighborAtIndex:(NSUInteger)idx;
- (void)setNeighbor:(__weak Triangle *)neighbor atIndex:(NSUInteger)idx;

- (NSArray *)subdivide2;
- (NSArray *)subdivide4;

@end
