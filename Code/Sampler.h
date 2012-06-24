//
//  Sampler.h
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 24.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import <Foundation/Foundation.h>

@class Segment;

@interface Sampler : NSObject

+ (void)sampleVesseltree:(Segment *)segment withAlpha:(float)alpha andBeta:(float)beta;

@end
