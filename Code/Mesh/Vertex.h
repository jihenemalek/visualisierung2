//
//  Vertex.h
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 26.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "Triangle.h"

@interface Vertex : NSObject

@property (assign) NSUInteger idx;
@property (weak)   Triangle *triangle;

@end
