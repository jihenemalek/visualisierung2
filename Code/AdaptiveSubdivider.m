//
//  AdaptiveSubdivider.m
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 26.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import "AdaptiveSubdivider.h"

#import "Triangle.h"
#import "Vertex.h"

@interface AdaptiveSubdivider ()

@property (assign) float threshold;
@property (assign) NSUInteger maxIterations;

- (NSArray *)subdivideMesh:(NSArray *)mesh;

@end

@implementation AdaptiveSubdivider

@synthesize threshold = _threshold;
@synthesize maxIterations = _maxIterations;

+ (NSArray *)subdivideMesh:(NSArray *)mesh withThreshold:(float)threshold maximumIterations:(NSUInteger)maxIter
{
  if (!mesh || [mesh count] == 0) return nil;
  
  AdaptiveSubdivider *divider = [AdaptiveSubdivider new];
  divider.threshold = threshold;
  divider.maxIterations = maxIter;
  
  return [divider subdivideMesh:mesh];
}

- (NSArray *)subdivideMesh:(NSArray *)mesh
{

  // A - If converted mesh is too coarse, subdivide it by one level to get more accurate curvature estimation
  //

  // B - Estimate the curvature for each vertex - Curvatures are equalized by histogram equalization after scaling the curvature ranging between 0 and 1
  //
  for (Triangle *triangle in mesh) {
    for (NSUInteger i = 0; i < 3; i++) {
      // 1 - Determine the neighboring points. This can be a 1-ring or 2-ring neighborhood
      NSMutableArray *neighboringVertices = [NSMutableArray array];
      NSMutableArray *neighboringTriangles = [NSMutableArray arrayWithObject:triangle];
      
      // Add the two neighboring triangles
      [neighboringTriangles addObject:[triangle neighborAtIndex:i]];
      [neighboringTriangles addObject:[triangle neighborAtIndex:(i + 1) % 3]];
      
      // 2 - Compute the normal N at x_i and hence the plane P passing through the vertex (for now skip this)
      GLKVector3 weightedNormal = [triangle normalAtIndex:i];
      
      // 3 - Define an orthonormal coordinate system in P with x_i as its origin and two arbritrary unit vectors u, v in P
      GLKVector3 origin = [triangle vertexAtIndex:0];
      GLKVector3 u = GLKVector3CrossProduct(weightedNormal, GLKVector3Make(0, 1, 0));
      GLKVector3 v = GLKVector3CrossProduct(weightedNormal, u);
      
      // 4 - Compute the distance of all vertices in the neighborhood from x_i
      NSMutableArray *computedDistances = [NSMutableArray arrayWithCapacity:[neighboringVertices count]];
      for (Vertex *v in neighboringVertices) {
        [computedDistances addObject:[NSNumber numberWithFloat:GLKVector3Distance(origin, [v.triangle vertexAtIndex:v.idx])]];
      }
      
      // 5 - Project all points in the neighborhood on the plane P and represent their projections with respect to the local coordinate system in P.
      
      
      // 6 - Interpret the projections of each point in P as abscissa (on plane) and the distances as ordinate values (d_i)
      
      
      // 7 - Construct a bivariate polynomial f by solving a least squares system
      
      
      // 8 - Compute the principal curvatures of f's graph at xi
      //        f(u,v) = 0.5 * (C_2,0 * u^2_j + 2 * C_1,1 * u_j * v_j + C_0,2 * v^2_j)
      /*  
       [  u^2_1     2 * u_1 * v_1     v^2_1  ]   [ C_2,0 ]       [  d_1  ]
       [   ...          ....           ...   ] * [ C_1,1 ] = d = [  ...  ]
       [ u^2_n_i  2 * u_n_i * v_n_i  v^2_n_i ]   [ C_0,2 ]       [ d_n_i ] 
       */
      // Solve for k: k^2 - (C_2,0 + C_0,2) * k + C_2,0 * C_0,2 - C^2_1,1 = 0
    }
  }
  
}

@end
