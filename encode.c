#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h" 

/* Function Definitions for Encoding*/

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
	uint width, height;
	// Seek to 18th byte
	fseek(fptr_image, 18, SEEK_SET);

	// Read the width (an int)
	fread(&width, sizeof(int), 1, fptr_image);
	//	printf("width = %u\n", width);

	// Read the height (an int)
	fread(&height, sizeof(int), 1, fptr_image);
	//	printf("height = %u\n", height);

	// Return image capacity
	return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
	// Src Image file
	encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
	// Do Error handling
	if (encInfo->fptr_src_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

		return e_failure;
	}
	printf("INFO: Opened %s\n", encInfo->src_image_fname);

	// Secret file
	encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
	// Do Error handling
	if (encInfo->fptr_secret == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

		return e_failure;
	}
	printf("INFO: Opened %s\n", encInfo->secret_fname);

	// Stego Image file
	encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
	// Do Error handling
	if (encInfo->fptr_stego_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

		return e_failure;
	}
	printf("INFO: Opened %s\n", encInfo->stego_image_fname);

	// No failure return e_success
	return e_success;
}

Status read_and_validate_encode_args( char *argv[], EncodeInfo *encInfo )
{
	// To check if src file is .bmp
	if( strstr( argv[2], ".bmp") != NULL )
	{
		encInfo->src_image_fname = argv[2];		// Store filename to structre
	}
	else
	{
		printf("ERROR: The source file passed should be a .bmp file\n");
	}
	if( strchr( argv[3], '.') != NULL )
	{
		encInfo->secret_fname = argv[3];
		// After storing saperate the extn of secret file and store to structre
		strcpy(encInfo->extn_secret_file, strchr( argv[3], '.' ) );
	}
	else
	{
		printf("Error : The secret file does not have an extension\n");
		return e_failure;
	}
	// Check if output filename is passed or not
	if( argv[4] != NULL )
	{
		// Check if it is a .bmp file
		if( strstr ( argv[4] , ".bmp" ) != NULL )
		{
			encInfo->stego_image_fname = argv [4];
		}
		else
		{
			printf("Error : The output file is not a bmp file\n");
			return e_failure;
		}
	}
	// Else give error and use default name for storing o/p file name
	else
	{
		printf("INFO: Output File not mentioned. Creating stego.bmp as default\n");
		encInfo->stego_image_fname = "stego.bmp";			// Store default filename if not .bmp
	}

	return e_success;
}

// Encoding process begins
Status do_encoding( EncodeInfo *encInfo )
{
	printf("INFO: Opening required files\n");
	if ( open_files( encInfo ) == e_success )
	{
		printf("INFO: Done\n");
		printf("INFO: ## Encoding Procedure Started ##\n");
		printf("INFO: Checking for %s size\n", encInfo->secret_fname);
		if( get_file_size( encInfo->fptr_secret ) > 0 )
		{
			printf("INFO: Done. Not Empty\n");
			printf("INFO: Checking for %s capacity to handle %s\n", encInfo->src_image_fname, encInfo->secret_fname);
			if ( check_capacity( encInfo ) == e_success )
			{
				printf("INFO: Done. Found OK\n");
				printf("INFO: Copying Image Header\n");
				if( copy_bmp_header( encInfo->fptr_src_image, encInfo->fptr_stego_image ) == e_success )
				{
					printf("INFO: Done\n");
					printf("INFO: Encoding Magic String Signature\n");
					if (encode_magic_string(MAGIC_STRING, encInfo) == e_success )
					{
						printf("INFO: Done\n");
						printf("INFO: Encoding %s Extension Size\n", encInfo->secret_fname);
						if( encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_success )
						{
							printf("INFO: Done\n");
							printf("INFO: Encoding %s Extension\n", encInfo->secret_fname);
							if( encode_secret_file_extn( encInfo->extn_secret_file, encInfo) == e_success)
							{
								printf("INFO: Done\n");
								printf("INFO: Encoding %s File Size\n", encInfo->secret_fname);
								if( encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
								{
									printf("INFO: Done\n");
									printf("INFO: Encoding %s File Data\n", encInfo->secret_fname);
									if( encode_secret_file_data( encInfo) == e_success )
									{
										printf("INFO: Done\n");
										printf("INFO: Copying Left Over Data\n");
										if( copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure )
										{
											printf("ERROR: Unable to copy remaining image data\n");
											return e_failure;
										}
										else
										{
											printf("INFO: Done\n");
											return e_success;
										}
									}
									else
									{
										printf("ERROR: Unable to encode secret file data\n");
										return e_failure;
									}
								}	
								else
								{
									printf("ERROR: Unable to encode secret file size\n");
									return e_failure;
								}
							}
							else
							{
								printf("ERROR: Unable to encode secret file extenstion\n");
								return e_failure;
							}
						}
						else
						{
							printf("ERROR: Unable to encode secret file extenstion size\n");
							return e_failure;
						}
					}	
					else
					{
						printf("ERROR: Unable to encode magic string\n");
						return e_failure;
					}
				}
				else
				{
					printf("ERROR: Unable to copy header data\n");
					return e_failure;
				}
			}
			else
			{
				printf("ERROR: File %s does not have the capacity to store data\n",encInfo->src_image_fname);
				return e_failure;
			}
		}
		else
		{
			printf("ERROR: Unable to fetch file size of %s\n",encInfo->src_image_fname);
			return e_failure;
		}
	}
	else
	{
		printf("ERROR: Unable to open files\n");
		return e_failure;
	}
	return e_success;
}

// Function to check if the source image file has enough space to store data to be encoded
Status check_capacity( EncodeInfo *encInfo )
{
	encInfo->image_capacity= get_image_size_for_bmp( encInfo->fptr_src_image );
	encInfo->size_secret_file = get_file_size( encInfo->fptr_secret );
	if( (encInfo->image_capacity) > (( strlen(MAGIC_STRING) + strlen(encInfo->extn_secret_file) + 4 + 4 + encInfo->size_secret_file )*8) )
	{
		return e_success;
	}
	else
	{
		return e_failure;
	}
} 

// Function to get the number of characters in the secret file
uint get_file_size( FILE *fptr )
{
	fseek(fptr, 0, SEEK_END); // Moves to the end of the file
	return ftell(fptr);       // Returns the number of index to which file pointer points
}

// Function reads 54 byte header data from source file and write to stego file directly
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image )
{
	rewind( fptr_src_image); 		// Source image file pointer should be reset after get image file size fn is called
	char buffer[54]; 				// Buffer to store the header data from source file
	if( fread( buffer, 1, 54, fptr_src_image) != 54 )
	{
		return e_failure;
	}
	if ( fwrite( buffer, 1, 54, fptr_dest_image) != 54 ) 
	{
		return e_failure;
	}

	return e_success;
}
// To encode magic string to op image
Status encode_magic_string( const char *magic_string, EncodeInfo *encInfo )
{
	// Here instead of using loops, encode_data_to_image fn is called to write the data to the image
	if( encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure )
	{
		return e_failure;
	}
	return e_success;
}

Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
	// Store 8 bytes of RGB data from soruce file to buffer
	char buffer[8];
	for( int i = 0 ; i < size ; i++ )
	{
		// To get 8 bytes of data from source file and store it to the buffer to store 1 byte of magic string data
		if(  fread( buffer, 1, 8, fptr_src_image)  != 8 )	
		{
			return e_failure;
		}
		if( encode_byte_to_lsb( data[i], buffer) == e_failure )
		{
			return e_failure;
		}
		if( fwrite( buffer, 1, 8, fptr_stego_image) != 8 ) //To write the data obtained to the output file
		{
			return e_failure;
		}
	}
	return e_success;
}

// Logic to encode the data to of magic string to the image buffer
Status encode_byte_to_lsb( char data, char *image_buffer )
{
	// Loop is used to store each bit of the 1 byte data to the LSB of each character of buffer
	int i=0, k=0;
	for( i = 7 ; i >= 0 ; i-- )
	{
		unsigned char temp = (data & ( 1 << i ) );
		temp = temp >> i;
		image_buffer[k] = ( image_buffer[k] & ( ~ ( 0x01 ) ) ) | temp;
		k++;
	}
	return e_success;
}

// Function to encode the size data ( 4 byte data ) to the buffer of size 32 byte
Status encode_size_to_lsb( long size, char *buffer )
{
	// Loop is used to store each bit of size to LSB of each element of buffer
	int i = 0 , k = 0 ;
	for( i = 31 ; i >= 0 ; i-- )
	{
		unsigned char temp = (size & ( 1 << i ) );
		temp = temp >> i;
		buffer[k] = ( buffer[k] & ( ~ ( 0x01 ) ) ) | temp;
		k++;
	}
	return e_success;
}

// Function to encode the extn size of secret file to the output image
Status encode_secret_file_extn_size( long secret_file_extn_size, EncodeInfo *encInfo )
{
	char buffer[32];
	if( fread( buffer, 1, 32, encInfo->fptr_src_image) != 32 )
	{
		return e_failure;
	}
	if( encode_size_to_lsb( secret_file_extn_size, buffer) == e_failure )
	{
		return e_failure;
	}
	if( fwrite( buffer, 1, 32, encInfo->fptr_stego_image) != 32 )
	{
		return e_failure;
	}
	return e_success;
}

// The function encodes the extension of the secret file to the output image
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo )
{
	// encInfo->image_data is the buffer declared from structre
	for( int i = 0 ; i < strlen( file_extn ) ; i++ )
	{
		if( fread( encInfo->image_data, 1, 8, encInfo->fptr_src_image) != 8 )
		{
			return e_failure;
		}
		if( encode_byte_to_lsb( file_extn[i], encInfo->image_data) == e_failure )
		{
			return e_failure;
		}
		if( fwrite( encInfo->image_data, 1, 8, encInfo->fptr_stego_image) != 8 ) // To write the data obtained to the output file
		{
			return e_failure;
		}
	}
	return e_success;
}
// To encode secret file size to op file
Status encode_secret_file_size( long file_size, EncodeInfo *encInfo)
{

	char buffer[32];
	if( fread( buffer, 1, 32, encInfo->fptr_src_image ) != 32 )
	{
		return e_failure;
	}
	if( encode_size_to_lsb( file_size, buffer ) == e_failure )
	{
		return e_failure;
	}
	if( fwrite( buffer, 1, 32, encInfo->fptr_stego_image ) != 32 )
	{
		return e_failure;
	}

	return e_success;
}

Status encode_secret_file_data( EncodeInfo *encInfo )
{
	char secret_data[encInfo->size_secret_file]; // Declares buffer to store secret file data for encoding
	rewind( encInfo->fptr_secret);
	fread( secret_data, 1, encInfo->size_secret_file, encInfo->fptr_secret );

	// Loop to encode each byte to the RGB data
	for( int i = 0 ; i < encInfo->size_secret_file ; i++ )
	{
		if( fread( &encInfo->image_data, 1, 8, encInfo->fptr_src_image) != 8 )	// To get RGB data from BMP file
		{
			return e_failure;
		}
		if( encode_byte_to_lsb( secret_data[i], encInfo->image_data) == e_failure ) // To store each character to the bmp file buffer
		{
			return e_failure;
		}
		if( fwrite( &encInfo->image_data, 1, 8, encInfo->fptr_stego_image) != 8 )	// To write the encoded buffer to the output file
		{
			return e_failure;
		}
	}

	return e_success;
}
// To copy the remaining data
Status copy_remaining_img_data( FILE *fptr_src, FILE *fptr_dest)
{
	// Use char buffer to read rest of data from source file and write to the output file using a loop
	char ch; 								// Read each byte from the file and store to the variable
	while( fread(&ch, 1, 1, fptr_src) != 0 ) // Use while loop until return fread returns 0 ( which occurs when EOF is encoutered )
	{
		if( fwrite(&ch, 1, 1, fptr_dest) != 1 )
		{
			// If character is not sucessfully read to ch, the process fails
			return e_failure;
		}
	}
	return e_success;
}



