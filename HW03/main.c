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
#define MIN_PRIME_NUMBER 2

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
} insert_status;

size_t hashtable_prime_size = MIN_PRIME_NUMBER; // starts from minimal prime size because of empty textfile existence probability (we do not got memory loss in this case)

element hashtable_source[MIN_PRIME_NUMBER] = { { EMPTY_ELEMENT, 0 }, { LAST_ELEMENT, 0 } }; // there are may be no words in a file at all (end element with NULL string always stored for barrier search)

void hashtable_content_print(element hashtable[])
{
	if (hashtable not_eq NULL)
	{
		size_t i = 0;
		element found;
		
		while (strcmp((found = hashtable[i++]).word, LAST_ELEMENT) not_eq 0) // barrier search			
			if (strcmp(found.word, EMPTY_ELEMENT) not_eq 0) // check for empty element		
				printf("%s count is %zu\n", found.word, found.count);			
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
	return (params_count == 2) and (params not_eq NULL) and (params[1] not_eq NULL);
}

size_t hash(char value[], size_t offset) // double hashing probe
{
	size_t result = 0;
	
	if (value not_eq NULL and strlen(value) > 0)
		for (size_t i = 0; value[i] not_eq '\0'; ++i)
			result += remainders[i] * value[i];
	else
		puts("NULL value pointer or empty value passed to hash function.");
		
	return (result % hashtable_prime_size + offset * (1 + result % (hashtable_prime_size - 1))) % hashtable_prime_size;
}

bool hashtable_element_search(element hashtable[], char key[], found_element * result)
{	
	if ((hashtable not_eq NULL) and (key not_eq NULL))
	{
		element found;
		size_t offset = 0, index;
		
		do		
			if (strcmp((found = hashtable[index = hash(key, offset++)]).word, key) == 0)	
			{	
				*result = (found_element) { index, found.count };
				
				return true;			
			}
		while(offset not_eq hashtable_prime_size);
	}
	else
		puts("NULL table pointer or NULL key pointer passed to search.");
	
	return false;
}

bool hashtable_size_increase(element hashtable[], size_t rounds) // rounds is a number of prime hashtable size rises
{	
	if (hashtable not_eq NULL and rounds > 0)
		{	
			size_t new_size, offset = 0, index = 0;
		
			while(rounds-- != 0)
				new_size = get_next_prime_number(hashtable_prime_size);
			
			element new_hashtable[new_size];
			hashtable_prime_size = new_size;
			strcpy(new_hashtable[--new_size].word, LAST_ELEMENT);
			new_hashtable[new_size].count = 0;
			
			// cleaning array
			while(--new_size not_eq 0)
				new_hashtable[new_size] = (element) { EMPTY_ELEMENT, 0 };
				
			new_hashtable[new_size] = (element) { EMPTY_ELEMENT, 0 };
			element found;	

			// empty element in new hashtable already exists
			while(strcmp((found = hashtable[new_size++]).word, LAST_ELEMENT) not_eq 0)
				if (strcmp(found.word, EMPTY_ELEMENT) == 0) // pass
					continue;
				else
				{
					while(strcmp(new_hashtable[index = hash(found.word, offset++)].word, EMPTY_ELEMENT) not_eq 0);
					new_hashtable[index] = found;	
				}
			if (memcpy(hashtable, new_hashtable, sizeof(new_hashtable)))
				return true; // success
		}
		else
			puts("NULL pointer to hashtable or incorrect rounds count passed to increase function.");
			
	return false;
}

insert_status hashtable_element_insert(element hashtable[], element new_element)
{
	insert_status result = { false, true, 0 };
	
	if (new_element.word not_eq NULL and strcmp(new_element.word, LAST_ELEMENT) not_eq 0 and strcmp(new_element.word, EMPTY_ELEMENT) not_eq 0) //validation
	{
		found_element search_output;
		result.is_error = false;
		
		if (hashtable_element_search(hashtable, new_element.word, &search_output))
		{	
			result.index = search_output.index;
				
			return result; // element already exists		
		}
		
		if (!hashtable_element_search(hashtable, EMPTY_ELEMENT, &search_output)) // key not found
		{
			hashtable_size_increase(hashtable, 1); // reconstruct hashtable to add empty elements
			hashtable_element_search(hashtable, EMPTY_ELEMENT, &search_output);		
		}
		
		hashtable[search_output.index] = new_element; // rewrite empty element
		result.result = true;		
		
		return result;		
	}
	else
		puts("Incorrect new element word.");
	
	return result;
}

bool hashtable_elements_fill(FILE * textfile, element hashtable[])
{	
	char word[MAX_WORD_LENGTH_PRIME];
	word[0] = '\0';
	bool is_word = false;
	char input;
	
	while(!feof(textfile) and !ferror(textfile))
	{
		input = getc(textfile);
	
		if (isalpha(input)) // append to a whole word
		{			
			is_word = true;			
    		strncat(word, &input, 1);    						
		}
		else if (is_word)
		{			
			element new_element = { "", 1 };
			strcpy(new_element.word, word);
			insert_status output;
			
			if((output = hashtable_element_insert(hashtable, new_element)).is_error)
				continue; // invalid word
			else if (!output.result)		
				++hashtable[output.index].count; // increasing found word counter	
			
			word[0] = '\0';
			is_word = false;		
		}
	}	
	
	if (ferror(textfile))
	{		
		perror("File reading error occured.");
		
		return false;
	}
	
	return true;
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
	element * hashtable = hashtable_source; 
	
    if (hashtable_elements_fill(textfile, hashtable_source)) // no need for memory check
 		hashtable_content_print(hashtable); 	
 		
   	fclose(textfile);
    fflush(textfile);    	
	
	return EXIT_SUCCESS;
}
