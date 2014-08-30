#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <iconv.h>
#include <ctype.h>

#include <sys/xattr.h>

/*

Compile/link: sudo gcc -wall -O3 -l iconv hfresource_clean.c -o /usr/local/bin/hf_clean

To migrate from a Xinet AFP volume to an Apple AFP volume.
*/

/* Input: folder/folder/file */

void logwindowf( char *formatString, ... );

const char *namedfork = "/..namedfork/rsrc";
const char *resource = ".HSResource/";
const char *xmap = ".HSxmap";
const char *ancillary = ".HSancillary";
const char *program = "DeXinet";

char *colon_convert( char *source );
char *slash2colon( char *source );
int is_cap_hex( char digit );
char *convert_filename_to_utf8( char *in );
char *convert_filename_to_macroman( char *in );

int DeXinet( const char *path )
{
	char *basepath = NULL;
    char *hf_resource_file = NULL;
 	char *destination_filepath = NULL;
    char *dest = NULL;
    unsigned char *buffer = NULL;
    char *ancillary_filepath = NULL;
    char *xmap_filename = NULL;
    char *destination_filename = NULL;
    char *destination_slash2colon_filename = NULL;
    char *destination_utf8_filename = NULL;
    char *macroman_destination_filename = NULL;
    
    FILE *source_file = NULL;
    FILE *dest_file = NULL;
    FILE *anceillary_file = NULL;
    FILE *xmap_file = NULL;
    
    struct stat stat_buf;
	char *type_string = "filename";
    int return_value = 0;
    
    stat(path, &stat_buf);

	//logwindowf( "%s\n", path );
	
	/* Copy resource from from .HFResource folder into named fork */
	
	/* extract filename from suppilied path */
	char *filename = strrchr( path, '/' );
	size_t base_length;
	
	/* extract basepath (without filename) from suppilied path */
	if( filename == NULL )
	{
		filename = (char *)path;
		base_length = 0;
		basepath = "";
	}
	else
	{
		filename++;
		base_length = filename - path - 1;
		basepath = malloc( base_length + 1 );
		
		if( basepath == NULL )
		{
			logwindowf( "Memmory allocation failed: basepath.\n" );
            return_value = -1;
            goto done;
		}

		strncpy( basepath, path, base_length );
		basepath[base_length] = 0;
	}
	
	size_t filename_length = strlen( filename );
	hf_resource_file = malloc( base_length + strlen(resource) + filename_length + 2 );

	if( hf_resource_file == NULL )
	{
		logwindowf( "Memmory allocation failed: source.\n" );
        return_value = -1;
        goto done;
	}

	strcpy( hf_resource_file, basepath);
	if( base_length > 0 ) strcat( hf_resource_file, "/" );
	strcat( hf_resource_file, resource );
	strcat( hf_resource_file, filename );

	destination_filename = colon_convert(filename);
    
	if( destination_filename == NULL )
	{
		destination_filename = filename;
	}
	else
	{

        destination_slash2colon_filename = slash2colon( destination_filename );
        destination_utf8_filename = convert_filename_to_utf8( destination_slash2colon_filename );

        destination_filepath = malloc( base_length + strlen(destination_utf8_filename) + 3 );
		
		if( destination_filepath == NULL )
		{
			logwindowf( "Memmory allocation failed: destination_filepath.\n" );
            return_value = -1;
            goto done;
		}

        strcpy( destination_filepath, basepath);
		if( base_length > 0 ) strcat( destination_filepath, "/" );
		strcat( destination_filepath, destination_utf8_filename );
	}

	if( S_ISREG(stat_buf.st_mode) )
	{
		source_file = fopen( hf_resource_file, "r" );
		
		if( source_file != NULL )
		{
			struct stat stat_buf_resource;
			stat(hf_resource_file, &stat_buf_resource);
			size_t final_length = strlen(path) + strlen(namedfork) + 1;
			dest = malloc( final_length );
			
			if( dest == NULL )
			{
				logwindowf( "Memmory allocation failed: dest.\n" );
                return_value = -1;
                goto done;
			}
			
			strcpy( dest, path );
			strcat( dest, namedfork );
		
			off_t file_size = stat_buf_resource.st_size;
			
			buffer = malloc( file_size );
		
			if( buffer == NULL )
			{
				logwindowf( "Memmory allocation failed: buffer.\n" );
                return_value = -1;
                goto done;
			}
			
			size_t read_size = fread(buffer, 1, file_size, source_file);
		
			if( read_size != file_size )
			{
				logwindowf( "%s did not read expected size for '%s' file: %ld != %lld\n", program, path, read_size, file_size );
                return_value = -1;
                goto done;
			}
			
			dest_file = fopen( dest, "w" );
			
			if( dest_file == NULL )
			{
				logwindowf( "%s could not open file %s for writing.\n", program, dest );
                return_value = -1;
                goto done;
			}
			
			size_t write_size = fwrite(buffer, 1, file_size, dest_file);
		
			if( write_size != file_size )
			{
				logwindowf( "%s did not write expected size for destination named fork: %ld != %lld\n", program, write_size, file_size );
                return_value = -1;
                goto done;
			}
		}
//		else
//		{
//			logwindowf( "No Resource: %s\n", path );
//		}
	}
	else
	{
		type_string = "foldername";
	}
	
	/* Copy finder info from ancillary file */

	ancillary_filepath = malloc( base_length + strlen(ancillary) + 2 );

	if( ancillary_filepath == NULL )
	{
		logwindowf( "Memmory allocation failed: ancillary_filepath.\n" );
        return_value = -1;
        goto done;
	}

	strcpy( ancillary_filepath, basepath);
	if( base_length > 0 ) strcat( ancillary_filepath, "/" );
	strcat( ancillary_filepath, ancillary );

	anceillary_file = fopen( ancillary_filepath, "r" );
	
	if( anceillary_file == NULL )
	{
		logwindowf( "%s could not open accessory file: %s (%s)\n", program, ancillary_filepath, strerror(errno) );
		//logwindowf( "input: %s\n", path );
        return_value = -1;
        goto done;
	}
	
	unsigned char fread_buffer[300];
	int file_in_xmap = 0;
	int position_in_xmap = 0;

	/* Try xmap file first */
	xmap_filename = malloc( base_length + strlen(xmap) + 2 );
	if( xmap_filename == NULL )
	{
		logwindowf( "Memmory allocation failed: xmap_filename.\n" );
        return_value = -1;
        goto done;
	}

	strcpy( xmap_filename, basepath);
	if( base_length > 0 ) strcat( xmap_filename, "/" );
	strcat( xmap_filename, xmap );

	xmap_file = fopen( xmap_filename, "r" );

	if( xmap_file != NULL )
	{
		unsigned char xmap_bufer[500];
		
		while( 1 )
		{
			size_t xmap_length = fread( xmap_bufer, 1, 64, xmap_file );
			
			if( xmap_length == 0 ) break;
			
			int additional_blocks;
			
			if( position_in_xmap != 0 && xmap_bufer[0] == 0 )
			{
				int skip = (xmap_bufer[2] << 8) + xmap_bufer[3];
				additional_blocks = skip - 1;
			}
			else
			{
				additional_blocks = xmap_bufer[0] / 64;
			}
			
			xmap_length = fread( &(xmap_bufer[64]), 1, additional_blocks * 64, xmap_file );
			
			if( xmap_length != additional_blocks * 64 )
			{
				logwindowf( "%s on file: %s, could not read in additional buffers from xmap file: %s, pos: %d\n", program, path, xmap_filename, position_in_xmap );
                return_value = -1;
                goto done;
			}
			
			if( xmap_bufer[0] != 0 )
			{
				/* make C-String */
				xmap_bufer[ xmap_bufer[0] + 1] = '\0';
				
//                logwindowf( "xmap: %s ?= %s", filename, (char *)&(xmap_bufer[1]));
                
				if( strcmp( filename, (char *)&(xmap_bufer[1]) ) == 0 )
				{
					file_in_xmap = 1;
//                    logwindowf( " %d, !\n", position_in_xmap );
					break;
				}
                else
                {
//                    logwindowf( "\n" );
                }
			}
			
			position_in_xmap = position_in_xmap + additional_blocks + 1;
		}
	}

	int found = 0;
	
	while( 1 )
	{
		size_t anceillary_file_read = fread(fread_buffer, 1, 300, anceillary_file);
		
		if( anceillary_file_read != 300 )
		{
			break;
		}
		
		int lookup_value = (fread_buffer[6] << 8) + fread_buffer[7];

//        if( file_in_xmap == 1 )
//        {
//            logwindowf( "ancillary position: %d ?= %d", lookup_value, position_in_xmap);
//        }
//        else
//        {
//            logwindowf( "ancillary string: %s ?= %s", destination_filename, (char *)&(fread_buffer[254]) );
//        }
        
        //macroman_destination_filename = convert_filename_to_macroman( destination_filename );
		if( (file_in_xmap == 1 && lookup_value == position_in_xmap) || strcmp( (char *)&(fread_buffer[254]), destination_filename ) == 0 )
		{
			/* found a match, copy finder info */
			
//            logwindowf( " !\n" );
			int result = setxattr( path, "com.apple.FinderInfo", &(fread_buffer[8]), 32, 0, 0 );
			
			if( result != 0 )
			{
				logwindowf( "%s error setting finder info for %s, (%s)\n", program, path, strerror( errno ) );
                return_value = -1;
                goto done;
			}
			
			found = 1;
			break;
		}
        else
        {
//            logwindowf( "\n" );
        }
	}

	if( found == 0 )
	{
		logwindowf( "%s found no finder info for %s: %s (%s)\n", program, type_string, path, destination_filename );
//		logwindowf( "real %s: %s\n", type_string, destination_filename );
	}

	if( destination_filepath != NULL )
	{
		int result = rename(path, destination_filepath);
		
		//logwindowf( "renaming: %s to %s\n", path, destination_filepath );
		
		if( result != 0 )
		{
			logwindowf( "%s could not rename %s to %s, errno: %d, error string: %s\n", program, path, destination_filepath, errno, strerror(errno) );
            return_value = -1;
            goto done;
		}
	}

done:
	
    if( basepath != NULL ) free( basepath );
    if( hf_resource_file != NULL ) free( hf_resource_file );
 	if( destination_filepath != NULL ) free( destination_filepath );
    if( dest != NULL ) free( dest );
    if( buffer != NULL ) free( buffer );
    if( ancillary_filepath != NULL ) free( ancillary_filepath );
    if( xmap_filename != NULL ) free( xmap_filename );
    if( (destination_filename != NULL) && (destination_filename != filename)) free( destination_filename );
    if( destination_slash2colon_filename != NULL ) free( destination_slash2colon_filename );
    if( destination_utf8_filename != NULL ) free( destination_utf8_filename );
    if( macroman_destination_filename != NULL) free( macroman_destination_filename );
    
    if( source_file != NULL ) fclose( source_file );
    if( dest_file != NULL ) fclose( dest_file );
    if( anceillary_file != NULL ) fclose( anceillary_file );
    if( xmap_file != NULL ) fclose( xmap_file );
 
//	int utimes_result; 
//	struct timeval times[2];
//	
//	times[0].tv_sec = stat_buf.st_atimespec.tv_sec;
//	times[0].tv_usec = 0;
//	
//	times[1].tv_sec = stat_buf.st_mtimespec.tv_sec;
//	times[1].tv_usec = 0;
//
//	utimes_result = utimes(path, times);
//	
//	if( utimes_result != 0 )
//	{
//		logwindowf( "%s could not update times for %s, errno: %d, error string: %s\n", program, path, errno, strerror(errno) );
//	}
	
	return return_value;
}

int is_cap_hex( char digit )
{
	int result = 0;
	
	if( isdigit(digit) )
	{
		result = 1;
	}
	else
	{
		if( digit >= 'A' && digit <= 'F' )
		{
			result = 1;
		}
	}
	
	return result;
}

char *convert_filename_to_utf8( char *in )
{
    char *ib = in;
    size_t inbytesleft = strlen(in);
    size_t outbytesleft = inbytesleft*4;
    char *result = malloc( outbytesleft );
    char *ob = result;
    
 	iconv_t ic = iconv_open ("UTF8", "MacRoman");
 	iconv( ic, &ib, &inbytesleft, &ob, &outbytesleft );
	iconv_close( ic );
    
    *ob = '\0';
    
    return result;
}

char *convert_filename_to_macroman( char *in )
{
    char *ib = in;
    size_t inbytesleft = strlen(in);
    size_t outbytesleft = inbytesleft*4;
    char *result = malloc( outbytesleft );
    char *ob = result;
    
 	iconv_t ic = iconv_open ("MacRoman", "UTF8" );
 	iconv( ic, &ib, &inbytesleft, &ob, &outbytesleft );
	iconv_close( ic );
    
    *ob = '\0';
    
    return result;
}

char *colon_convert( char *source )
{
	char *result = NULL;
    
    
	int colon_count = 0;
	char *temp = source;
	int found = 0;
	
	while( *temp != '\0' )
	{
		if( *(temp++) == ':' ) colon_count++;
	}
	
	if( colon_count > 0 )
	{
		result = malloc( strlen(source) + (colon_count*4) + 1 );
		char *dest = result;
		
		while( *source != '\0' )
		{
			if( *source == ':' )
			{
				if( source[1] != '\0' || source[2] != '\0' )
				{
					if( is_cap_hex(source[1]) && is_cap_hex(source[2]) )
					{
						char in_buffer;

						in_buffer = (digittoint(source[1]) << 4) + digittoint(source[2]);
						*(dest++) = in_buffer;

						found = 1;
						source += 3;
					}
					else
					{
						*(dest++) = *(source++);
					}
				}
				else
				{
					*(dest++) = *(source++);
				}
			}
			else
			{
				*(dest++) = *(source++);
			}
		}
		
        *dest = '\0';
        
		if( found == 0 )
		{
			free( result );
			result = NULL;
		}
	}

	return result;	
}

char *slash2colon( char *source )
{
	char *result = strdup( source );
	int i = 0;
	
	while( result[i] != 0 )
	{
		if( result[i] == '/' )
		{
			result[i] = ':';
		}
		
		i++;
	}

	return result;
}
