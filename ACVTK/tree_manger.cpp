/***********************************************************************/
/*   FAST TREE KERNEL                                                  */
/*                                                                     */
/*   tree_manger.cpp                                                   */
/*                                                                     */
/*   Procedures to manage Trees in SVM light		       	           */
/*                                                                     */
/*   Author: joewellHe                                                 */
/*   Date: 2018-1-15                                                   */
/*                                                                     */
/*   Modified by Alessandro Moschitti 				                   */	
/*   Date: 25.10.04                                                    */
/***********************************************************************/

#include<string.h>
#include<stdlib.h>
#include<math.h>
#include "svm_common.h"

// MAX_NUMBER_OF_CHILDREN 100
// MAX_NUMBER_OF_NODES 5000

TreeNode* loadTreeInMemory(char *InputString);

char *remove_spaces(char *str){

  char *p, *q;
  p=q=str;

  while(*p!=0)
    if((*p)!=' '){*q=*p; q++;p++;}
    else p++;

  *q=0;
  return str;
}

char *remove_spaces_between_parenthesis(char *str){

  char *p, *q;
  p=q=str;
  
  while(*(p+1)!=0)
    if(*p == ')' && *(p+1) == ' ')p++;
    else {*(q+1)=*(p+1);p++;q++;}
    
  *(q+1)=0;
  return str;
}

void read_a_vector(char *line, WORD **words, long int max_words_doc, long int *totwords){
     
  register long wpos;
  long wnum;
  double weight;
  int numread,
      i, 
      pos=0;
      
  WORD *tempWords;
  char featurepair[1000],junk[1000];

    tempWords = (WORD *)my_malloc(sizeof(WORD)*(max_words_doc+5));
    wpos=0;
    while(isspace((int)line[pos])) pos++;
// TEST	printf("LINE: %s\n",line);
    while(((numread=sscanf(line+pos,"%s",featurepair)) != EOF) && (numread > 0) && 
	(wpos<max_words_doc) && strstr(featurepair,"|EV|")==NULL && strstr(featurepair,"|BV|")==NULL) {
 
     if(sscanf(featurepair,"%ld:%lf%s",&wnum,&weight,junk)==2) {
      /* it is a regular feature */
      
      //TEST       printf ("%ld:%lf\n",wnum,weight);fflush(stdout);
      if(wnum<=0) { 
	perror ("Feature numbers must be larger or equal to 1!!!\n"); 
	printf("LINE: %s\n",line);
	exit (1); 
      }
      if((wpos>0) && ((tempWords[wpos-1]).wnum >= wnum)) { 
	perror ("Features must be in increasing order!!!\n"); 
	printf("LINE: %s\n",line);
	exit (1); 
      }
      (tempWords[wpos]).wnum=wnum;
      (tempWords[wpos]).weight=(FVAL)weight; 
      wpos++;
      while((!isspace((int)line[pos])) && line[pos]) pos++;
      while(isspace((int)line[pos])) pos++;
    }
    else {
      perror ("Cannot parse feature/value pair!!!\n"); 
      printf("'%s' in LINE: %s\n",featurepair,line);
      exit (1); 
    }
 
   }
   
   if((wpos>0) && ((tempWords[wpos-1]).wnum>(*totwords))) 
   (*totwords)=(tempWords[wpos-1]).wnum;

	(*words) = (WORD *)my_malloc(sizeof(WORD)*(wpos+1));
// printf("wpos %d, tempWords[wpos-1].wnum %d, totwords %ld\n",wpos,tempWords[wpos-1].wnum,*totwords); fflush(stdout);
    for(i=0;i<wpos;i++) { 
        (*words)[i]=tempWords[i];
        //printf("%ld::%f ",(*words)[i].wnum,(*words)[i].weight); fflush(stdout);
    }

    	    /* printf("\n");*/
   	(*words)[wpos].wnum=0; /* end of the vector*/
   // (*numwords)=wpos+1;
    free(tempWords);
  }
  
void read_vector_set(char *line, DOC *example, long int max_words_doc, long int *totwords){

  int vector_index,
      j;
   
  char *V_start;
  char *V_end;
  short EV=0; // flag end vector
  VECTOR_SET *vec_set_tmp[MAX_NUMBER_OF_TREES];


// *** Read the Parse-Tree in the first line  
  example->vectors=NULL;vector_index=0;
  V_end = NULL;
  V_start=line; EV=0;
 // printf("\n\n<%s>\n",V_start);fflush(stdout);
  while(V_start!=NULL && !EV){
        V_end=strstr(V_start,"|BV|");
        if(V_end == NULL){ 
             V_end=strstr(V_start,"|EV|");
             if(V_end == NULL)V_end = V_start+strlen(V_start);
             EV=1;
/*             if(V_end == NULL) {
                printf("ERROR: an End Vector |EV| or an End Tree |ET| marker is missing (line %d)\n",example->docnum);
                exit(-1);
             }
*/      }
        //printf("\n-----------------------------------------\nParse Tree: %s\n\n\n",T_start);

/*    if(sscanf(featurepair,"qid:%ld%s",&wnum,junk)==1) {
      // it is the query id 
      doc->queryid=(long)wnum;
    }
    else if(sscanf(featurepair,"cost:%lf%s",&weight,junk)==1) {
      // it is the example-dependent cost factor 
      doc->costfactor=(double)weight;
    }
    else */
        vec_set_tmp[vector_index]=(VECTOR_SET *) my_malloc(sizeof(VECTOR_SET));
        read_a_vector(V_start, &(vec_set_tmp[vector_index]->words), max_words_doc, totwords);
        vector_index++;
        V_start = V_end + 4;
    }
  
  example->num_of_vectors=vector_index;
  if(vector_index>0){
    example->vectors =(VECTOR_SET **) my_malloc(sizeof(VECTOR_SET)*vector_index);
    for(j=0;j<vector_index;j++)
      example->vectors[j]=vec_set_tmp[j];
  }
  else example->vectors=NULL;
}

void read_tree_forest(char *line, DOC *example, int *pos){
     
  int tree_index, 
	  j; 
  char *T_start;
  char *T_end;
  short ET=0; 
  char *line_tmp;
  short TKG_temp;
  static short warning = 0;
  FOREST *forest_vec_tmp[MAX_NUMBER_OF_TREES];

  line_tmp=strdup(line);

  tree_index=0;
  T_end = NULL;
  T_start=strstr(line,"|BT|");
  ET=0;
  if(T_start!=NULL) 
	  T_start+=4;

  while(T_start!=NULL && !ET){                      
          T_end=strstr(T_start,"|BT|");
          if(T_end == NULL){ 
             T_end=strstr(T_start,"|ET|");
             ET=1;
             if(T_end == NULL) {
               printf("ERROR: the End Tree |ET| marker is missing\n");
               printf("Line Content:>>> %s\n",line_tmp);fflush(stdout);
			   exit(-1);
             }
           }
        
      *T_end = 0; 
      forest_vec_tmp[tree_index]=(FOREST *) my_malloc(sizeof(FOREST));
      forest_vec_tmp[tree_index]->root=(TreeNode *)loadTreeInMemory(T_start);
	  if(PARAM_VECT == 1 && tree_kernel_params[tree_index].kernel_type==END_OF_TREE_KERNELS){
		  if(warning==0){
				printf("\n\nWARNING: the number of parameters found in the parameter file is lower than");
				printf("\n         the number of parameters required by the trees of the forest (only %d trees are fulfilled)\n\n",tree_index);
				warning=1;
			}
		  PARAM_VECT = 0; 
		}

      TKG_temp = TKGENERALITY; 
	  if(PARAM_VECT==1)
		  TKGENERALITY=tree_kernel_params[tree_index].TKGENERALITY;
    
      if(forest_vec_tmp[tree_index]->root!=NULL && TKGENERALITY!=STRING_KERNEL) 
          extractOrderedListofTreeNodes(forest_vec_tmp[tree_index]);
      else 
		  forest_vec_tmp[tree_index]->orderedNodeSet=NULL;

	  TKGENERALITY=TKG_temp;   
      tree_index++;
      *pos = T_end - line + 4;
      T_start = T_end + 4;
  }
      
	  example->num_of_trees=tree_index;
	  if(tree_index>0){
		example->forest_vec=(FOREST **) my_malloc(sizeof(FOREST)*tree_index);
		for(j=0;j<tree_index;j++)
		  example->forest_vec[j]=forest_vec_tmp[j];
	  }
	  else example->forest_vec=NULL;

	  free(line_tmp);

}


TreeNode *buildTreeNode(char *n, int ID){

TreeNode *node;

	node = (TreeNode *) malloc(sizeof (TreeNode));
	node->pChild= (TreeNode **) malloc(sizeof(TreeNode *) * MAX_NUMBER_OF_CHILDREN);

	node->sName = strdup(n);
	node->iNoOfChildren = 0;
	node->nodeID=ID;

return node;
}
			

TreeNode *loadParseTree(char **tree, int *par_num, int *ID,char *tempstring)
{
	char *root_symbol,
		   *leaf;
			 
	int par_num_child=0;
	
	TreeNode *root;
				 		 
	if(**tree!='(') {
	   leaf=*tree;
		 
	   while(**tree!=')' && (*tree-leaf) < strlen(leaf))
		   (*tree)++;
	   if(**tree!=')'){
           printf("\n\n>>>>>ERROR: malformed tree: %s\n\n",tempstring);
           if(strcmp(*tree,"")!=0)printf(">>>>>The problem occurred at:\n%s\n\n",*tree);
           else printf(">>>>>The problem occurred at the end of the tree (closing parenthesis?)\n");
           fflush(stdout);
           exit(-1);
       }
		 
	   (**tree)=0; 
 	   do {
	       (*tree)++;
	       (*par_num)--; 
	      } 
	   while(**tree==')');
	   return buildTreeNode(leaf,(*ID)++);
	}
    else (*par_num)++; 

     
    if((strstr(*tree," ")!=NULL) && strstr(*tree,")") - *tree > strstr(*tree," ") - *tree)
          root_symbol=strtok(*tree," "); 
    else {
          root_symbol=strtok(*tree,")"); 
          (*par_num)--; 
         }

    (*tree)+=strlen(root_symbol)+1;
     while(**tree==')'){(*tree)++;(*par_num)--;} 
	  	   
    root_symbol++; 
	root=buildTreeNode(root_symbol,*ID); 

    while((*par_num)>0){
      root->pChild[root->iNoOfChildren]=loadParseTree(tree,&par_num_child,ID,tempstring);
  	  root->iNoOfChildren++;
	  (*par_num)+=par_num_child;
    }
        
    root->nodeID=(*ID)++;	
    
    if(*par_num > 0){
       printf("\n\n>>>>>ERROR: malformed tree: %s\n\n",tempstring);
       if(strcmp(*tree,"")!=0)printf(">>>>>The problem occurred at:\n%s\n\n",*tree);
       else printf(">>>>>The problem occurred at the end of the tree (closing parenthesis?)\n");
       fflush(stdout);
       exit(-1);
    }
   
	return root;
}
	

void putDynamic(TreeNode *node){

    int i;
    TreeNode **temp;		

	if(node->iNoOfChildren>0){
	   temp=node->pChild;
	   node->pChild = (TreeNode **) malloc(sizeof(TreeNode *)*node->iNoOfChildren);
	   for(i=0;i<node->iNoOfChildren;i++){
	      node->pChild[i]=temp[i];
          putDynamic(temp[i]);
       }
	   free(temp);
	} 
	else {free(node->pChild);node->pChild=NULL;}
}


TreeNode* loadTreeInMemory(char *InputString){	
	
	char tempstring[MAX_PARSE_TREE_LENGTH];
	TreeNode *pRoot=NULL;
  	char *input;
	int par_num=0,
	    ID = 0;
	strcpy(tempstring,InputString);

	input=remove_spaces_between_parenthesis(InputString);
	while(*input==' ' || *input=='\t' ) input++;

	if(strlen(input)==0) return NULL;

	if(strstr(input,"(")==NULL) { 
       printf("\n\nERROR: A tree with no left parenthesis \n\n");
       exit(-1);
    }

	pRoot= loadParseTree(&input,&par_num,&ID,tempstring);

	if(par_num!=0){
       if(par_num>0)
          printf("\n\n>>>>>ERROR: too many '(' in the tree: %s\n\n",tempstring);
       if(par_num<0)
          printf("\n\n>>>>>ERROR: too many ')' in the tree: %s\n\n",tempstring);
           
       if(strcmp(input,"")!=0)printf(">>>>>The problem occurred at:\n%s\n\n",input);
           else printf(">>>>>The problem occurred at the end of the tree\n");
       fflush(stdout);
       exit(-1);
    }
                   
    if(strlen(input)>1) { 
       printf("\n\n>>>>>ERROR: malformed tree: %s\n\n",tempstring);
       if(strcmp(input,"")!=0)printf(">>>>>The problem occurred at:\n%s\n\n",input);
       else printf(">>>>>The problem occurred at the end of the tree\n");
       fflush(stdout);
       exit(-1);
    }


    putDynamic(pRoot);

	return pRoot;

}


void freeTree(TreeNode *node){
 int i;

  if(node != NULL){
   if(node->iNoOfChildren>0){
      for(i=0;i<node->iNoOfChildren;i++)
         freeTree(node->pChild[i]);
      free(node->sName);
      free(node->pChild);
      free(node->production);
      free(node);
   }
    else{
         free(node->sName);
    	 free(node);
    }
  }
}


void freeForest(DOC *d){

    int j;
    for(j=0; j<d->num_of_trees;j++)
	     if(d->forest_vec[j]->root != NULL){
         freeTree(d->forest_vec[j]->root); // free tree
         if(d->forest_vec[j]->orderedNodeSet!=NULL)
         free(d->forest_vec[j]->orderedNodeSet);
         free(d->forest_vec[j]);
        }
   free(d->forest_vec);
}


void freeVectorSet(DOC *d){

    int j;
        
    for(j=0; j<d->num_of_vectors;j++)
	    if(d->vectors[j] != NULL){
         free(d->vectors[j]->words);
         free(d->vectors[j]);
        }
   free(d->vectors);
}


void freeExample(DOC *d){
     freeVectorSet(d);
     freeForest(d);
}


void writeTreeString(TreeNode *node){

    int i;

if(node==NULL) return;

	if(node->iNoOfChildren>0){
	   	 printf("(%s ",node->sName); 
		 for(i=0;i<node->iNoOfChildren;i++){
	    	   writeTreeString(node->pChild[i]);
		 }
         printf(")"); 
	}
	else printf("(%s)",node->sName);
}

/*
void getStringTree(TreeNode *node, char *temp){

    int i;

	if(node == NULL) return;

	if(node != NULL){ 
	   if(node->iNoOfChildren>0){
	            strcat(temp,"(");
		    strcat(temp,node->sName);
		    strcat(temp," "); 
		    for(i=0;i<node->iNoOfChildren;i++)
		       getStringTree(node->pChild[i], temp);
		    strcat(temp,")");
	   }
	   else strcat(temp,node->sName);
    }
}*/
void getStringTree(TreeNode *node, char *temp){

    int i;

	if(node == NULL) return;
       	
	if(node->iNoOfChildren>0){
	         strcat(temp,"(");
		 strcat(temp,node->sName);
		 strcat(temp," "); 
		 for(i=0;i<node->iNoOfChildren;i++)
		    getStringTree(node->pChild[i], temp);
	         strcat(temp,")");
	   }
	   else {strcat(temp,"("); strcat(temp,node->sName); strcat(temp,")");}
}


long partition(OrderedTreeNode *vect,long part_low, long part_high) {
	
	long lastsmall;
	long comp1;
	long i;
	char *median_val,*transit;
	TreeNode *trans_node;

	comp1=(part_low+part_high) / 2;		

	transit=vect[part_low].sName;
	vect[part_low].sName=vect[comp1].sName;
	vect[comp1].sName=transit;

	trans_node=vect[part_low].node;
	vect[part_low].node=vect[comp1].node;
	vect[comp1].node=trans_node;

	median_val=vect[part_low].sName;
	lastsmall=part_low;

	for (i=part_low+1; i<=part_high; i++) {
		if (strcmp(vect[i].sName,median_val)<0) {
			lastsmall++;
			transit=vect[lastsmall].sName;
			vect[lastsmall].sName=vect[i].sName;
			vect[i].sName=transit;

			trans_node=vect[lastsmall].node;
			vect[lastsmall].node=vect[i].node;
			vect[i].node=trans_node;

		}
	}
	transit=vect[part_low].sName;
	vect[part_low].sName=vect[lastsmall].sName;
	vect[lastsmall].sName=transit;

	trans_node=vect[part_low].node;
	vect[part_low].node=vect[lastsmall].node;
	vect[lastsmall].node=trans_node;


	return lastsmall;
}

void quicksort(OrderedTreeNode *vect,long qk_low, long qk_high) {
	if (qk_low < qk_high)	{
		long median=partition(vect,qk_low, qk_high);
		quicksort(vect,qk_low, median);
		quicksort(vect,median+1, qk_high);
	}
}


void fillTheNodeList(TreeNode *N, char * parentname, OrderedTreeNode *list, int *counter){

    int i,pre_term;
    char production[MAX_PRODUCTION_LENGTH];

	*production=0;
	if((N->iNoOfChildren>0) ){
       if(TKGENERALITY==PARTIAL_TREE_KERNEL || TKGENERALITY == NOLEAVES_PT_KERNEL)
		   sprintf(production,"%s",N->sName); 
       else 
		   sprintf(production,"%s->",N->sName);             

	   pre_term=1;

	   for(i=0;i<N->iNoOfChildren;i++){
          if(TKGENERALITY!= PARTIAL_TREE_KERNEL && TKGENERALITY != NOLEAVES_PT_KERNEL){
	         strcat(production," ");  
	         strcat(production,N->pChild[i]->sName);
			 
          }
	      if(N->pChild[i]->iNoOfChildren>0) 
			  pre_term=0;
		  fillTheNodeList(N->pChild[i],N->sName, list, counter);
	   }
	   list[*counter].node=N;
	   N->pre_terminal=pre_term;
	   if(N->pre_terminal==1)
		   sprintf(production,"%s-> %s_POS",N->sName,N->sName);

	   N->production=list[*counter].sName=strdup(production);
	   (*counter)++;
	}
	else if(TKGENERALITY >= BOW_SST_KERNEL) { 
               list[*counter].node=N;
	           N->production=list[*counter].sName=strdup(N->sName);
	           N->pre_terminal=-1; 
	          (*counter)++;        
			 
    }
	else if(TKGENERALITY == SUBSET_TREE_KERNEL){
		 list[*counter].node=N;
	     N->production=list[*counter].sName=strdup(N->sName);
	     N->pre_terminal=-1; 
	     (*counter)++;        
	}
    else {
		N->pre_terminal=-1;
		N->production=NULL;
		list[*counter].node=NULL;
	}
	        
}


void extractOrderedListofTreeNodes(FOREST *tree){

int counter=0;
int i=0;
    if(tree->root==NULL){ 
       tree->listSize=0;
	   tree->orderedNodeSet=NULL;
	   return;
     }

     tree->orderedNodeSet=(OrderedTreeNode *)malloc(sizeof(OrderedTreeNode)*(tree->root->nodeID+1));
     fillTheNodeList(tree->root, "root",tree->orderedNodeSet, &counter);     

     quicksort(tree->orderedNodeSet,0,counter-1);
     tree->listSize=counter;
}