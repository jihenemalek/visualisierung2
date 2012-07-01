//
//  Sampler.h
//  SmoothVesselTree
//

#import <Foundation/Foundation.h>

@class Segment;

@interface Sampler : NSObject

+ (void)sampleVesseltree:(Segment *)segment withAlpha:(float)alpha andBeta:(float)beta;

@end
