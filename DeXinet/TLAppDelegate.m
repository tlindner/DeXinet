//
//  TLAppDelegate.m
//  DeXinet
//
//  Created by tim lindner on 3/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "TLAppDelegate.h"
#import "DoWork.h"

NSOperationQueue* aQueue;
NSDate *dateStarted;

@implementation TLAppDelegate

@synthesize window;
@synthesize logView;
@synthesize progress;
@synthesize logWindow;
@synthesize timeView;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 
}

- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames
{
	
	[progress setMaxValue:[filenames count]];
	
	if( aQueue == nil ) aQueue = [[NSOperationQueue alloc] init];
	[aQueue setMaxConcurrentOperationCount:1];
	[aQueue setSuspended:YES];
	
	for (NSString *filename in filenames)
	{
		DoWork *theOp = [[[DoWork alloc] initWithString:filename] autorelease];
		
		[aQueue addOperation:theOp];
	}
	
	[aQueue setSuspended:NO];
}

- (void)setMaxProgress:(id)max
{
	NSNumber *number = max;
	
	[progress setMaxValue:[number doubleValue]];
	[progress setDoubleValue:0.0];
}

- (void)incrementProgress:(id)unused
{
	[progress incrementBy:1.0];	
}

- (void)progressStart:(id)Unused
{
	[dateStarted release];
	dateStarted = [[NSDate alloc] init];
	[timeView setStringValue:@"Calculating…"];
	
	[self performSelector:@selector(updateRemainingTime:) withObject:nil afterDelay:1.5];
}

- (void)progressDone:(id)unused
{
	[progress setDoubleValue:[progress maxValue]];
	[dateStarted release];
	dateStarted = nil;
}

- (void)logString: (NSString *) log
{
	[[logView textStorage] appendAttributedString:[[[NSAttributedString alloc] initWithString:log] autorelease]];
	
	if( logView.textStorage.length > 1 )
	{
		NSRange range = NSMakeRange(logView.textStorage.length - 1, 1);
		[logView scrollRangeToVisible:range];
	}
}

- (void)updateRemainingTime:(id)unused
{
	NSString *unit = @"seconds";
	NSTimeInterval checkAgain = 0.75;
	
	if( [progress doubleValue] == [progress maxValue] )
	{
		[timeView setStringValue:@"Done."];
		return;
	}
	
	double count = [progress maxValue];
	double current = [progress	doubleValue];
	double percentDone = current/count;
	double timeTaken = [dateStarted timeIntervalSinceNow];
	double totalTime = timeTaken/percentDone;
	double timeLeft = timeTaken - totalTime;
	
	if( timeLeft > 60 * 60 * 24 )
	{
		unit = @"days";
		checkAgain = 60 * 60;
		timeLeft /= 60 * 60 * 12;
		if( timeLeft < 1.5f ) unit = @"day";
	}
	else if ( timeLeft > 60 * 60 )
	{
		unit = @"hours";
		checkAgain = 60;
		timeLeft /= 60 * 60;
		if( timeLeft < 1.5f ) unit = @"hour";
	}
	
	else if ( timeLeft > 60 )
	{
		unit = @"minutes";
		checkAgain = 1;
		timeLeft /= 60;
		
		if( timeLeft < 1.5f ) unit = @"minute";
	}
	else
	{
		if( timeLeft < 1.5f ) unit = @"second";

	}
	
	[timeView setStringValue:[NSString stringWithFormat:@"%.0f %@ remaining", timeLeft, unit]];
	
	[self performSelector:@selector(updateRemainingTime:) withObject:nil afterDelay:checkAgain];
	
}

@end
