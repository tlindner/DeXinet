//
//  TLAppDelegate.h
//  DeXinet
//
//  Created by tim lindner on 3/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TLAppDelegate : NSObject {
    NSWindow *window;
	NSWindow *logWindow;
	NSProgressIndicator *progress;
	NSTextView *logView;
	NSTextField *timeView;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSWindow *logWindow;
@property (assign) IBOutlet NSTextView *logView;
@property (assign) IBOutlet NSTextField *timeView;
@property (assign) IBOutlet NSProgressIndicator *progress;

- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames;
- (void)incrementProgress:(id)unused;
- (void)setMaxProgress:(id)max;
- (void)progressStart:(id)Unused;
- (void)progressDone:(id)unused;
- (void)logString: (NSString *) log;

@end
