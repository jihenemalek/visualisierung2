//
//  Vertex.h
//  SmoothVesselTree
//

#import <Foundation/Foundation.h>

#import "Triangle.h"

@interface Vertex : NSObject

@property (assign) NSUInteger idx;
@property (weak)   Triangle *triangle;

@end
