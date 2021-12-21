//Dennis Preston Beaty
//CS 360 Lab 1
#include <string.h>
#include <stdlib.h>
#include <jval.h>
#include <stdlib.h>
#include "fields.h"
#include "jrb.h"
#include "jval.h"
#include "dllist.h"

//Struct Declaration
typedef struct
{
	char *name;
	char sex;
	char *father;
	char *mother;
	Dllist children;
	int visited;
	int toprint;
} Person;

int is_descendant(Person *p){
	/* I.e. we've processed this person before and he/she's ok */
	if(p->visited == 1){
		return 0;
	}
	/* I.e. the graph is messed up */
	if(p->visited == 2){
		return 1;
	}
	p->visited = 2;
	Dllist list;
	dll_traverse(list, p->children){
		Person *p2= (Person *)list->val.v;
		if(is_descendant(p2)) {
			return 1;
		}
	}
	p->visited= 1;
	return 0;
}

Person* person_exists(char* name, JRB base){
	if(name==NULL) {
		return NULL;
	}
	JRB finder= jrb_find_str(base,name);
	if(finder==NULL) {
		return NULL;
	}
	return (Person *)finder->val.v;
}

int add_parent(Person* p, Person* c, int parent){
	if(p->sex&&p->sex!= (parent?'M':'F')) {
		/* incorrect gender */
		return 1;
	}
	if(parent) {
		/* code */
		if(c->father==NULL) {
			c->father= strdup(p->name);
			return 0;
		}
		//if father already father exists
		else if(strcmp(c->father,p->name)!=0){
			return 2;
		}
		return 0;
	}
	else{
		if (c->mother==NULL) {
			c->mother= strdup(p->name);
			return 0;
		}
		//if mother already mother already exists
		else if(strcmp(c->mother,p->name)!=0){
			return 2;
		}
		return 0;
	}
}

int add_sex(char sex,Person* p){
	if(p->sex&&p->sex!= sex) {
		/* code */
		return 1;
	}
	p->sex= sex;
	return 0;
}

Person* add_person(char* key,JRB people){
	//allocate person space
	Person* p= malloc(sizeof(Person));
	//copy name to person
	p->name= strdup(key);
	p->children= new_dllist();
	//insert person into jrb tree
	jrb_insert_str(people,p->name,new_jval_v((void *)p));
	return p;
}

void jrb_deconstruct(JRB tree){
	Person *p;
	JRB tmp;
	//free each person and their reads from the tree
	jrb_traverse(tmp,tree){
		p= (Person *)tmp->val.v;
		free(p->name);
		if(p->father) {
			free(p->father);
		}
		if(p->mother) {
			free(p->mother);
		}
		free_dllist(p->children);
		free(p);
	}
	//once each person is freed then free the tree
	jrb_free_tree(tree);
}


int main(int argc, char** argv){
	//Intialize Variables
	JRB people;
	IS is;
	Person *p;
	//Set input argv1
	is= new_inputstruct(argv[1]);
	//create tree
	people = make_jrb();

	//error checking
	if(is==NULL) {
		perror(argv[1]);
		exit(1);
	}
//read by line
	while(get_line(is)>= 0) {
    for (int i = 0; i < is->NF; i++) {
			int linenum= is->line;
		//start search
		char *first_word= is->fields[0];
		char* read;
		int sizer, iter;
		/*Our first task is to calculate the ssize of our name. */
		sizer= strlen(is->fields[1]);
		for(iter = 2; iter < is->NF; iter++) {
			/* code */
			sizer= sizer+(strlen(is->fields[iter])+1);
		}
		/* We then allocate the string and copy the first word into the string. */
		read= (char *) malloc(sizeof(char)*(sizer+1));
		strcpy(read,is->fields[1]);
		/* We copy in the remaining words*/
		sizer= strlen(is->fields[1]);
		for(iter = 2; iter < is->NF; iter++) {
			read[sizer]= ' ';
			strcpy(read+sizer+1, is->fields[iter]);
			sizer= sizer+strlen(read+sizer);
		}
		//if the first word on the line is person
		if (strcmp(first_word,"PERSON")== 0){
			p = person_exists(read,people);
			//test to see if the person with that name is in the people tree. If not, you create the struct and insert it.
			if (p== NULL){
				p= add_person(read,people);
			}
		}
		else if(strcmp(first_word, "SEX")== 0){
			//sex doesnt match error
			if(add_sex(is->fields[1][0], p)== 1){
				fprintf(stderr,"Bad input - sex mismatch on line %d\n",linenum);
				//free memory
				jettison_inputstruct(is);
				free(read);
				jrb_deconstruct(people);
				exit(1);
			}
		}
		else if(strcmp(first_word,"FATHER")== 0|| strcmp(first_word,"MOTHER")== 0){
			Person *par= person_exists(read,people);
			int f= strcmp(first_word, "FATHER")== 0?1:0;
			//if parent doesnt exist add parent to tree
			if(par== NULL){
				par= add_person(read,people);
			}
			if(add_sex(f?'M':'F',par)== 1){
				//sex doesnt match error
				fprintf(stderr,"Bad input - sex mismatch on line %d\n",linenum);
				//free memory
				jettison_inputstruct(is);
				free(read);
				jrb_deconstruct(people);
				exit(1);
			}
			//adding parent to child
			int parent= add_parent(par,p,f);
			if(parent== 2){
				//two mother or father error
				fprintf(stderr,"Bad input -- child with two %s on line %d\n",f?"fathers":"mothers",linenum);
				//free memory
				jettison_inputstruct(is);
				free(read);
				jrb_deconstruct(people);
				exit(1);
			}
			else if(parent== 1){
				//sex doesnt match error
				fprintf(stderr,"Bad input - sex mismatch on line %d\n",linenum);
				//free memory
				jettison_inputstruct(is);
				free(read);
				jrb_deconstruct(people);
				exit(1);
			}
			Dllist list2;
			int exists=0;
			//Search to see if a person is a child already
			dll_traverse(list2, par->children){
				Person* tempc= (Person *)list2->val.v;
				if(strcmp(tempc->name,p->name)==0) {
					exists=1;
				}
			}
			//if not add child to parents children list
			if(exists==0) {
				dll_append(par->children,new_jval_v((void *)p));
			}
		}
		//if the first word is mother of or father of. father == 0 mother == 1
		else if(strcmp(first_word,"MOTHER_OF")== 0|| strcmp(first_word,"FATHER_OF")== 0){
			Person *c= person_exists(read,people);
			int f= strcmp(first_word,"FATHER_OF")==0?1:0;
			if (add_sex(f?'M':'F',p)== 1){
				//incorrect sex error
				fprintf(stderr,"Bad input - sex mismatch on line %d\n",linenum);
				//free memory
				jettison_inputstruct(is);
				free(read);
				jrb_deconstruct(people);
				exit(1);
			}
			//if the child doesnt exist add to tree
			if (c== NULL){
				c= add_person(read,people);
			}
			//add parent
			int parent= add_parent(p,c,f);
			if(parent== 1){
				//incorrect sex error
				fprintf(stderr,"Bad input - sex mismatch on line %d\n",linenum);
				//free memory
				jettison_inputstruct(is);
				free(read);
				jrb_deconstruct(people);
				exit(1);
			}
			else if (parent== 2){
				//two fathers or mothers error code
				fprintf(stderr,"Bad input -- child with two %s on line %d\n",f?"fathers":"mothers",linenum);
				//free memory
				jettison_inputstruct(is);
				free(read);
				jrb_deconstruct(people);
				exit(1);
			}
			//add_child(c,p);
			Dllist list;
			int exists=0;
			//Search to see if a person is a child already
			dll_traverse(list, p->children){
				Person* tempc= (Person *)list->val.v;
				if(strcmp(tempc->name,c->name)==0) {
					exists=1;
				}
			}
			//if not add child to parents children list
			if(exists==0) {
				dll_append(p->children,new_jval_v((void *)c));
			}
		}
		free(read);
		}
	}
	JRB tree; //temporary
	jrb_traverse(tree,people){
		//Cycle Check
		p = (Person *)tree->val.v;
		if (is_descendant(p)){
			fprintf(stderr,"Bad input -- cycle in specification\n");
			//free memory
			jettison_inputstruct(is);
			jrb_deconstruct(people);
			exit(1);
		}
	}
	Dllist toprint,list;
	toprint= new_dllist();
	jrb_traverse(tree,people){
		p =(Person *)tree->val.v;
		dll_append(toprint,new_jval_v((void *)p));
	}
	int parents;
	//while toprint is not empty
	while(!dll_empty(toprint)){
		p= (Person *)dll_first(toprint)->val.v;
		//take p off the head of toprint
		dll_delete_node(dll_first(toprint));
		if(!p->toprint){
			if (p->mother== NULL&& p->father== NULL){
				 parents = 2;
			 }
			Person *m= person_exists(p->mother,people);
			Person *f= person_exists(p->father,people);
			if(f&& !f->toprint){
				 parents = 0;
			}
			else if (m&& !m->toprint){
				parents = 0;
			}
			else{
				parents = 1;
			}
			if (parents== 1|| parents== 2){
				printf("%s\n",p->name);
				if(p->sex == 'M'){
					printf("  Sex: Male\n");
				}
				else if(p->sex == 'F'){
					printf("  Sex: Female\n");
				}
				else{
					printf("  Sex: Unknown\n");
				}
				if(p->father){
					printf("  Father: %s\n",p->father);
				}
				else{
					printf("  Father: Unknown\n");
				}
				if(p->mother){
					printf("  Mother: %s\n",p->mother);
				}
				else{
					printf("  Mother: Unknown\n");
				}
				if(dll_empty(p->children)){
					printf("  Children: None\n");
				}
				else{
					printf("  Children:\n");
					Dllist temp;
					dll_traverse(temp,p->children){
						Person *tChild = (Person *)temp->val.v;
						printf("    %s\n",tChild->name);
					}
				}
				printf("\n");
				p->toprint = 1;
				dll_traverse(list,p->children){
					Person *tempc = (Person *)list->val.v;
					dll_append(toprint,new_jval_v((void *)tempc));
				}
			}
		}
	}
	//free memory
	jettison_inputstruct(is);
	jrb_deconstruct(people);
	free_dllist(toprint);
	return 0;
}

//cast struct to jrb jrb_insert_str(t, p->key, new_jval_v((void *) p));
