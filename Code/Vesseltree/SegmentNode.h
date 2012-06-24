//
//  SegmentNode.h
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface SegmentNode : NSObject <NSCopying>

@property         GLKVector3      position;
@property         float           radius;
@property         float           offset;

@property (weak)  SegmentNode     *prev;
@property (weak)  SegmentNode     *next;

@property         GLKVector3      direction;
@property         GLKVector3      normal;
@property         GLKVector3      up;

@property         NSMutableArray  *patches;

- (SegmentNode *)combineWithNode:(SegmentNode *)otherNode;

@end
