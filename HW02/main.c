#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <iso646.h>
#include <string.h>
#include <stdint.h>

#define MAX_N 256 // 2^8 values without NUL as the end

// exit on failure						   
#define FAIL return EXIT_FAILURE

// mappings between 8-bit characters index of source encoding and unicode characters
struct single_correspondence_table
{
	uint32_t unicode[MAX_N + 1];
};

// possible input encodings
enum source_encoding_type { KOI8_R, ISO_8859_5, CP_1251 }; // order important

bool check_params(int params_count, char *params[])
{
	return (params_count == 4) and (params != NULL) and (params[1] != NULL) and (params[2] != NULL) and (params[3] != NULL);
}

bool translate_to_utf8_and_write(FILE *source, FILE *destination, enum source_encoding_type source_encoding)
{
	struct single_correspondence_table table[] = {
		{ // KOI8-R
					  {      9472, 9474, 9484, 9488, 9492, 9496, 9500, 9508, 9516, 9524, 9532, 9600, 9604, 9608, 9612, 9616,
				     		 9617, 9618, 9619, 8992, 9632, 8729, 8730, 8776, 8804, 8805, 160, 8993, 176, 178, 183, 247,
				     		 9552, 9553, 9554, 1105, 9555, 9556, 9557, 9558, 9559, 9560, 9561, 9562, 9563, 9564, 9565, 9566,
				     		 9567, 9568, 9569, 1025, 9570, 9571, 9572, 9573, 9574, 9575, 9576, 9577, 9578, 9579, 9580, 169,
				     		 1102, 1072, 1073, 1094, 1076, 1077, 1092, 1075, 1093, 1080, 1081, 1082, 1083, 1084, 1085, 1086,
				     		 1087, 1103, 1088, 1089, 1090, 1091, 1078, 1074, 1100, 1099, 1079, 1096, 1101, 1097, 1095, 1098,
				     		 1070, 1040, 1041, 1062, 1044, 1045, 1060, 1043, 1061, 1048, 1049, 1050, 1051, 1052, 1053, 1054,
				     		 1055, 1071, 1056, 1057, 1058, 1059, 1046, 1042, 1068, 1067, 1047, 1064, 1069, 1065, 1063, 1066}						 
		},
		{ // ISO-8859-5
			       {		 
			       		160, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 173, 1038, 1039,
			       		1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055,
			       		1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071,
			       		1072, 1073, 1074, 1075,	1076, 1077, 1078, 1079, 1080, 1081, 1082, 1083, 1084, 1085, 1086, 1087,		       		
			       		1088, 1089, 1090, 1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1099, 1100, 1101, 1102, 1103,
			       		8470, 1105, 1106, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115, 1116, 167, 1118, 1119
			        }
		},
		{ // Windows-1251
			   {   1026, 1027, 8218, 1107, 8222, 8230, 8224, 8225, 8364, 8240, 1033, 8249, 1034, 1036, 1035, 1039,
				   1106, 8216, 8217, 8220, 8221, 8226, 8211, 8212, 9888, 8482, 1113, 8250, 1114, 1116, 1115, 1119,
				   160, 1038, 1118, 1032, 164, 1168, 166, 167, 1025, 169, 1028, 171, 172, 173, 174, 1031, 176, 177,
				   1030, 1110, 1169, 181, 182, 183, 1105, 8470, 1108, 187, 1112, 1029, 1109, 1111, 1040, 1041, 1042,
				   1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058,
				   1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074,
				   1075, 1076, 1077, 1078, 1079, 1080, 1081, 1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089, 1090,
				   1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1099, 1100, 1101, 1102, 1103 }
		}
	}; 
	
	int current; // EOF compatibility
	
	while(!feof(source) and !ferror(source))
	{
		current = getc(source);
		
		if (current == EOF)
			break;
		
		if (current < 0x80) // ASCII
			fputc(current, destination);
		else
		{
			
			uint32_t ch = table[source_encoding].unicode[current - 128];
            
            if (ch < 0x800) {
            	fputc(0xC0 | ch >> 6,destination);
			} else if (ch < 0x8000) {
				fputc(0xE0 | ch >> 12,destination);
				fputc(0x80 | (ch >> 6 & 0x3F),destination);
			} else {
				fputc(0xF0 | ch >> 18, destination);
				fputc(0x80 | (ch >> 12 & 0x3F), destination);
				fputc(0x80 | (ch >> 6 & 0x3F), destination);
			}
			fputc(0x80 | (ch & 0x3F),destination);
		}
	}
	
	if (ferror(source))
		return false;
	else
		return true;	
}

int main(int params_count, char *params[])
{			
    if (!check_params(params_count, params))
    	FAIL;
    
    char *source_filename, *destination_filename;
    
    source_filename = params[1];        		    		
    FILE *source = fopen(source_filename, "r");

    if(source == NULL)       		
        FAIL;
        
    destination_filename = params[3];        		    		
    FILE *destination = fopen(destination_filename, "w+");

    if(destination == NULL) 
    {
    	fclose(source);      		
    	fflush(source);
        FAIL;
    }
   	
   	enum source_encoding_type encoding_type;
    char *encoding = params[2]; 
    
    if (strcmp(encoding, "koi8") == 0)
    	encoding_type = KOI8_R;
    else if (strcmp(encoding, "iso8859") == 0)
    	encoding_type = ISO_8859_5;
    else if (strcmp(encoding, "cp1251") == 0)
    	encoding_type = CP_1251;
   	else
   	{
    	fclose(source);      		
    	fflush(source);
    	fclose(destination);
    	fflush(destination);
    	FAIL;
    }

    bool result = translate_to_utf8_and_write(source, destination, encoding_type);
    fclose(source);      		
    fflush(source);
    fclose(destination);
    fflush(destination);
	
	if (!result)
		puts("An error occured while reading file.");
	
	return EXIT_SUCCESS;
}
