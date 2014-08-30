//
//  DoWork.m
//  DeXinet
//
//  Created by Tim on 3/9/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import "DoWork.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <ftw.h>

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
int rmrf(char *path);
void logString( NSString *log );
void logwindowf( char *formatString, ... );
int DeXinet( const char *path );

@implementation DoWork

- (id)init {
    self = [super init];
    if (self) {
        executing = NO;
        finished = NO;
    }
    return self;
}

- (id)initWithString:(NSString *)data {
	if (self = [super init])
		directoryPath = [data retain];
	return self;
}

- (void)dealloc {
	[directoryPath release];
	[super dealloc];
}

- (void)start {
	
	// Always check for cancellation before launching the task.
	if ([self isCancelled])
	{
		// Must move the operation to the finished state if it is canceled.
		[self willChangeValueForKey:@"isFinished"];
		finished = YES;
		[self didChangeValueForKey:@"isFinished"];
		return;
	}
	
	// If the operation is not canceled, begin executing the task.
	[self willChangeValueForKey:@"isExecuting"];
	[NSThread detachNewThreadSelector:@selector(main) toTarget:self withObject:nil];
	executing = YES;
	[self didChangeValueForKey:@"isExecuting"];
}

-(void)main {
	@try {
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        char delete_file[8192];
        int err;
        
		logString( @"------------------------------\n" );
		
		BOOL isDirectory = NO;
		[[NSFileManager defaultManager] fileExistsAtPath:directoryPath isDirectory:&isDirectory];
		
		if( isDirectory )
		{
			logString( [NSString stringWithFormat:@"Counting children of %@.\n", directoryPath]);
			char *path[2];
            path[0] = (char *)[directoryPath cStringUsingEncoding:(NSUTF8StringEncoding)];
            path[1] = NULL;
            
			/* Count Files */
            FTS *fts = fts_open(path, FTS_NOCHDIR|FTS_XDEV, NULL);
            FTSENT *fts_entry;
			unsigned int file_count = 0;
			
            while ((fts_entry = fts_read(fts)) != 0 )
            {
                if( fts_entry->fts_info == FTS_D )
                {
                    if( strcmp( fts_entry->fts_name, ".HSResource" ) == 0 )
                    {
                        fts_set(fts, fts_entry, FTS_SKIP);
                    }
                }
                else
                {
                    if( strcmp( fts_entry->fts_name, ".HSancillary" ) == 0 ) continue;
                    if( strcmp( fts_entry->fts_name, ".HSxmap" ) == 0 ) continue;
                    if( strcmp( fts_entry->fts_name, ".HSicon" ) == 0 ) continue;
                    if( strcmp( fts_entry->fts_name, ".HSResource" ) == 0 ) continue;
                    if( strcmp( fts_entry->fts_name, ".HSiconlow" ) == 0 ) continue;
                    if( strcmp( fts_entry->fts_name, ".DS_Store" ) == 0 ) continue;
					
					file_count++;
				}
			}
			
			fts_close(fts);
			
			logString( [NSString stringWithFormat:@"Found %u children.\n", file_count]);
			[[NSApp delegate] performSelectorOnMainThread:@selector(setMaxProgress:) withObject:[NSNumber numberWithUnsignedInt:file_count] waitUntilDone:YES];
			[[NSApp delegate] performSelectorOnMainThread:@selector(progressStart:) withObject:nil waitUntilDone:YES];
			
			logString( @"Working...\n" );
			
            /* DeXinet Files */
			
            fts = fts_open(path, FTS_NOCHDIR|FTS_XDEV, NULL);
			
            while ((fts_entry = fts_read(fts)) != 0 )
            {
//                logwindowf( "%2.2d -> %s\n", fts_entry->fts_info, fts_entry->fts_path);
                
                if( fts_entry->fts_info == FTS_D )
                {
                    if( strcmp( fts_entry->fts_name, ".HSResource" ) == 0 )
                    {
                        fts_set(fts, fts_entry, FTS_SKIP);
                    }
                }
                else
                {
                    if( strcmp( fts_entry->fts_name, ".HSancillary" ) == 0 ) continue;
                    if( strcmp( fts_entry->fts_name, ".HSxmap" ) == 0 ) continue;
                    if( strcmp( fts_entry->fts_name, ".HSicon" ) == 0 ) continue;
                    if( strcmp( fts_entry->fts_name, ".HSResource" ) == 0 ) continue;
                    if( strcmp( fts_entry->fts_name, ".HSiconlow" ) == 0 ) continue;
                    if( strcmp( fts_entry->fts_name, ".DS_Store" ) == 0 ) continue;

                    if( (fts_entry->fts_name[0] == '.') && (fts_entry->fts_name[1] == '_') )
                    {
                        err = unlink(fts_entry->fts_path);
                        
                        if( (err == -1) && (errno != ENOENT) )
                        {
                            logwindowf("Could not delete file: %s (%s)\n", fts_entry->fts_path, strerror(errno));
                        }
                    }
                    else if( strcmp( fts_entry->fts_path, path[0]) == 0 )
                    {
                        /* skip */
                    }
                    else
                    {
                        DeXinet( fts_entry->fts_path );
						[[NSApp delegate] performSelectorOnMainThread:@selector(incrementProgress:) withObject:nil waitUntilDone:NO];
                    }
                }
                
                if( fts_entry->fts_info == FTS_DP )
                {
                    sprintf(delete_file, "%s/.HSancillary", fts_entry->fts_path);
                    err = unlink(delete_file);
                    
                    if( (err == -1) && (errno != ENOENT) )
                    {
                        logwindowf("Could not delete file: %s (%s)\n", delete_file, strerror(errno));
                    }
                    
                    sprintf(delete_file, "%s/.HSxmap", fts_entry->fts_path);
                    err = unlink(delete_file);
                    
                    if( (err == -1) && (errno != ENOENT) )
                    {
                        logwindowf("Could not delete file: %s (%s)\n", delete_file, strerror(errno));
                    }
                    
                    sprintf(delete_file, "%s/.HSicon", fts_entry->fts_path);
                    err = unlink(delete_file);
                    
                    if( (err == -1) && (errno != ENOENT) )
                    {
                        logwindowf("Could not delete file: %s (%s)\n", delete_file, strerror(errno));
                    }
                    
                    sprintf(delete_file, "%s/.HSResource", fts_entry->fts_path);
                    err = rmrf(delete_file);
                    
                    if( (err == -1) && (errno != ENOENT) )
                    {
                        logwindowf("Could not delete folder: %s (%s)\n", delete_file, strerror(errno));
                    }
                }
            }
            
            fts_close(fts);
            
            
		}
		else
		{
			logString( [NSString stringWithFormat:@"Error: cannot process file: %@\n", directoryPath] );
		}
        
        logString( @"\nDone.\n\n" );
		[[NSApp delegate] performSelectorOnMainThread:@selector(progressDone:) withObject:nil waitUntilDone:YES];
		
		[self completeOperation];		
		[pool release];
		
	}
	@catch(...) {
		// Do not rethrow exceptions.
	}
}

- (void)completeOperation {
    [self willChangeValueForKey:@"isFinished"];
    [self willChangeValueForKey:@"isExecuting"];
	
    executing = NO;
    finished = YES;
	
    [self didChangeValueForKey:@"isExecuting"];
    [self didChangeValueForKey:@"isFinished"];
}

- (BOOL)isConcurrent
{
	return YES;
}

- (BOOL)isExecuting {
    return executing;
}

- (BOOL)isFinished {
    return finished;
}

@end

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    return rv;
}

int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void logString( NSString *log )
{
	[[NSApp delegate] performSelectorOnMainThread:@selector(logString:) withObject:log waitUntilDone:NO];
}

void logwindowf( char *formatString, ... )
{
    char result[4096];
    
	va_list args;
    va_start(args, formatString);
    vsnprintf(result, 4096, formatString, args);
    va_end(args);
    
    result[4095] = '\0';
    
	logString( [NSString stringWithCString:result encoding:NSMacOSRomanStringEncoding] );
}

