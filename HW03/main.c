#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <iso646.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>

// exit on failure						   
#define FAIL return EXIT_FAILURE
#define MAX_WORD_LENGTH_PRIME 251 // some high word length
#define LAST_ELEMENT "<END_OF_STRING>" // do not use in text
#define EMPTY_ELEMENT "<EMPTY_SYMBOL>" // do not use in text
#define MIN_PRIME_NUMBER 257

// remainders for each letter code in hash function computation
size_t remainders[MAX_WORD_LENGTH_PRIME];

// hash table entry
typedef struct
{
	char word[MAX_WORD_LENGTH_PRIME]; // 8-bit UTF-8
	size_t count;
} element; // to use declaration without struct keyword over all source

typedef struct
{
	size_t index;
	size_t count;
} found_element;

typedef struct
{
	bool result;
	bool is_error;
	size_t index;
	size_t new_size;
	element * new_table;
	bool increased;
} insert_status;

typedef struct
{
	size_t new_size;
	element * new_table;
} increase_out;

typedef struct
{
	size_t new_size;
	element * new_table;
} fill_result;

found_element found_data = { 0, 0 };

void hashtable_content_print(element * hashtable, size_t size)
{
	if (hashtable != NULL)
	{	
		element element_i;		
		
		for(size_t i = 0; i< size; ++i)
		{
			element_i = hashtable[i];
			
			if (strcmp(element_i.word, EMPTY_ELEMENT) != 0 and strcmp(element_i.word, LAST_ELEMENT) != 0)
				printf("%s count is %zu\n", element_i.word, element_i.count);
		}
	}	
	else
		puts("NULL table pointer passed to print.");
}


void recalculate_remainders()
{
	for (size_t i = 0; i < MAX_WORD_LENGTH_PRIME; ++i)
		remainders[i] = rand() % MAX_WORD_LENGTH_PRIME;
}

size_t get_next_prime_number(size_t current_prime_number) // required to get new hashtable size on reconstruction (with rounds == 1 we will got minimal memory loss)
{
	size_t saved = current_prime_number;
	bool is_prime = true;
	
	if (current_prime_number >= MIN_PRIME_NUMBER)
		for(current_prime_number = current_prime_number + 1; current_prime_number < SIZE_MAX; ++current_prime_number)	
			{
				for(size_t j = 2; j < current_prime_number; ++j)
					if (current_prime_number % j == 0)
					{
						is_prime = false;
						
						break;	
					}
					else
						is_prime = true;
	
				if (is_prime)
					break;		
			}
	
	return (current_prime_number == saved) ? 0 : current_prime_number;
}

// argc, argv checking
bool check_params(int params_count, char *params[])
{
	return (params_count == 2) and (params != NULL) and (params[1] != NULL);
}

size_t hash(char value[], size_t offset, size_t hashtable_prime_size) // double hashing probe
{
	size_t result = 0;
	
	if (value != NULL and strlen(value) > 0)
		for (size_t i = 0; value[i] != '\0'; ++i)
			result += remainders[i] * value[i];
	else
	{
		puts("NULL value pointer or empty value passed to hash function. Terminated.");
		
		exit(1);
	}
	result = (result % hashtable_prime_size + offset * (1 + result % (hashtable_prime_size - 1))) % hashtable_prime_size;
	
	return result;
}

bool hashtable_element_search(element * hashtable, char key[], size_t hashtable_prime_size)
{	
	if ((hashtable != NULL) and (key != NULL))
	{
		found_data = (found_element){ 0, 0 };		
		element found;
		size_t offset = 0, index = 0;
		
		do
		{					
			if (key != NULL and strlen(key) > 0 and strcmp((found = hashtable[index = hash(key, offset++, hashtable_prime_size)]).word, key) == 0)	
			{	
				found_data.index = index;
				found_data.count = found.count;			
				
				return true;			
			}			
		}
		while(offset <= hashtable_prime_size);		
	}
	else
	{	
		puts("NULL table pointer or NULL key pointer passed to search. Terminated.");
		
		exit(1);
	}
	
	return false;
}

element * place_new_elements(element * hashtable_old_in, element * hashtable_new_in, size_t new_prime_size)
{
	if (new_prime_size < SIZE_MAX / 2)
	{
		element found;
		size_t new_size = 0, offset = 0, index = 0;
		// empty element in new hashtable already exists
		while(strcmp((found = hashtable_old_in[new_size++]).word, LAST_ELEMENT) != 0)
		{					
			if (strcmp(found.word, EMPTY_ELEMENT) == 0 || found.word == NULL || strlen(found.word) <= 0) // pass
			{				
				continue;
			}
			else
			{				
				size_t stop_defense = 2 * new_prime_size;
				
				while(strcmp(hashtable_new_in[index = hash(found.word, offset++, new_prime_size)].word, EMPTY_ELEMENT) != 0 and offset < stop_defense);						
				
				hashtable_new_in[index].count = found.count;
				hashtable_new_in[index].word[0] = '\0';	
				strcpy(hashtable_new_in[index].word, found.word);			
			}
		}
	}
	else
	{		
		puts("Hashtable size limit exceeded. Terminated.");
		
		exit(1);
	}
		
	return hashtable_new_in;
}

increase_out hashtable_size_increase(element * hashtable_in, size_t rounds, size_t hashtable_prime_size) // rounds is a number of prime hashtable size rises
{	
	increase_out result = { 0, NULL };

	if (hashtable_in != NULL and rounds > 0)
	{	
		size_t new_size, saved_size = 0;
		
		while(rounds >= 1)
		{
			saved_size = new_size = get_next_prime_number(hashtable_prime_size);				
			--rounds;
		}
		
		element new_hashtable[new_size];
		memset(&new_hashtable, 0, new_size * sizeof(element)); 
		result.new_size = new_size;
		new_hashtable[--new_size].count = 0; 
		new_hashtable[new_size].word[0] = '\0';
		strcpy(new_hashtable[new_size].word, LAST_ELEMENT);
					
	
		while(--new_size > 0)
		{				
			new_hashtable[new_size].count = 0;
			new_hashtable[new_size].word[0] = '\0';
			strcpy(new_hashtable[new_size].word, EMPTY_ELEMENT);
		}
	
		new_hashtable[new_size].count = 0;
		new_hashtable[new_size].word[0] = '\0';
		strcpy(new_hashtable[new_size].word, EMPTY_ELEMENT);			
		element * filled = place_new_elements(hashtable_in, new_hashtable, saved_size);	
		hashtable_in = (element *)realloc(hashtable_in, saved_size * sizeof(element));
		
		if(hashtable_in)
		{
			if (memcpy(hashtable_in, filled, saved_size * sizeof(element)))			
				result.new_table = hashtable_in;		
		}
		else
		{				
			puts("Memory reallocation fails. Terminated.");
		
			free(hashtable_in);				
			exit(1);
		}
	}
	else
	{
		puts("NULL pointer to hashtable or incorrect rounds count passed to increase function. Terminated.");
		
		exit(1);
	}
				
	return result;
}

insert_status hashtable_element_insert(element * hashtable, element new_element, size_t hashtable_prime_size)
{
	insert_status result = { false, true, 0, hashtable_prime_size, hashtable, false };
	
	if (hashtable != NULL and new_element.word != NULL and strcmp(new_element.word, LAST_ELEMENT) != 0 and strcmp(new_element.word, EMPTY_ELEMENT) != 0) //validation
	{
		result.is_error = false;
		
		if (hashtable_element_search(hashtable, new_element.word, hashtable_prime_size))
		{	
			result.index = found_data.index;
			
			return result; // element already exists		
		}
		
		if (!hashtable_element_search(hashtable, EMPTY_ELEMENT, hashtable_prime_size)) // key not found
		{				
			increase_out output = hashtable_size_increase(hashtable, 1, hashtable_prime_size); // reconstruct hashtable to add empty elements		
			
			if (output.new_table)
			{
				result.new_size = output.new_size;
				result.new_table = output.new_table;
				result.increased = true;							
				hashtable_element_search(result.new_table, EMPTY_ELEMENT, result.new_size);						
			}
			else
			{
				puts("Resizement of hashtable failed. Terminated.");
				
				free(hashtable);
				exit(1);
			} 				
		}		
		
		result.new_table[found_data.index].count = new_element.count; // rewrite empty element
		result.new_table[found_data.index].word[0] = '\0';
		strcpy(result.new_table[found_data.index].word, new_element.word);
		result.result = true;	
		
		return result;		
	}
	else
	{
		puts("Incorrect new element word. Terminated.");
		
		exit(1);
	}
	
	return result;
}

fill_result hashtable_elements_fill(FILE * textfile, element * hashtable, size_t hashtable_prime_size)
{	
	fill_result result = { hashtable_prime_size, hashtable };
	char word[MAX_WORD_LENGTH_PRIME];
	word[0] = '\0';
	bool is_word = false;
	char input;
	
	while(!feof(textfile) and !ferror(textfile))
	{
		input = getc(textfile);		
		
		if (isalpha(input)) // append to a whole UTF-8 word (not isspace() because of segfault)
		{			
			is_word = true;			
    		strncat(word, &input, 1);     							
		}
		else if (is_word and word != NULL and strlen(word) > 0)
		{			
			element new_element = { "", 1 };
			new_element.word[0] = '\0';
			strcpy(new_element.word, word);    		
			insert_status output = hashtable_element_insert(result.new_table, new_element, result.new_size);
			result.new_table = output.new_table;
			result.new_size = output.new_size;			
			
			if(output.is_error)
			{				
				continue; // invalid word
			}
			else if (!output.result)				
				++result.new_table[output.index].count; // increasing found word counter			
			
			word[0] = '\0';
			is_word = false;			
		}
	}	
	
	if (ferror(textfile))
	{	
		perror("File reading error occured. Terminated.");	
		
		exit(1);
	}
		
	return result;
}

int main(int params_count, char *params[])
{			
	srand(time(NULL)); // setup randomizer

    if (!check_params(params_count, params))
    {
    	puts("Params check failed. Program terminated.");
    	
    	FAIL;
    }
    
    char *filename;
    
    filename = params[1];        		    		
    FILE *textfile = fopen(filename, "r");

    if(textfile == NULL)       	
    {	
        puts("Cannot read input file. Program terminated.");
        
        FAIL;
    }
	
	recalculate_remainders();	
	element * hashtable = malloc(MIN_PRIME_NUMBER * sizeof(element)); 
	
	if (hashtable)
	{
		size_t counter = MIN_PRIME_NUMBER,
		hashtable_prime_size = MIN_PRIME_NUMBER; // starts from minimal prime size because of empty textfile existence probability (we do not got memory loss in this case);
		hashtable[--counter].count = 0;
		hashtable[counter].word[0] = '\0';
		strcpy(hashtable[counter].word, LAST_ELEMENT);
		
		while(counter > 0)
		{
			hashtable[--counter].count = 0;
			hashtable[counter].word[0] = '\0';
			strcpy(hashtable[counter].word, EMPTY_ELEMENT);
		}
		// there are may be no words in a file at all (end element with NULL string always stored for barrier search)
		
		fill_result result = hashtable_elements_fill(textfile, hashtable, hashtable_prime_size); 
    	
    	if (result.new_table)
    	{
    		puts("===HASHTABLE CONTENT===");
			hashtable_content_print(result.new_table, result.new_size);
			puts("===END===");		
    		free(result.new_table);	
    	}    	
 	}
 	else 		
 		perror("Cannot assign memory location.");
 	
   	fclose(textfile);   	
	
	return EXIT_SUCCESS;
}
