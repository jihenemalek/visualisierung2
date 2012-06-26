//
//  AdaptiveSubdivider.h
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 26.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface AdaptiveSubdivider : NSObject

+ (NSArray *)subdivideMesh:(NSArray *)mesh withThreshold:(float)threshold maximumIterations:(NSUInteger)maxIter;

@end
