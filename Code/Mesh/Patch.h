//
//  Patch.h
//  SmoothVesselTree
//

#import <Foundation/Foundation.h>

@interface Patch : NSObject

@property (nonatomic, strong) NSArray *subpatches;
@property (nonatomic, strong) NSArray *triangles;

- (GLKVector3)vertexAtIndex:(NSUInteger)idx;
- (void)setVertex:(GLKVector3)vertex atIndex:(NSUInteger)idx;

- (GLKVector3)centerAtIndex:(NSUInteger)idx;
- (void)setCenter:(GLKVector3)center atIndex:(NSUInteger)idx;

- (float)curvatureAtIndex:(NSUInteger)idx;
- (void)setCurvature:(float)curvature atIndex:(NSUInteger)idx;

- (float)radiusAtIndex:(NSUInteger)idx;
- (void)setRadius:(float)radius atIndex:(NSUInteger)idx;

- (GLKVector3)normalAtIndex:(NSUInteger)idx;

- (__weak Patch *)neighborAtIndex:(NSUInteger)idx;
- (void)setNeighbor:(__weak Patch *)neighbor atIndex:(NSUInteger)idx;


- (NSArray *)subdivide:(NSUInteger)times;
- (NSArray *)triangulate;

@end
