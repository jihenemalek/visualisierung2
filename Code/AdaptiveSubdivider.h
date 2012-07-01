//
//  AdaptiveSubdivider.h
//  SmoothVesselTree
//

#import <Foundation/Foundation.h>

@interface AdaptiveSubdivider : NSObject

+ (NSArray *)subdivideMesh:(NSArray *)mesh withThreshold:(float)threshold maximumIterations:(NSUInteger)maxIter;

@end
