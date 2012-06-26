//
//  SegmentNode.h
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface SegmentNode : NSObject <NSCopying>

@property (assign) GLKVector3     position;
@property (assign) float          radius;
@property (assign) float          offset;

@property (weak)  SegmentNode     *prev;
@property (weak)  SegmentNode     *next;

@property (assign) float          curvature;

@property (assign) GLKVector3     direction;
@property (assign) GLKVector3     normal;
@property (assign) GLKVector3     up;

@property (strong) NSMutableArray *patches;

- (SegmentNode *)combineWithNode:(SegmentNode *)otherNode;

@end
