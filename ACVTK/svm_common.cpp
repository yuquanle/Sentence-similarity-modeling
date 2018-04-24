/***********************************************************************/
/*                                                                     */
/*   svm_common.cpp                                                    */
/*                                                                     */
/*   Author: Yuquan Le, joewellHe                                      */
/*   Date: 2018-1-15                                                   */
/*                                                                     */
/*   Modified by Thorsten Joachims                                     */
/*   Date: 02.07.02                                                    */
/***********************************************************************/

# include "ctype.h"
# include "svm_common.h"
# include "kernel.h"           /* this contains a user supplied kernel */

long   verbosity;              /* verbosity level (0-4) */

void read_documents(char *, DOC *, long, long, long *, long *, KERNEL_PARM *, map<string,double*>);


CFLOAT kernel(KERNEL_PARM *kernel_parm, DOC *a, DOC *b) 
     /* calculate the kernel function */
{
    // double k1;
  switch(kernel_parm->kernel_type) { 
    case 0: /* linear */ 
            return(CFLOAT)sprod(a, b); 
    case 1: /* polynomial */   
            return ((CFLOAT)pow(kernel_parm->coef_lin*sprod(a, b)
                                +kernel_parm->coef_const,(double)kernel_parm->poly_degree));
    case 2: /* radial basis function */
            return((CFLOAT)exp(-kernel_parm->rbf_gamma*(a->twonorm_sq-2*sprod(a,b)+b->twonorm_sq)));
    case 3: /* sigmoid neural net */
            return((CFLOAT)tanh(kernel_parm->coef_lin*sprod(a,b)+kernel_parm->coef_const)); 
    case 4: /* custom-kernel supplied in file kernel.h */
            return((CFLOAT)custom_kernel(kernel_parm,a,b));
    case 5: /* combine kernels */
            return ((CFLOAT)custom_kernel(kernel_parm,a,b));

default: /* Advanced vectorial kernels*/
             printf("\nNo kernel corresponding to -t = %ld option \n",kernel_parm->kernel_type);
             exit(-1);

      }
}

double sprod(DOC *a, DOC *b){ // compatibility with standard svm-light
       if(a->num_of_vectors>0 && b->num_of_vectors>0 ){
             if(a->vectors[0]==NULL || b->vectors[0]==NULL){
                printf("ERROR: first vector not defined (with a traditional kernel it must be defined)\n");
                exit(-1);
             }
             else return sprod_ss(a->vectors[0]->words,b->vectors[0]->words);
       }      
       return 0;
}

double sprod_i(DOC *a, DOC *b, int i, int j){ // compatibility with standard svm-light
       if(a->num_of_vectors>0 && b->num_of_vectors>0 ){
             if(a->vectors[i]==NULL || b->vectors[j]==NULL){
                printf("ERROR: first vector not defined (with a traditional kernel it must be defined)\n");
                exit(-1);
             }
             else return sprod_ss(a->vectors[i]->words,b->vectors[j]->words);
       }      
       return 0;
}

double sprod_ss(WORD *a, WORD *b)
     /* compute the inner product of two sparse vectors */
{
    register FVAL sum=0;
    register WORD *ai,*bj;
    ai=a;
    bj=b;
    while (ai->wnum && bj->wnum) {
      if(ai->wnum > bj->wnum) {
	    bj++;
      }
      else if (ai->wnum < bj->wnum) {
	    ai++;
      }
      else {
	    sum+=ai->weight * bj->weight;
	    ai++;
	    bj++;
      }
    }
    return((double)sum);

    return 1;
}

WORD* sub_ss(WORD *a, WORD *b) 
     /* compute the difference a-b of two sparse vectors */
{
    register WORD *sum,*sumi;
    register WORD *ai,*bj;
    long veclength;
  
    ai=a;
    bj=b;
    veclength=0;
    while (ai->wnum && bj->wnum) {
      if(ai->wnum > bj->wnum) {
	veclength++;
	bj++;
      }
      else if (ai->wnum < bj->wnum) {
	veclength++;
	ai++;
      }
      else {
	veclength++;
	ai++;
	bj++;
      }
    }
    while (bj->wnum) {
      veclength++;
      bj++;
    }
    while (ai->wnum) {
      veclength++;
      ai++;
    }
    veclength++;

    sum=(WORD *)my_malloc(sizeof(WORD)*veclength);
    sumi=sum;
    ai=a;
    bj=b;
    while (ai->wnum && bj->wnum) {
      if(ai->wnum > bj->wnum) {
	(*sumi)=(*bj);
	sumi->weight*=(-1);
	sumi++;
	bj++;
      }
      else if (ai->wnum < bj->wnum) {
	(*sumi)=(*ai);
	sumi++;
	ai++;
      }
      else {
	(*sumi)=(*ai);
	sumi->weight-=bj->weight;
	sumi++;
	ai++;
	bj++;
      }
    }
    while (bj->wnum) {
      (*sumi)=(*bj);
      sumi->weight*=(-1);
      sumi++;
      bj++;
    }
    while (ai->wnum) {
      (*sumi)=(*ai);
      sumi++;
      ai++;
    }
    sumi->wnum=0;
    return(sum);
}

double sprod_ns(double *vec_n, WORD *vec_s)
{
  register double sum=0;
  register WORD *ai;
  ai=vec_s;
  while (ai->wnum) {
    sum+=(vec_n[ai->wnum]*ai->weight);
    ai++;
  }
  return(sum);
}

void read_documents(char *docfile, DOC *docs, 
		    long int max_words_doc, long int ll, 
		    long int *totwords, long int *totdoc, KERNEL_PARM *kernel_parm)
{
  char *line;
  DOC doc;
  long dnum=0;
  FILE *docfl;

  line = (char *)my_malloc(sizeof(char)*ll);

  if ((docfl = fopen (docfile, "r")) == NULL)
  { printf("\n\nerror\n\n");perror (docfile); exit (1); }

  if(verbosity>=1) {
    printf("Reading examples into memory...\n"); 
	fflush(stdout);
  }
  dnum=0;
  (*totwords)=0;
  
  while((!feof(docfl)) && fgets(line,(int)ll,docfl)) {

      doc.docnum=dnum+1;
	  doc.docsim = 0; // initial value
	  //doc.isant=0;
      if(line[0]=='\n'){
		  continue;
      }
	  if(doc.docnum % 10 == 0) 	printf("%dth computation has done\n",doc.docnum);

      if(!parse_document(line, &doc, totwords, max_words_doc, kernel_parm)) {
         printf("\nParsing error in line %ld!\n%s",dnum,line);
         exit(1);
      }

    docs[dnum].costfactor = doc.costfactor;
    
    docs[dnum].forest_vec = doc.forest_vec;
	docs[dnum].num_of_trees = doc.num_of_trees;
	docs[dnum].vectors = doc.vectors;
	docs[dnum].num_of_vectors = doc.num_of_vectors;
	docs[dnum].docsim = doc.docsim;
    // less than 5 basic kernels and greater than 50 only vectors (to save memory)
    if (kernel_parm->kernel_type<4) { // from 0 to 3 are original kernels => no trees
       freeForest(&doc); // save memory by freeing trees
       docs[dnum].num_of_trees = 0;
       docs[dnum].forest_vec =NULL;
       kernel_parm->second_kernel=kernel_parm->kernel_type;
    }   
      
    docs[dnum].docnum=dnum;

    dnum++;  
    if(verbosity>=1) {
      if((dnum % 100) == 0) {
	           printf("%ld..",dnum); 
			   fflush(stdout);
      }
    }
  } 

  fclose(docfl);
  free(line);

  if(verbosity>=1) {
    fprintf(stdout, "OK. (%ld examples read)\n", dnum);
  }

fflush(stdout);

  (*totdoc)=dnum;

}

int parse_document(char *line, DOC *doc,  
		   long int *totwords, long int max_words_doc, KERNEL_PARM *kernel_parm)
{
  int pos=0;

  doc->vectors=NULL;
  doc->num_of_vectors=0;
  doc->costfactor=1;

  while(isspace((int)line[pos])&& line[pos]!=0) pos++;// go to "|BV|" marker or to the first number
  read_tree_forest(line, doc, &pos);// read the two tree forest

  doc->docnum=-1;
  evaluateNorma(kernel_parm,doc);
  
  return(1);
}


void nol_ll(char *file, long int *nol, long int *wol, long int *ll) 
     /* Grep through file and count number of lines, maximum number of
        spaces per line, and longest line. */
{
  FILE *fl;
  int ic;
  char c;
  long current_length,current_wol;

  if ((fl = fopen (file, "r")) == NULL)
  { printf("\n\nerror");perror (file); exit (1); }
  current_length=0;
  current_wol=0;
  (*ll)=0;
  (*nol)=1;
  (*wol)=0;
  while((ic=getc(fl)) != EOF) {
	c=ic;
    current_length++;
	if(c==32)
		current_wol++;
    if(c == '\n') {
      (*nol)++;
      if(current_length>(*ll)) {
	    (*ll)=current_length;
      }
      if(current_wol>(*wol)) {
	    (*wol)=current_wol;
      }
      current_length=0;
      current_wol=0;
    }
  }
  fclose(fl);

  if(current_length>(*ll)) {
	(*ll)=current_length;
   }
 }

long minl(long int a, long int b)
{
  if(a<b)
    return(a);
  else
    return(b);
}

long maxl(long int a, long int b)
{
  if(a>b)
    return(a);
  else
    return(b);
}

long get_runtime(void)
{
  clock_t start;
  start = clock();
  return((long)((double)start*100.0/(double)CLOCKS_PER_SEC));
}


void *my_malloc(size_t size)
{
  void *ptr;
  ptr=(void *)malloc(size);
  if(!ptr) { 
    perror ("Out of memory!\n"); 
    exit (1); 
  }
  return(ptr);
}

void copyright_notice(void)
{
  printf("\nCopyright: Thorsten Joachims, thorsten@ls8.cs.uni-dortmund.de\n\n");
  printf("This software is available for non-commercial use only. It must not\n");
  printf("be modified and distributed without prior permission of the author.\n");
  printf("The author is not responsible for implications from the use of this\n");
  printf("software.\n\n");
}

