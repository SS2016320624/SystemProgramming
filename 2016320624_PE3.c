#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

int main(int argc, char* argv[])
{

	if( argc != 3 )
	{
		perror( "Usage : mv [file1] [file2] or mv [file1] [directory]." );	//매개변수 관련 오류 처리
		return -1;
	}

	if( strcmp( argv[1], argv[2] ) == 0 )
	{
		char buffer[BUFSIZ];
		sprintf( buffer, "\'%s\' and \'%s\' area the same file.", argv[1], argv[2] ); //매개변수 관련 오류 처리
		perror( buffer );
		return -1;
	}


	int error = 0;	
	DIR* dir;
	if( dir = opendir( argv[2] ) ) {			//디렉토리일경우 이동
		char newLocation[BUFSIZ];
		sprintf( newLocation, "%s/%s", argv[2], argv[1] );
		error = rename( argv[1], newLocation );	
		printf("move %s to %s.\n", argv[1], argv[2]);
	}
	else {										//디렉토리가아닐경우 이름변경
		error = rename( argv[1], argv[2] );	
		printf("remane %s to %s.\n", argv[1], argv[2]);
	}


	if( error )		//에러출력
	{
		perror( "Unable to relocate or rename file." );
		return -1;
	}

	return 0;
}



