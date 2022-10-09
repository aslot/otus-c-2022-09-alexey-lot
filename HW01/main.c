#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <iso646.h>

#define IO_ERR_MSG "I/O error when reading"

// print string with return code check
#define PS(format_string, string) do {\
										int code = printf(format_string, string);\
										\
										if (code > 0) {}\
										else {\
												perror("String printing failed");\
												\
												return EXIT_FAILURE;\
										 	}\
								   	}\
								   	while(0)

// exit on failure						   
#define FAIL return EXIT_FAILURE

#define FILE_CHECK(eof_message) do { if (ferror(fp)) {\
        									puts(IO_ERR_MSG);\
        									\
        									FAIL;\
    									} else if (feof(fp)) {\
        									puts(eof_message);\
        									\
        									FAIL;\
    									}\
    								}\
    								while(0)

// run over jpeg to find format signature
#define JPEG_CYCLE(fp, FIRST_BYTE, SECOND_BYTE, eof_message) do {\
    do\
    {\
    	if (fgetc(fp) == FIRST_BYTE)\
    		if (fgetc(fp) == SECOND_BYTE)\
    			break;\
    }\
    while(!feof(fp) and !ferror(fp));\
    \
    FILE_CHECK(eof_message);\
}\
while(0)
			
// find start jpeg signature
#define CHECK_FOR_JPEG(fp) do {\
	const unsigned char FIRST_BYTE_JPEG_STARTS = 0xFF,\
    					SECOND_BYTE_JPEG_STARTS = 0xD8;\
    JPEG_CYCLE(fp, FIRST_BYTE_JPEG_STARTS, SECOND_BYTE_JPEG_STARTS, "No JPEG signature found.");\
}\
while(0)

// find end jpeg signature
#define FIND_JPEG_END(fp) do {\
	const unsigned char PENULTIMATE_BYTE_JPEG_ENDS = 0xFF,\
						LAST_BYTE_JPEG_ENDS = 0xD9;\
	JPEG_CYCLE(fp, PENULTIMATE_BYTE_JPEG_ENDS, LAST_BYTE_JPEG_ENDS, "End of file reached successfully. No JPEG signature found.");\
}\
while(0)
								 
int main(int params_count, char *params[])
{
	char *zipjpeg_filename;
	
    if ((params_count == 2) and (params != NULL) and (params[1] != NULL))
    	{
    		zipjpeg_filename = params[1];
    		PS("Passed file name: %s\n", zipjpeg_filename);    		
    		FILE *fp = fopen(zipjpeg_filename, "r");
   		
    		if(fp == NULL) {
        		PS("File\'s %s opening failed.\n", zipjpeg_filename);     		
        		
        		FAIL;
    		}
    		
    		CHECK_FOR_JPEG(fp);
    		FIND_JPEG_END(fp);
    		PS("JPEG file's %s end was found.\n", zipjpeg_filename);
    		
    		// ZIP PKWARE signature
    		const unsigned char FIRST_ZIP = 0x50,
    							SECOND_ZIP = 0x4B,
    							THIRD_ZIP = 0x03,
    							FOURTH_ZIP = 0x04;      	
    		   		
    		do
    		{   
    			if (fgetc(fp) == FIRST_ZIP)
    				if (fgetc(fp) == SECOND_ZIP)
    					if (fgetc(fp) == THIRD_ZIP)
    						if (fgetc(fp) == FOURTH_ZIP)  		  			
    						{    			    													
    							PS("JPEG file %s contains in-zip file.\n", zipjpeg_filename);    				
    							char temp[22]; // garbage fields size between signature and file name length
    							fread(temp, sizeof(char), 22, fp);    							
    							FILE_CHECK("EOF when reading through next in-zip file.");  						
    							
    							// read in-zip file name length
    							unsigned short int name_length;
    							fread(&name_length, sizeof(unsigned short int), 1, fp);    							 						
    							FILE_CHECK("EOF when reading next in-zip file name length."); 
    							
    							unsigned short int extra_length; // garbage field extra field length
    							fread(&extra_length, sizeof(unsigned short int), 1, fp);    							
    							FILE_CHECK("EOF when reading next in-zip file extra field length."); 
    							
    							// read file name
    							char filename[name_length];
    							fread(filename, sizeof(char), name_length, fp);    													
    							FILE_CHECK("EOF when reading in-zip file name.");   
    							filename[name_length] = '\0';
    							PS("In-zip file name is %s\n", filename);   																				
    						}    						       			
    		}
    		while(!feof(fp) and !ferror(fp));   
    		
    		FILE_CHECK("No more in-zip files found or there is no zip file at all."); 	    		
    		
    		if (fclose(fp) == EOF)
    		{
    			puts("File closing error.");
    			
    			FAIL;
    		}
    	}	
    else
    	FAIL;
    
    return EXIT_SUCCESS;
}
