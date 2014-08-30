//
//  DoWork.h
//  DeXinet
//
//  Created by Tim on 3/9/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface DoWork : NSOperation {

	   NSString *directoryPath;
    BOOL        executing;
    BOOL        finished;
}

-(id)initWithString:(NSString *)data;
- (void)completeOperation;

@end
