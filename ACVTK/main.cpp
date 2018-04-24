/***********************************************************************/
/*                                                                     */
/*   main.cpp                                                          */
/*                                                                     */
/*   Author: Yuquan Le, joewellHe                                      */
/*   Date: 2018-1-15                                                   */
/*                                                                     */
/*   Modified by Thorsten Joachims                                     */
/*   Date: 02.07.02                                                    */
/*                                                                     */
/***********************************************************************/


#include <stdio.h>
#include <math.h>
#include "svm_common.h"
map<string,double*> dict;

char docfile[1000];           /* file with training examples */
char outfile[1000];           /* file for resulting kernel compute */
char embeddingfile[1000];      /* file for word-embeding */

void read_input_parameters(int, char **, char *,  char *, char*, long *, long *, KERNEL_PARM *,double, double);
int ReadEmbedding(const char *file_name);


int main (int argc, char* argv[])
{ 
  double MU=0.2, LAMBDA=0.1; //MU, LAMBDA
  DOC *docs;  /* training examples */
  long max_docs,max_words_doc;
  long totwords,totdoc,ll,i;
  long kernel_cache_size;
  long runtime_start, runtime_end;
  double *target;

  
  int wordNum;
  int size;
  
  KERNEL_PARM kernel_parm;
 
  FILE *fp;
  read_input_parameters(argc,argv,docfile,outfile,embeddingfile, &verbosity,&kernel_cache_size,&kernel_parm,MU,LAMBDA);
  printf("Reading Embedding!\n");
  ReadEmbedding(embeddingfile);
  printf("Reading Embedding End!\n");
  LAMBDA = kernel_parm.lambda; // to make faster the kernel evaluation


  nol_ll(docfile,&max_docs,&max_words_doc,&ll); /* scan size of input file */

  max_words_doc+=10;
  ll+=10;
  max_docs+=2;


  docs = (DOC *)my_malloc(sizeof(DOC)*max_docs);         /* feature vectors */
  runtime_start = get_runtime();
  printf("==========ACVT computation begin==========!\n");
  read_documents(docfile, docs,max_words_doc,ll,&totwords,&totdoc,&kernel_parm);

  printf("\nNumber of examples: %ld, linear space size: %ld\n\n",totdoc,totwords);
 
  runtime_end = get_runtime();
	
  printf("Training [%s] time in cpu seconds (excluding I/O): %.2f\n", docfile,((float) runtime_end - (float) runtime_start)/100.0);  
	
  printf("=================ACVT kernel end====================\n");

   fp = fopen(outfile,"w");
   for(i=0;i<totdoc;i++){
	   fprintf(fp,"%.1f\n",docs[i].docsim*((double)5));
   }
   fclose(fp);
  
   for(i=0;i<totdoc;i++){
     freeExample(&docs[i]);
   }
  
  free(docs);

  system("pause");
  return(0);
}


/*---------------------------------------------------------------------------*/

void read_input_parameters(int argc,char *argv[],char *docfile,char *outfile,
			   char* embeddingfile, long *verbosity,long *kernel_cache_size,
			   KERNEL_PARM *kernel_parm,double mu,double lambda)
{
	kernel_parm->mu=mu; // Default Duffy and Collins Kernel 
	kernel_parm->lambda=lambda;
	kernel_parm->first_kernel=4; 
	kernel_parm->normalization=3;

    long i;

    for(i=1;(i<argc) && ((argv[i])[0] == '-');i++) {
     switch ((argv[i])[1]) 
      { 
      case 'i': i++; strcpy (docfile, argv[i]); break;
      case 'o': i++; strcpy (outfile, argv[i]); break;
	  case 'w': i++; strcpy (embeddingfile, argv[i]);break;
      default: printf("\nerror parameter %s!\n\n",argv[i]);
	       exit(0);
      }
  }
  printf("=================ACVT kernel begin====================");
  printf("\n");
  LAMBDA = kernel_parm->lambda; // to make faster the kernel evaluation 
  LAMBDA2 = LAMBDA*LAMBDA;
  MU= kernel_parm->mu;
  TKGENERALITY=kernel_parm->first_kernel;
 
}

int ReadEmbedding(const char *file_name) {
	int size;
	FILE *f = fopen(file_name, "rb");
	if (f == NULL) {
		printf("Embedding file not found\n");
		return -1;
	}
	int wordNum;
	fscanf(f, "%d", &wordNum);
	fscanf(f, "%d", &size);

	char str[1000];
	double *tmp = new double[size];
	for (int b = 0; b < wordNum; b++) {
		char ch;
		fscanf(f, "%s%c", str, &ch);
		dict[str] = new double[size];
		for (int a = 0; a < size; a++)
			fscanf(f, "%lf", &dict[str][a]);
	}
	fclose(f);
	return size;
}