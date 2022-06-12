#include <stdio.h>             // for I/O
#include <stdlib.h>            // for lib functions
#include <libgen.h>            // for dirname()/basename()
#include <string.h>

typedef struct node {
	char  name[64];       // node's name string
	char  type;
	struct node *child, *sibling, *parent;
}NODE;


NODE *root, *cwd, *start, *ptracker,*lstracker; //ptracker is used as helper to point to correct nodes like how start does
char command[16], pathname[64];
FILE *fp;

char *gpath[128];               // global gpath[] to hold token strings
char *filetype;					//used to check file type for reload function
char *stringname[128];			//used 
char *name[64];                // token string pointers
int  n;                        // number of token strings
int t;							//global flag variable used for mkdir to see if dir found or not
char *stringtemp[500];			//used to read inorder print of nodes and store in string for save()
char *stringtemp2[500];			//used to reverse order of stringtemp

char dname[64], bname[64];     // dirname, basename of pathname

//               0       1      2    
char *cmd[] = { "mkdir", "ls", "quit", "cd","pwd","creat","rmdir","rm","save","reload","menu","quit", 0 };

int findCmd(char *command)
{
	int i = 0;
	while (cmd[i]) {
		if (strcmp(command, cmd[i]) == 0)
			return i;
		i++;
	}
	return -1;
}

NODE *search_child(NODE *parent, char *name)
{
	NODE *p;
	printf("search for %s in parent DIR\n", name);
	p = parent->child;

	if (p == 0)
		return 0;
	while (p) {
		if (strcmp(p->name, name) == 0)
		{
			
			ptracker = p;
			lstracker=ptracker;
				return p;//returns this if node is found
		}
		p = p->sibling;
	}
	
	return 0;//returns this if node not found
}

NODE *search_child_cd(NODE *parent, char *name)
{
	NODE *p;
	printf("search for %s in parent DIR\n", name);
	p = parent->child;
	if (p == 0)
		
		return 0;
	while (p) {
		if (strcmp(p->name, name) == 0)
		{
			if (p->type == 'F')
			{
				printf("File type found\n");
				return 0;
			}
			cwd = p;
			return p;
		}
		p = p->sibling;
	}

	return 0;
}

int insert_child(NODE *parent, NODE *q)
{
	NODE *p;
	printf("insert NODE %s into parent child list\n", q->name);

	p = parent->child;
	if (p == 0)
		parent->child = q;
	else {
		while (p->sibling) 
			p = p->sibling;
		p->sibling = q;
	}
	q->parent = parent;
	q->child = NULL;//changed from 0
	q->sibling = NULL;//CHANGED FROM 0
}

/***************************************************************
 This mkdir(char *name) makes a DIR in the current directory
 You MUST improve it to mkdir(char *pathname) for ANY pathname
****************************************************************/

int mkdir(char *pathname)
{
	NODE *p, *q;
	

	printf("mkdir: name=%s\n", pathname);

	// write YOUR code to not allow mkdir of /, ., ./, .., or ../
	//checks for name

	if (pathname[0] == '.')
	{
		printf("Can not allow mkdir with . as intial character \n");
		return -1;
	}
	if (strcmp(pathname, "/") == 0)
	{
		printf("can not allow mkdir of / \n");
		return -1;
	}

	if (pathname[0] == '/')
	{
		start = root;
		ptracker = root;
	}
	else
		start = cwd;

	printf("check whether %s already exists\n", pathname);
	
	//if (pathname[0] == '/')
	//{
	//	//memmove(name, name + 1, strlen(name));

	//}

	helperfun(pathname);
	if (t == 1)
	{
		printf("name %s already exists, mkdir FAILED\n", pathname);
		return -1;
	}
	printf("%s", name[n-1]);
	printf("--------------------------------------\n");
	printf("ready to mkdir %s\n", name[n - 1]);
	q = (NODE *)malloc(sizeof(NODE));
	q->type = 'D';
	strcpy(q->name, name[n - 1]);
	
	printf("ptracker:%s\n", ptracker->name);
	if (ptracker == NULL)
	{
		ptracker = root;
	}
	insert_child(ptracker, q);
	printf("mkdir %s OK\n", name[n - 1]);
	printf("--------------------------------------\n");

	return 1;
}


int creat(char *pathname)
{
	NODE *p, *q;


	printf("creat: name=%s\n", pathname);

	// write YOUR code to not allow mkdir of /, ., ./, .., or ../
	//checks for name

	if (pathname[0] == '.')
	{
		printf("Can not allow creat with . as intial character \n");
		return -1;
	}
	if (strcmp(pathname, "/") == 0)
	{
		printf("can not allow creat of / \n");
		return -1;
	}

	if (pathname[0] == '/')
	{
		start = root;
		ptracker = root;
	}
	else
		start = cwd;

	printf("check whether %s already exists\n", pathname);
	

	helperfun(pathname);
	if (t == 1)
	{
		printf("name %s already exists, creat FAILED\n", pathname);
		return -1;
	}
	printf("%s", name[n - 1]);
	printf("--------------------------------------\n");
	printf("ready to creat  %s\n", name[n - 1]);
	q = (NODE *)malloc(sizeof(NODE));
	q->type = 'F';
	strcpy(q->name, name[n - 1]);

	printf("ptracker:%s\n", ptracker->name);
	if (ptracker == NULL)
	{
		ptracker = root;
	}
	insert_child(ptracker, q);
	printf("creat %s OK\n", name[n - 1]);
	printf("--------------------------------------\n");

	
	return 1;
}

// This ls() list CWD. You MUST improve it to ls(char *pathname)
int ls(char *pathname)
{
	//NODE *p = cwd->child;
	NODE *p;
	p = cwd->child;
	if (strcmp(pathname,"")==0)// 
	{
		p = cwd->child;

	}
	else
	{
		
		helperfun(pathname);
		p = lstracker;
		p = p->child;
	}
	

	printf("cwd contents = ");
	while (p) {
		printf("[%c %s] ", p->type, p->name);
		p = p->sibling;
	}
	printf("\n");
	
}

int quit()
{
	save(pathname);
	printf("Program exit\n");
	exit(0);

	
}

int initialize()  // create / node, set root and cwd pointers
{
	root = (NODE *)malloc(sizeof(NODE));
	strcpy(root->name, "/");
	root->parent = root;
	root->sibling = 0;
	root->child = 0;
	root->type = 'D';
	cwd = root;
	printf("Root initialized OK\n");
}

int main()
{
	int index;
	char line[128];

	initialize();

	printf("NOTE: commands = [mkdir|ls|quit|cd|pwd|creat|rmdir|rm|save|reload|menu|quit]\n");

	while (1) {
		printf("Enter command line : ");
		fgets(line, 128, stdin);
		line[strlen(line) - 1] = 0;
		*pathname = NULL;//////////////////////////

		sscanf(line, "%s %s", command, pathname);
		printf("command=%s pathname=%s\n", command, pathname);

		if (command[0] == 0)
			continue;
		ptracker = cwd;
		index = findCmd(command);

		switch (index) {
		case 0: mkdir(pathname); break;
		case 1: ls(pathname);            break;
		case 2: quit();          break;
		case 3: cd(pathname);    break;
		case 4: pwd();			 break;
		case 5: creat(pathname); break;
		case 6: rmdir(pathname); break;
		case 7: rm(pathname); break;
		case 8: save(pathname); break;
		case 9: reload(); break;
		case 10: menu();
			//printf("NOTE: commands = [mkdir|ls|quit|cd|pwd|creat|rmdir|rm|save|reload|menu|quit]\n");
			break;
		case 11: quit(); 
			return 0;
			break;
		

		}
	}
}


//********************* YOU DO THE FOLLOWING *************************
int tokenize(char *pathname)//tokenizes fed in string by /path/path....or path/path....
{
	char *s;
	int i = 0;
	n = 0;


	do
	{
		
		if (i == 0)
		{
			s = strtok(pathname, " /");
			name[i] = s;
			printf("%s ", name[i]);
			

		}
		else
		{
			s = strtok(0, "/");
			if (s != NULL)
			{
				name[i] = s;
				printf("%s ", name[i]);
			}
			

		}
		if (s != NULL)
		{
			i++;
			n++;
		}
		
	} while (s);


}

NODE *path2node(char *pathname)//takes fed in string, passes it to tokenize, then passes tokenized string to search_child and returns true or false
{
	// return pointer to the node of pathname, or NULL if invalid
	if (pathname[0] == '/')
	{
		start = root;
	}
	else
	{
		start = cwd;
	}
	tokenize(pathname);
	NODE *node = start;

	for (int i = 0; i < n; i++) 
	{
		node = search_child(node, name[i]);
		if (node == 0)
		{
			t = 0;
			return NULL;//not found
		}
	}
	t = 1;
	return node;




}



int dir_base_name(char *pathname)//got the code for this function from the book for the class that was part of the directions for this assignement
{
	//divide pathname into dirname in bname[], basename in bname[]
	char temp[128]; // dirname(), basename() destroy original pathname
	strcpy(temp, pathname);
	strcpy(dname, dirname(temp));
	strcpy(temp, pathname);
	strcpy(bname, basename(temp));


}

//Write your pwd(), rmdir(), creat(), rm(), save(), reload() functions

int pwd() //print absolute pathname of CWD
{
//(1). Save the name (string) of the current node
//(2). Follow parentPtr to the parent node until the root node;
//(3). Print the names by adding / to each name string

	NODE * temp = cwd;
	while (temp->parent != root)
	{
		printf("%s/",temp->name);
		temp = temp->parent;

	} 
	if (temp->parent == root)
	{
		printf("%s", temp->name);
	}
	printf("root\n");
}

int cd(char *pathname)
{
	//(1). find pathname node;
	//(2). check it's a DIR;
	//(3). change CWD to point at DIR

	  //code to return to root node if null space entered or if / entered
	if (strcmp(pathname, " ") == 0)
	{
		cwd = root;
		return 0;
	}
	if (strcmp(pathname, "/") == 0)
	{
		cwd = root;
		return 0;
	}
	//checks current node we're at if child node exists
	NODE *p;

	p = path2node(pathname);
	if (p)
	{
		printf("directory found\n");
		cwd = p;
	}
	else
	{
		printf("Can not find directory in current node\n");
		return -1;
	}




}

int rmdir(char *pathname)//passes string to path2node to point to desired dir, checks if meets criteria for deletion, rearranges child/sibling/parent pointers
{
	NODE *temp1, *temp2, *p;
	if (pathname[0] == '/')
		start = root;
	else
		start = cwd;

	p = path2node(pathname);
	if (p)
	{
		if (p->type == 'D')
		{
			printf("directory found\n");
			if (p->child != NULL)
			{
				printf("directory not empty\n");
				return -1;
			}
			//case1: node to be deleted is parents direct child: parent->child
			if (p->parent->child == p)
			{
				
				p->parent->child = p->sibling;
				
				free (p);
				
				return;
			}
			//case2: node to be deleted is parent's secondary or greater child: parent->child->sibling
			else
			{
				if (strcmp(p->parent->child->sibling, pathname) == 0)//node to be deleted is first sibling of parent's direct child
				{
					p->parent->child->sibling = p->sibling;
					free(p);
					return;
				}
				else
				{
					NODE *temp;
					temp = p->parent->child;
					while (strcmp(temp->sibling, pathname) != 0)//temp will stop at node just before p 
					{
						temp = temp->sibling;
					}
					temp->sibling = p->sibling;
					free(p);
					return;
				}

			}
		}
		else
		{
			printf("wrong filetype\n");
			return -1;
		}
		
		
	}
	else
	{
		printf("Can not find directory in current node\n");
		return -1;
	}


}

int helperfun(char *pathname)//used by mkdir and creat to access path2node because of annoying compiler issues
{
	NODE *p;

	p=path2node(pathname);
	
	return;

}

int rm(char *pathname)//functions identically to rmdir just checks for file type instead
{
	NODE *temp1, *temp2, *p;
	if (pathname[0] == '/')
		start = root;
	else
		start = cwd;

	p = path2node(pathname);
	if (p)
	{
		if (p->type == 'F')
		{
			printf("file found\n");
			if (p->child != NULL)
			{
				printf("file not empty\n");
				return -1;
			}
			//case1: node to be deleted is parents direct child: parent->child
			if (p->parent->child == p)
			{
				
				p->parent->child = p->sibling;


				free(p);
				
				return;
			}
			//case2: node to be deleted is parent's secondary or greater child: parent->child->sibling
			else
			{
				if (strcmp(p->parent->child->sibling, pathname) == 0)//node to be deleted is first sibling of parent's direct child
				{
					p->parent->child->sibling = p->sibling;
					free(p);
				
					return;
				}
				else//node to be deleted isn't first child's direct sibling so have to iterate to find it
				{
					NODE *temp;
					temp = p->parent->child;
					while (strcmp(temp->sibling, pathname) != 0)//temp will stop at node just before p 
					{
						temp = temp->sibling;
					}
					temp->sibling = p->sibling;
					free(p);
					
					return;
				}

			}
		}
		else
		{
			printf("wrong filetype\n");
			return -1;
		}


	}
	else
	{
		printf("Can not find directory in current node\n");
		return -1;
	}


}

int save(filename) //opens/closes file, triggers savefunction that parses through tree
{
	fp = fopen("myfile.txt", "w+"); // fopen a FILE stream for WRITE
	cwd = root;
	savehelper(cwd);
	
	
	
	fclose(fp); // close FILE stream when done



}

int savehelper(NODE *pathname)//works similarly to PrintInOrder, recursively takes node, feeds node to stringprint to get entire string from node to root, then saves to file
{
	
	if (pathname == NULL)
	{
		return;
	}
	
	stringprint(stringname, pathname);

	

	for (int i = 0; i < n; i++)
	{
		gpath[i] = stringtemp2[i]; 
		printf("%s\n", gpath[i]);
	}

	if (cwd->type == 'F')
	{
		fprintf(fp, "F,", NULL); // print a line to file

		for (int i = 0; i < n; i++)
		{
			fprintf(fp, "/%s", gpath[i]); // print a line to file

		}
		fprintf(fp, "\n", NULL); // print a line to file

	}
	else
	{
		fprintf(fp, "D,", NULL); // print a line to file
		for (int i = 0; i < n; i++)
		{
			fprintf(fp, "/%s", gpath[i]); // print a line to file

		}
		fprintf(fp, "\n", NULL); // print a line to file

	}

	savehelper(pathname->child);
	savehelper(pathname->sibling);
}

int stringprint(char *string, NODE *pathname)//follows fed in string from node to root, saves as string in string temp, then reverses string node order for desired format for file
{

	NODE * temp = pathname;

	char st[] = "/";

	
	 n=0;
	if (temp != root) {


		while (temp->parent != root)
		{
			
			stringtemp[n] = temp->name; 

			printf("temp%s", temp->name);
			printf("%s", stringtemp[n]);
			temp = temp->parent;

			n++;
		}
		if (temp->parent == root)
		{
			
			stringtemp[n] = temp->name;
			printf("temp%s", temp->name);
			printf("%s", stringtemp[n]);


			n++;
		}
	}
	else
	{
		
		stringtemp2[0] = "/";
		n++;
		return;

	}
	int k = n;
	for (int i = 0; i < n; i++)
	{
		printf("\n");
	
		stringtemp2[k-1] = stringtemp[i];
		
		printf("string1 %d %s string2 %d %s\n", i, stringtemp[i],k,stringtemp2[k]);
		k--;
		
	}
	printf("n:%d", n);
	


}

int reload()//reads opens/closes file, reads file and tokenizes read in line, passes tokenized line to creat or mkdir 
{
	root->child = NULL;
	cwd = root;
	//first line that just has root node: D, /
	fp = fopen("myfile.txt", "r"); // fopen a FILE stream for WRITE
	fscanf(fp, "%s", gpath);//garbage first line
	
	while (!feof(fp))
	{
		fscanf(fp, "%s", gpath);//garbage first line
		filetype = strtok(gpath,",");
		printf("%s\n", filetype);
		if (filetype != NULL) 
		{
			if (strcmp(filetype, "D") == 0)
			{
				filetype = strtok(NULL, "\n");
				if (filetype != NULL)
				{
					mkdir(filetype);

				}
			}
			if (filetype != NULL) 
			{
				if (strcmp(filetype, "F") == 0)
				{
					filetype = strtok(NULL, "\n");
					if (filetype != NULL)
					{
						creat(filetype);

					}

				}
			}
		}
	}


	fclose(fp);
}

int menu()
{
		printf("NOTE: commands = [mkdir|ls|quit|cd|pwd|creat|rmdir|rm|save|reload|menu|quit]\n");

}