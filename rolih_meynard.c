#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <unistd.h>
#include <limits.h>

struct uuids_list{
    char *uuidElement;
    char *fromFileName;
    struct uuids_list *nextUuid;
};

struct uuids_occur{
    char *uuidElement;
    char *fromFileName;
    int times;
    struct uuids_occur *nextUuid;
};
	
char * recover_filename(FILE * f) {
  int fd;
  char fd_path[255];
  char * filename = malloc(255);
  ssize_t n;

  fd = fileno(f);
  sprintf(fd_path, "/proc/self/fd/%d", fd);
  n = readlink(fd_path, filename, 255);
  if (n < 0)
      return NULL;
  filename[n] = '\0';
  return filename;
}

struct uuids_list *fileReader(FILE *file){
    struct uuids_list *i = (struct uuids_list*)malloc(sizeof(struct uuids_list));
    char uuidIdentifier[37];
    char * filename = malloc(255);
    filename = recover_filename(file);
    int fileRead = fscanf(file,"%s",uuidIdentifier);
    if(fileRead != EOF){
        i->uuidElement = strdup(uuidIdentifier);
        i->fromFileName = strdup(filename);
        i->nextUuid = fileReader(file);
    }
    if(fileRead == EOF) {
        return NULL;
    }
    return i;
}

void printUuids(struct uuids_list *i){

    for(;i != NULL; i = i->nextUuid) {
        printf("%s\n",i->uuidElement);
    }
}

char **getFilenames(const char* pattern1,const char* pattern2)
{
	char **filenames = NULL;
	glob_t globbuffer;
	unsigned int i;
	char *dirname = strdup(pattern1);
	char *extension = strdup(pattern2);
	char *concat_pattern = strcat(dirname,extension);
	
	if(glob(concat_pattern, 0, NULL, &globbuffer)==0)
	{
		filenames = malloc(sizeof(char*)*(globbuffer.gl_pathc+1));
		for(i = 0; i < globbuffer.gl_pathc; i++)
		{
			filenames[i] = strdup(globbuffer.gl_pathv[i]);
		}
		globfree(&globbuffer);
	}
	return filenames;
	
}

void occur(struct uuids_list *head, struct uuids_occur **result)
{
	struct uuids_list *p;
        struct uuids_occur *temp, *prev;
        p = head;

        while (p != NULL)
        {
        	temp = *result;
            	while (temp != NULL && strcmp(temp->uuidElement,p->uuidElement) != 0)
            	{
            		prev = temp;
            		temp = temp->nextUuid;
            	}
            	if (temp == NULL)
            	{
            		temp = (struct uuids_occur *)malloc(sizeof(struct uuids_occur));
            		temp->uuidElement = strdup(p->uuidElement);
            		temp->fromFileName = strdup(p->fromFileName);
            		temp->times = 1;
            		temp->nextUuid = NULL;
            		
            		if (*result != NULL)
            		{
            			prev->nextUuid = temp;
            		}
            		else
            		{
            			*result = temp;
            		}
            	}
            	else
            	{
            		temp->times += 1;
            	}
            	p = p->nextUuid;
	}

}

void printOccurences(struct uuids_occur *p)
{
	printf("****************\n  Number\tOccurence\n**************\n");

        while (p != NULL)
        {
        	printf("	%s\t\t%d\t\t%s\n", p->uuidElement, p->times, p->fromFileName);
        	p = p->nextUuid;
	}
}
   

int main()
{	
	char cwd[PATH_MAX];
	if(getcwd(cwd, sizeof(cwd))!=NULL){
		printf("Current working dir: %s\n", cwd);
	}else {
		perror("getcwd() error");
       		return 1;
   	}
	char *extension = "/*.txt";
	char **filenames = getFilenames(cwd,extension);
	unsigned int iFile = 0;
	struct uuids_list **root = (struct uuids_list **)malloc(sizeof(struct uuids_list*)*3000);
	struct uuids_occur *head = NULL;
	FILE *x;
	for(iFile = 0; filenames[iFile]!=NULL; iFile++)
	{
		root[iFile] = (struct uuids_list *)malloc(sizeof(struct uuids_list)*37);
		printf("%s\n", filenames[iFile]);
		x = fopen(filenames[iFile],"r");
		root[iFile] = fileReader(x);
		printUuids(root[iFile]);
		occur(root[iFile], &head);
	}
	printOccurences(head);
	free(filenames);
	fclose(x);
}
