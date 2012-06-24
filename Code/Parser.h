//
//  Parser.h
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import <Foundation/Foundation.h>

@class Segment;

@interface Parser : NSObject

+ (Segment *)parseDocument:(NSString *)documentName;

@end
