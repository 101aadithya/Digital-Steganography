/*
Name:	AADITHYA SATHEESH KUMAR
Project name:	LSB Steganography
Batch: 23029C
Date of submition:	28-2-2024	
 */

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

int main(int argc, char *argv[])
{
	EncodeInfo encInfo;
	DecodeInfo decInfo;
	int ret;
	if( argc > 2 )
	{
		ret = check_operation_type(argv);
	}
	else
	{
		printf("ERROR : Insufficent number of Command Line Arguments\n");
		return 0;
	}
	if( ret == e_encode )
	{
		printf("Selected Encoding\n");

		if( argc >= 4 ) // To check if correct number of CLA has been passed
		{
			if( read_and_validate_encode_args(argv, &encInfo) == e_success )
			{
				if( do_encoding(&encInfo) == e_success )
				{
					printf("## Encoding Done Sucessfully ##\n");
				}
				else
				{
					printf("ERROR: Encoding Unsucessful\n");
				}
			}
		}
		else
		{
			printf("ERROR : Insufficent number of Command Line Arguments\n");
		}
	}
	else if( ret == e_decode )
	{
		printf("Selected Decoding\n");

		if ( read_and_validate_decode_args(argv, &decInfo) == e_success )

		{
			if( do_decoding(&decInfo) == e_success )
			{
				printf("## Decoding Done Sucessfully ##\n");
			}
			else
			{
				printf("ERROR: Decoding Unsucessful\n");
			}
		}
		else
		{
			printf("ERROR: Unsupported format\n");
		}
	}
	return 0;
}

OperationType check_operation_type( char *argv[])
{
	if( strcmp(argv[1], "-e") == 0 )
	{
		return e_encode;
	}
	else if( strcmp(argv[1], "-d") == 0 )
	{
		return e_decode;
	}
	else
	{
		return e_unsupported;
	}
}

