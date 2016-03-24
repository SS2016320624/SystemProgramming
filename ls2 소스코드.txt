#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define READCHAR        10      
#define EXTENDCHAR      5       
#define NAME_MAX        255
//#define PATH_MAX      4096
#define PATHLEN         20      

#define PER0    0       //type
#define PER1    1       //owner 1
#define PER2    2       //owner 2
#define PER3    3       //owner 3
#define PER4    4       //group 1
#define PER5    5       //group 2
#define PER6    6       //group 3
#define PER7    7       //other 1
#define PER8    8       //other 2
#define PER9    9       //other 3
#define PER10   10      //NULL

typedef struct Info_st_{
        char    permission[11];
        int     linkcount;
        char    userid[20];
        char    groupid[20];
        int     size;
        int     date[2];
        int     time[2];
        char    filename[255];
        char    link[4096];
} Info_st;

typedef struct ListElmt_ {

	void               *data;
	struct ListElmt_   *next;

} ListElmt;

typedef struct List_ {

	int                size;

	int                (*match)(const void *key1, const void *key2);
	void               (*destroy)(void *data);

	ListElmt           *head;
	ListElmt           *tail;

} List;


int             SizeSum(char filepath[]); 
int             DirSeek(char dirname[]);        
void            CheckFile(struct stat *buf, Info_st * info);    
void            PrintfInfo(List * list);        
void            MaxSpace(List * list, int max[]);       
void            DeleteInfo(void * data);        
void            FilenameSort(List * list);      
char    *       ReadString();           
char    *       ReadPathAndFilenameCpy(char * dirname,char * filename );
int             ExtenMem(char ** str, int maxStringLen);        

void list_init(List *list, void (*destroy)(void *data))
{
	list->size = 0;
	list->destroy = destroy;
	list->head = NULL;
	list->tail = NULL;

	return;
}



void list_destroy(List *list) 
{
	void               *data;

	while(list->size > 0) 
	{
		if (list_rem_next(list, NULL, (void **)&data) == 0 && list->destroy != NULL)
		{			
			list->destroy(data);
		}
	}

	memset(list, 0, sizeof(List));

	return;
}

int list_ins_next(List *list, ListElmt *element, const void *data) 
{
	ListElmt           *new_element;

	if ((new_element = (ListElmt *)malloc(sizeof(ListElmt))) == NULL)
		return -1;	

	new_element->data = (void *)data;	
	if (element == NULL) 	
	{
		if (list->size == 0)
			list->tail = new_element;

		new_element->next = list->head;
		list->head = new_element;
	}

	else 		
	{
		if (element->next == NULL)
			list->tail = new_element;

		new_element->next = element->next;
		element->next = new_element;
	}
	list->size++; 

	return 0;
}

int list_rem_next(List *list, ListElmt *element, void **data) 
{
	ListElmt           *old_element;

	if (list->size == 0)	
		return -1;

	if (element == NULL)	
	{
		*data = list->head->data;
		old_element = list->head;
		list->head = list->head->next;

		if (list->size == 0)
			list->tail = NULL;
	}
	else 	
	{
		if (element->next == NULL)
			return -1;

		*data = element->next->data;
		old_element = element->next;
		element->next = element->next->next;

		if (element->next == NULL)
			list->tail = element;
	}

	free(old_element);

	list->size--; 

	return 0;
}

int main(void)
{	
	char * dirname;

	fputs("오픈할 directory를 입력하세요 : ",stdout);
	dirname=ReadString();	

	if(DirSeek(dirname)==-1)
	{
		puts("실패");
		return -1;
	}	

	free(dirname);
	return 0;
}



void CheckFile(struct stat *buf, Info_st * info)
{

	if(S_ISDIR(buf->st_mode)) 
		info->permission[PER0]='d';
	else if(S_ISLNK(buf->st_mode))
		info->permission[PER0]='l';
	else if(S_ISCHR(buf->st_mode))
		info->permission[PER0]='c';
	else if(S_ISBLK(buf->st_mode))
		info->permission[PER0]='b';
	else if(S_ISSOCK(buf->st_mode))
		info->permission[PER0]='s';
	else if(S_ISFIFO(buf->st_mode))
		info->permission[PER0]='P';
	else
		info->permission[PER0]='-';
	/*사용자 권한 검사*/
	if(buf->st_mode & S_IRUSR)
		info->permission[PER1]='r';
	else
		info->permission[PER1]='-';
	if(buf->st_mode & S_IWUSR)
		info->permission[PER2]='w';
	else
		info->permission[PER2]='-';
	if(buf->st_mode & S_IXUSR)
		info->permission[PER3]='x';
	else if(buf->st_mode & S_ISUID)
		info->permission[PER3]='s';
	else
		info->permission[PER3]='-';

	/*그룹 권한 검사*/
	if(buf->st_mode & S_IRGRP)
		info->permission[PER4]='r';
	else
		info->permission[PER4]='-';
	if(buf->st_mode & S_IWGRP)
		info->permission[PER5]='w';
	else
		info->permission[PER5]='-';
	if(buf->st_mode & S_IXGRP)
		info->permission[PER6]='x';
	else if(buf->st_mode & S_ISGID)
		info->permission[PER6]='s';
	else
		info->permission[PER6]='-'; 
	/*일반사용자 권한 검사*/
	if(buf->st_mode & S_IROTH)
		info->permission[PER7]='r'; 
	else 
		info->permission[PER7]='-';
	if(buf->st_mode & S_IWOTH)
		info->permission[PER8]='w';
	else
		info->permission[PER8]='-';

	if(buf->st_mode & S_IXOTH) //stiky bit 설정
	{
		if(buf->st_mode & S_ISVTX)
			info->permission[PER9]='t';
		else
			info->permission[PER9]='x';
	}
	else
	{
		if(buf->st_mode & S_ISVTX)
			info->permission[PER9]='T';
		else
			info->permission[PER9]='-';
	}	

	info->permission[PER10]='\0';
}


/*	Path를 입력한 그 디렉토리의 크기를 구하는 함수.		*/
int SizeSum(char filepath[]) //합계 구하는 함수 
{
         struct dirent * entry;
         DIR * dirpt;
         int sum=0;
         struct stat buf;
         char * filename;

         dirpt=opendir(filepath);

         while((entry=readdir(dirpt))!='\0')
         {       
		filename=ReadPathAndFilenameCpy(filepath,entry->d_name);
		 if((lstat(filename,&buf))==0)  //lstat 성공시만 출력 for check
			 sum=sum+buf.st_blocks;                  
	 }
	 free(filename);
	 closedir(dirpt);

	 return sum/2;
}



void MaxSpace(List * list, int max[])
{
	int i,j;
        ListElmt * ptr;
	char maxStr[2][10];

	for(i=0,ptr=list->head; i<list->size ; i++,ptr=ptr->next)
	{
		if(((Info_st *)ptr->data)->linkcount > max[0])
			max[0]=((Info_st *)ptr->data)->linkcount;

		if(max[1]<(strlen(((Info_st *)ptr->data)->userid)))
			max[1]=strlen(((Info_st *)ptr->data)->userid);

		if(max[2]<(strlen(((Info_st *)ptr->data)->groupid)))
			max[2]=strlen(((Info_st *)ptr->data)->groupid);

		if(max[3]<((Info_st *)ptr->data)->size)
			max[3]=((Info_st *)ptr->data)->size;
	}
	sprintf(maxStr[0],"%d",max[0]);
	sprintf(maxStr[1],"%d",max[3]);

	max[0]=strlen(maxStr[0])+1;
	max[3]=strlen(maxStr[1])+1;
}


void PrintfInfo(List * list)
{
        int i;
	int max[4]={0}; //[0]하드링크수,[1]유저아이디,[2]그룹아이디,[3]크기 
        ListElmt * ptr;

        if(list->size==0)
        {
                puts("저장된 자료가 없습니다.");
		exit(1);
        }

	MaxSpace(list,max);
 
        for(i=0,ptr=list->head ; i<list->size ; i++,ptr=ptr->next)
        {
		printf("%s",((Info_st *)ptr->data)->permission);
		printf("%*d",max[0],((Info_st *)ptr->data)->linkcount);
		printf(" %-*s",max[1],((Info_st *)ptr->data)->userid);
		printf(" %-*s",max[2],((Info_st *)ptr->data)->groupid);
		printf("%*d",max[3],((Info_st *)ptr->data)->size);
		printf(" %2d월 %2d",((Info_st *)ptr->data)->date[0]
				 ,((Info_st *)ptr->data)->date[1]);
		printf(" %02d:%02d",((Info_st *)ptr->data)->time[0]
				,((Info_st *)ptr->data)->time[1]);
		printf(" %s",((Info_st *)ptr->data)->filename);
					
		if(((Info_st *)ptr->data)->permission[0]=='l')
			printf(" -> %s\n",((Info_st *)ptr->data)->link);
		else
			puts("");
        }       
}
       


void DeleteInfo(void * data)
{
	free((Info_st *)data);
}	


/*	링크리스트에 filename을 이용해 정렬하는 함수	*/
void FilenameSort(List * list)
{
	int i, j, strLen1, strLen2, minNum, cmpN;
	int index;
	ListElmt * ptr;
	void * temp;

	for(i=list->size-1 ; i>0 ; i--)
	{
		for(j=0,ptr=list->head ; j<i ; j++,ptr=ptr->next)
		{
			strLen1=strlen(((Info_st *)ptr->data)->filename);
			strLen2=strlen(((Info_st *)ptr->next->data)->filename);

			if(strLen1<strLen2)
				minNum=strLen2;
			else
				minNum=strLen1;	

			for(cmpN=minNum,index=0; cmpN!=0 ;cmpN--,index++)
			{
				if(((Info_st *)ptr->data)->filename[index] >
					 ((Info_st *)ptr->next->data)->filename[index])
				{
					temp=ptr->data;
					ptr->data=ptr->next->data;
					ptr->next->data=temp;
					break;
				}
				else if(((Info_st *)ptr->data)->filename[index] ==
					((Info_st *)ptr->next->data)->filename[index]) 
				{
					continue;
				}
				break;
			}
		}
	}
}


/*	Path를 읽고 동적메모리 할당을 하고 복사하는 함수	*/
char * ReadPathAndFilenameCpy(char * dirname,char * filename )
{
	int maxPathLen=PATHLEN;
	char * path=(char *)malloc(sizeof(char)*maxPathLen);
	int idx=0, idx2=0;
	
	while(1)
	{
		if(idx>=maxPathLen)
			maxPathLen=ExtenMem(&path,maxPathLen);
		if(dirname[idx]=='\0')
			break;
		path[idx]=dirname[idx];	
		idx++;
	}
	
	if(idx>=maxPathLen)
		maxPathLen=ExtenMem(&path,maxPathLen);

	path[idx++]='/';

	while(1)
	{
		if(idx>=maxPathLen)
			maxPathLen=ExtenMem(&path,maxPathLen);
		if(filename[idx2]=='\0')
		{
			path[idx]='\0';
			break;
		}
		path[idx]=filename[idx2];
		idx++, idx2++;
	}
	return path;
}


/*      입력한 디렉토리의 자료를 Info_st구조체에 저장하고 
        링크드 리스트를 생성 후 주소를 저장해 자료구조에 저장하는 함수  */
int DirSeek(char dirname[])
{
	List list;
	ListElmt * elmt={'\0'};
	Info_st * info;
	DIR * dirpt;
	struct dirent * entry;
	struct stat buf;
	struct group * grp;
	struct passwd * pwd;
	struct tm * time;
	char fileName[NAME_MAX];
	int readc=0, total=0;
	char * dirPath;

	list_init(&list,DeleteInfo);

	if((dirpt=opendir(dirname))=='\0')	
	{
		puts("Directory open fail");
		return -1;
	}
	
	printf("%s:\n",dirname);
		
	total=SizeSum(dirname);
	printf("합계 %d\n",total);

	while((entry=readdir(dirpt))!=NULL)
	{
		strcpy(fileName, dirname);
		strcat(fileName, "/");
		strcat(fileName,entry->d_name);
		if((lstat(fileName,&buf)==0))
		{
			info=(Info_st *)malloc(sizeof(Info_st));
			pwd=getpwuid(buf.st_uid);
			grp=getgrgid(buf.st_gid);
			strcpy(info->userid, pwd->pw_name);
			strcpy(info->groupid, grp->gr_name);	

			info->linkcount=buf.st_nlink;
			info->size=buf.st_size;

			time=localtime(&buf.st_mtime);

			info->date[0]=(time->tm_mon)+1;
			info->date[1]=time->tm_mday;	
			info->time[0]=time->tm_hour;
			info->time[1]=time->tm_min;

			CheckFile(&buf,info);
			strcpy(info->filename, entry->d_name);

			if(S_ISLNK(buf.st_mode))
			{
				readc=readlink(fileName,info->link,sizeof(info->link));
				info->link[readc]='\0';
			}
			
			if((list_ins_next(&list,elmt,info))==-1)
			{	
				puts("list삽입실패");
				return -1;
			}
		}
	} 

	FilenameSort(&list);
	PrintfInfo(&list);
	
	elmt=list.head;

	while(elmt->next!=0)
	{


		if(((Info_st *)elmt->data)->permission[0]=='d' 
			&& (strcmp(((Info_st *)elmt->data)->filename,".."))!=0 
			&& (strcmp(((Info_st *)elmt->data)->filename,"."))!=0)

			
		{
			dirPath=ReadPathAndFilenameCpy(dirname,((Info_st *)elmt->data)->filename);
			puts("");
			DirSeek(dirPath);
			free(dirPath);
		}		
		elmt=elmt->next;
	}
	list_destroy(&list);
	closedir(dirpt);

	return 0;
}


/*	입력받은 문자가 큰 경우 동적메모리 크기를 확장하는 함수	*/
int ExtenMem(char ** str, int maxStringLen)
{
	char * newStr;
	int i;
	newStr=(char *)malloc(sizeof(char)*(EXTENDCHAR+maxStringLen));	

	for(i=0 ; i<maxStringLen ; i++)
		newStr[i]=(*str)[i];
	
	free(*str);
	*str=newStr;

	return maxStringLen+=EXTENDCHAR;
}


/*	입력받는 문자를 동적메모리로 입력 받는 함수	*/
char * ReadString()
{
	int maxStringLen=READCHAR;
	char * str=(char *)malloc(sizeof(char)*maxStringLen);
	int idx=0;

	while(1)
	{
		if(idx>=maxStringLen)
			maxStringLen=ExtenMem(&str,maxStringLen);

		str[idx]=getchar();
		if(str[idx]=='\n')
		{
			str[idx]='\0';
			break;
		}
		idx++;
	}
	return str;
}
