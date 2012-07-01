//
//  Mesher.h
//  SmoothVesselTree
//

#import <Foundation/Foundation.h>

#import "Mesh.h"
#import "Vesseltree.h"

@interface Mesher : NSObject

+ (NSArray *)generateQuadrilateralMesh:(Segment *)rootSegment;

//
// These methods do not alter the generated base mesh, but emit new objects
//

+ (NSArray *)subdivideQuadrilateralMesh:(NSArray *)mesh times:(NSUInteger)times;

+ (NSArray *)triangulateQuadrilateralMesh:(NSArray *)mesh;

@end
